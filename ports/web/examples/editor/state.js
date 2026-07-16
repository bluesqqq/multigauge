(function(root) {
  var ns = root.MGEditorApp = root.MGEditorApp || {};
  var state = {
    runtime: null,
    editor: null,
    canvas: null,
    face: null,
    selectedId: 0,
    started: false
  };

  function handleId(value) {
    if (value && typeof value === "object" && typeof value.id === "number") {
      return value.id;
    }
    if (typeof value === "number" && !isNaN(value)) {
      return value;
    }
    return 0;
  }

  ns.state = state;
  ns.handleId = handleId;

  ns.setRuntime = function(runtime) {
    state.runtime = runtime || null;
    return state.runtime;
  };

  ns.setEditor = function(editor) {
    state.editor = editor || null;
    return state.editor;
  };

  ns.setCanvas = function(canvas) {
    state.canvas = canvas || null;
    return state.canvas;
  };

  ns.setFace = function(face) {
    state.face = face || null;
    return state.face;
  };

  ns.setSelectedId = function(value) {
    state.selectedId = handleId(value);
    return state.selectedId;
  };

  ns.clearSelection = function() {
    state.selectedId = 0;
    return state.selectedId;
  };

  ns.getSelectedId = function() {
    return state.selectedId;
  };

  ns.getRuntime = function() {
    return state.runtime;
  };

  ns.getEditor = function() {
    return state.editor;
  };

  ns.getCanvas = function() {
    return state.canvas;
  };

  ns.getFace = function() {
    return state.face;
  };
})(typeof globalThis !== "undefined" ? globalThis : window);
