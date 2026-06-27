const GAUGE_SCHEMA_URL = "../../multigauge-core/docs/schemas/gauge.schema.json";

const DEFAULT_FACE_TEMPLATE = {
	$schema: GAUGE_SCHEMA_URL,
	bgColor: "#ffffff",
	layout: {
		"flex-container": {
			direction: "column",
			justify: "center",
			"align-items": "center",
		},
		padding: {
			left: 0,
			right: 0,
			top: 0,
			bottom: 0,
		},
	},
	children: [],
};

const DEFAULT_ELEMENT_TEMPLATE = {
	type: "rectangle",
	paint: {
		fill: "#22c55e",
	},
	radius: 8,
	children: [],
};

function getModule() {
	const module = globalThis.Module;
	if (!module || typeof module.ccall !== "function") {
		throw new Error("Multigauge runtime is not ready.");
	}
	return module;
}

function callJson(symbol, argTypes, args) {
	const raw = getModule().ccall(symbol, "string", argTypes, args);
	if (typeof raw !== "string") {
		return raw;
	}
	return JSON.parse(raw);
}

function normalizeSaveText(result) {
	if (typeof result === "string") {
		return result;
	}
	if (result == null) {
		return "";
	}
	if (typeof result === "object") {
		if (typeof result.json === "string") return result.json;
		if (typeof result.data === "string") return result.data;
	}
	return JSON.stringify(result, null, 2);
}

export function makeBlankFaceTemplate() {
	const face = structuredClone(DEFAULT_FACE_TEMPLATE);
	return face;
}

export function makeDefaultElementTemplate() {
	return structuredClone(DEFAULT_ELEMENT_TEMPLATE);
}

export async function createEditorSession() {
	if (!globalThis.Multigauge || typeof globalThis.Multigauge.init !== "function") {
		throw new Error("Multigauge is not available.");
	}

	await globalThis.Multigauge.init();
	const editor = globalThis.Multigauge.createEditor();

	const api = {
		editor,
		listElements() {
			return typeof editor.listElements === "function"
				? editor.listElements()
				: editor.listElementTypes?.();
		},
		formatSaveText(value) {
			return normalizeSaveText(value);
		},
	};

	for (const name of [
		"createFace",
		"createElement",
		"setPackageInfo",
		"getPackageInfo",
		"setFaceName",
		"getFaceName",
		"loadDocument",
		"saveDocument",
		"getHierarchy",
		"getHistory",
		"getPropertiesMeta",
		"setProperty",
		"removeElement",
		"reorderElement",
		"undo",
		"redo",
		"jumpTo",
		"historyIndex",
		"copyElement",
		"pasteElement",
	]) {
		api[name] = editor[name].bind(editor);
	}

	api.cutElement = (id) => callJson("mg_editor_cut_element", ["number", "number"], [editor.id, id]);
	api.copyFace = (id) => callJson("mg_editor_copy_face", ["number", "number"], [editor.id, id]);
	api.cutFace = (id) => callJson("mg_editor_cut_face", ["number", "number"], [editor.id, id]);
	api.pasteFace = (index) => callJson("mg_editor_paste_face", ["number", "number"], [editor.id, index]);
	api.removeFace = (id) => callJson("mg_editor_remove_face", ["number", "number"], [editor.id, id]);
	api.reorderFace = (id, index) => callJson("mg_editor_reorder_face", ["number", "number", "number"], [editor.id, id, index]);
	api.listElementTypes = () => callJson("mg_editor_list_element_types", ["number"], [editor.id]);

	return api;
}
