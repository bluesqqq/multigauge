const RUNTIME_SCRIPT_URL = "/multigauge-web/wasm/multigauge.js";
const RUNTIME_WASM_URL = "/multigauge-web/wasm/multigauge.wasm";

const RENDER_MODES = new Map([
  ["intrinsic", 0],
  ["contain", 1],
  ["cover", 2],
  ["responsive", 3],
]);

let multigaugePromise = null;
let frameLoopStarted = false;
const activeViews = new Map();

function toJsonText(value) {
  return typeof value === "string" ? value : JSON.stringify(value ?? null);
}

function parseJson(text) {
  if (typeof text !== "string") {
    return text;
  }

  try {
    return JSON.parse(text);
  } catch {
    return text;
  }
}

function unwrapResult(raw) {
  const parsed = parseJson(raw);
  if (!parsed || typeof parsed !== "object") {
    return parsed;
  }

  if (parsed.ok === false) {
    throw new Error(parsed.error || "Multigauge request failed.");
  }

  if (Object.prototype.hasOwnProperty.call(parsed, "data")) {
    return parsed.data;
  }

  return parsed;
}

function ensureCanvasId(canvas) {
  if (!canvas.id) {
    canvas.id = `multigauge-canvas-${Math.random().toString(36).slice(2, 10)}`;
  }

  return canvas.id;
}

function loadScriptOnce(src) {
  return new Promise((resolve, reject) => {
    const existing = document.querySelector(`script[src="${src}"]`);
    if (existing) {
      existing.addEventListener("load", () => resolve(), { once: true });
      existing.addEventListener("error", () => reject(new Error(`Failed to load ${src}`)), { once: true });
      return;
    }

    const script = document.createElement("script");
    script.src = src;
    script.async = true;
    script.onload = () => resolve();
    script.onerror = () => reject(new Error(`Failed to load ${src}`));
    document.head.appendChild(script);
  });
}

async function ensureMultigaugeModule() {
  if (multigaugePromise) {
    return multigaugePromise;
  }

  multigaugePromise = new Promise((resolve, reject) => {
    const previousModule = globalThis.Module;
    const module = {
      ...(previousModule && typeof previousModule === "object" ? previousModule : {}),
      locateFile(path) {
        if (path === "multigauge.wasm") {
          return RUNTIME_WASM_URL;
        }
        return `/multigauge-web/wasm/${path}`;
      },
      onRuntimeInitialized() {
        try {
          previousModule?.onRuntimeInitialized?.();
        } catch (error) {
          console.warn("[multigauge] previous onRuntimeInitialized failed:", error);
        }
        resolve(globalThis.Module);
      },
      onAbort(what) {
        try {
          previousModule?.onAbort?.(what);
        } catch (error) {
          console.warn("[multigauge] previous onAbort failed:", error);
        }
        reject(new Error(String(what || "Multigauge runtime aborted.")));
      },
    };

    globalThis.Module = module;
    loadScriptOnce(RUNTIME_SCRIPT_URL).catch(reject);
  });

  return multigaugePromise;
}

