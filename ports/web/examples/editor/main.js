(function(root) {
  async function loadJson(path) {
    var response = await fetch(path);
    if (!response.ok) {
      throw new Error("Failed to load " + path);
    }
    return response.json();
  }

  async function bootstrap() {
    try {
      if (!root.Multigauge || typeof root.Multigauge.init !== "function") {
        throw new Error("Multigauge is not available.");
      }

      var mg = await root.Multigauge.init();
      var canvas = document.getElementById("screen");
      if (!canvas) {
        throw new Error("Canvas #screen was not found.");
      }

      var runtime = mg.createRuntime(canvas);
      var editor = mg.createEditor();
      var faceJson = await loadJson("./face.json");
      var rectangleJson = await loadJson("./rectangle.json");
      var face = editor.createFace(faceJson);
      var element = editor.createElement(face, rectangleJson);

      runtime.bindEditor(editor, face);

      if (!root.MGEditorApp || typeof root.MGEditorApp.initEditorUI !== "function") {
        throw new Error("Editor UI modules were not loaded.");
      }

      root.MGEditorApp.initEditorUI({
        runtime: runtime,
        editor: editor,
        canvas: canvas,
        face: face,
        selectedId: face.id
      });

      runtime.start();
    } catch (err) {
      console.error("Multigauge editor harness failed to start:", err);
    }
  }

  if (document.readyState === "loading") {
    document.addEventListener("DOMContentLoaded", bootstrap);
  } else {
    bootstrap();
  }
})(typeof globalThis !== "undefined" ? globalThis : window);
