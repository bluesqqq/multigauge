(function(root) {
  var ns = root.MGEditorApp = root.MGEditorApp || {};

  function getCanvas() {
    return ns.getCanvas ? ns.getCanvas() : null;
  }

  function bindCanvas(canvas) {
    if (!canvas) {
      return;
    }

    ns.setCanvas(canvas);
  }

  ns.canvas = {
    bind: bindCanvas,
    canvasPointFromEvent: function() {
      return { x: 0, y: 0 };
    },
    handleClick: function() {}
  };
})(typeof globalThis !== "undefined" ? globalThis : window);