function createRuntimeApi(Module, canvas) {
  const canvasId = ensureCanvasId(canvas);
  const contextId = Module.ccall("mg_runtime_create_context", "number", ["string"], [canvasId]);
  if (!Number.isFinite(contextId) || contextId < 0) {
    throw new Error("Failed to create runtime context.");
  }

  let destroyed = false;

  const runtime = {
    id: contextId,
    load(value) {
      const payload = toJsonText(value);
      return !!Module.ccall("mg_runtime_set_gauge_screen", "number", ["number", "string"], [contextId, payload]);
    },
    setRenderMode(mode, width, height) {
      const numericMode = typeof mode === "string" ? (RENDER_MODES.get(mode.toLowerCase()) ?? 3) : (mode | 0);
      return !!Module.ccall(
        "mg_runtime_set_render_mode",
        "number",
        ["number", "number", "number", "number"],
        [contextId, numericMode, width | 0, height | 0],
      );
    },
    setGaugeScreen(value) {
      return this.load(value);
    },
    bindEditor(editor, faceId) {
      const editorId = typeof editor === "object" && editor ? (editor.id ?? 0) : (editor | 0);
      return !!Module.ccall(
        "mg_runtime_set_editor_screen",
        "number",
        ["number", "number", "number"],
        [contextId, editorId, faceId | 0],
      );
    },
    clear() {
      return !!Module.ccall("mg_runtime_clear_screen", "number", ["number"], [contextId]);
    },
    start() {
      if (frameLoopStarted) {
        return true;
      }

      frameLoopStarted = true;
      const raf = globalThis.requestAnimationFrame?.bind(globalThis) ?? ((callback) => setTimeout(callback, 16));
      const tick = () => {
        if (destroyed) {
          return;
        }

        try {
          Module.ccall("mg_runtime_frame", null, [], []);
        } catch (error) {
          console.error("[multigauge] runtime frame failed:", error);
        }

        raf(tick);
      };

      raf(tick);
      return true;
    },
    destroy() {
      destroyed = true;
      return !!Module.ccall("mg_runtime_remove_context", "number", ["number"], [contextId]);
    },
  };

  return runtime;
}

function createEditorApi(Module) {
  const editorId = Module.ccall("mg_editor_create", "number", [], []);
  if (!Number.isFinite(editorId) || editorId < 0) {
    throw new Error("Failed to create editor.");
  }

  const callJson = (symbol, argTypes, args) => unwrapResult(Module.ccall(symbol, "string", argTypes, args));
  const callBool = (symbol, argTypes, args) => !!Module.ccall(symbol, "number", argTypes, args);
  const callNumber = (symbol, argTypes, args) => Number(Module.ccall(symbol, "number", argTypes, args));
  const editor = {
    id: editorId,
    createFace(template) {
      return callJson("mg_editor_create_face", ["number", "string"], [editorId, toJsonText(template)]);
    },
    createElement(parentId, template) {
      return callJson("mg_editor_create_element", ["number", "number", "string"], [editorId, parentId | 0, toJsonText(template)]);
    },
    setPackageInfo(name, author, description) {
      return callJson("mg_editor_set_package_info", ["number", "string", "string", "string"], [editorId, name ?? "", author ?? "", description ?? ""]);
    },
    getPackageInfo() {
      return callJson("mg_editor_get_package_info", ["number"], [editorId]);
    },
    setFaceName(faceId, name) {
      return callJson("mg_editor_set_face_name", ["number", "number", "string"], [editorId, faceId | 0, name ?? ""]);
    },
    getFaceName(faceId) {
      return callJson("mg_editor_get_face_name", ["number", "number"], [editorId, faceId | 0]);
    },
    loadDocument(json) {
      return callJson("mg_editor_load_document", ["number", "string"], [editorId, toJsonText(json)]);
    },
    saveDocument() {
      return unwrapResult(Module.ccall("mg_editor_save_document", "string", ["number"], [editorId]));
    },
    getHierarchy() {
      return callJson("mg_editor_get_hierarchy", ["number"], [editorId]);
    },
    getHistory() {
      return callJson("mg_editor_get_history", ["number"], [editorId]);
    },
    getPropertiesMeta(nodeId, path) {
      return callJson("mg_editor_get_properties_meta", ["number", "number", "string"], [editorId, nodeId | 0, path ?? ""]);
    },
    setProperty(nodeId, path, value) {
      return callJson("mg_editor_set_property", ["number", "number", "string", "string"], [editorId, nodeId | 0, path ?? "", toJsonText(value)]);
    },
    removeElement(nodeId) {
      return callJson("mg_editor_remove_element", ["number", "number"], [editorId, nodeId | 0]);
    },
    reorderElement(nodeId, index) {
      return callJson("mg_editor_reorder_element", ["number", "number", "number"], [editorId, nodeId | 0, index | 0]);
    },
    undo() {
      return callBool("mg_editor_undo", ["number"], [editorId]);
    },
    redo() {
      return callBool("mg_editor_redo", ["number"], [editorId]);
    },
    jumpTo(index) {
      return callBool("mg_editor_jump_to", ["number", "number"], [editorId, index | 0]);
    },
    historyIndex() {
      return callNumber("mg_editor_history_index", ["number"], [editorId]);
    },
    copyElement(nodeId) {
      return callJson("mg_editor_copy_element", ["number", "number"], [editorId, nodeId | 0]);
    },
    cutElement(nodeId) {
      return callJson("mg_editor_cut_element", ["number", "number"], [editorId, nodeId | 0]);
    },
    pasteElement(parentId, index) {
      return callJson("mg_editor_paste_element", ["number", "number", "number"], [editorId, parentId | 0, index | 0]);
    },
    copyFace(faceId) {
      return callJson("mg_editor_copy_face", ["number", "number"], [editorId, faceId | 0]);
    },
    cutFace(faceId) {
      return callJson("mg_editor_cut_face", ["number", "number"], [editorId, faceId | 0]);
    },
    pasteFace(index) {
      return callJson("mg_editor_paste_face", ["number", "number"], [editorId, index | 0]);
    },
    removeFace(faceId) {
      return callJson("mg_editor_remove_face", ["number", "number"], [editorId, faceId | 0]);
    },
    reorderFace(faceId, index) {
      return callJson("mg_editor_reorder_face", ["number", "number", "number"], [editorId, faceId | 0, index | 0]);
    },
    listElementTypes() {
      return callJson("mg_editor_list_element_types", ["number"], [editorId]);
    },
    listElements() {
      return this.listElementTypes();
    },
    destroy() {
      return callBool("mg_editor_destroy", ["number"], [editorId]);
    },
  };

  return editor;
}

async function getMultigauge() {
  const Module = await ensureMultigaugeModule();
  const api = {
    createRuntime(canvas) {
      return createRuntimeApi(Module, canvas);
    },
    createEditor() {
      return createEditorApi(Module);
    },
  };

  if (globalThis.Multigauge && typeof globalThis.Multigauge === "object") {
    Object.assign(globalThis.Multigauge, api);
  }

  return api;
}

export async function getPageRenderer(onMessage) {
  const Multigauge = await getMultigauge();
  const views = new Map();
  let nextViewId = 1;

  const createView = async ({ canvas, gaugeJson, gaugeText, name, mode = "responsive", width, height, bindEditor }) => {
    if (!canvas) {
      throw new Error("A canvas is required.");
    }

    const runtime = Multigauge.createRuntime(canvas);
    const payload = gaugeText ?? gaugeJson;
    if (payload == null) {
      throw new Error("A gauge payload is required.");
    }

    const loaded = runtime.load(payload);
    if (!loaded) {
      throw new Error("Failed to load gauge payload.");
    }

    if (bindEditor) {
      runtime.bindEditor(bindEditor.editor ?? bindEditor, bindEditor.faceId ?? bindEditor.face ?? 0);
    }

    runtime.setRenderMode(mode, width ?? 0, height ?? 0);
    runtime.start();

    const id = nextViewId++;
    views.set(id, { runtime, canvas, name });
    onMessage?.(`${name || "Gauge"} rendered.`);
    return { id, runtime };
  };

  return {
    renderGaugeJson(options) {
      return createView({
        canvas: options?.canvas,
        gaugeJson: options?.gaugeJson,
        name: options?.name,
        width: options?.width,
        height: options?.height,
      });
    },
    renderGaugeText(options) {
      return createView({
        canvas: options?.canvas,
        gaugeText: options?.gaugeText,
        name: options?.name,
        width: options?.width,
        height: options?.height,
      });
    },
    removeView(viewId) {
      const view = views.get(Number(viewId));
      if (!view) {
        return false;
      }

      view.runtime.destroy();
      views.delete(Number(viewId));
      onMessage?.(`Removed view ${viewId}.`);
      return true;
    },
  };
}

globalThis.Multigauge = {
  init: getMultigauge,
};
