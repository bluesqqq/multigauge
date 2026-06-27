(function(root) {
  var ns = root.MGEditorApp = root.MGEditorApp || {};

  function editor() {
    var instance = ns.getEditor ? ns.getEditor() : ns.state && ns.state.editor;
    if (!instance) {
      throw new Error("The editor is not ready.");
    }
    return instance;
  }

  ns.actions = {
    createElement: function(parentId, json) {
      return editor().createElement(parentId, json);
    },

    removeElement: function(elementId) {
      return editor().removeElement(elementId);
    },

    copyElement: function(elementId) {
      return editor().copyElement(elementId);
    },

    pasteElement: function(parentId, index) {
      return editor().pasteElement(parentId, index);
    },

    reorderElement: function(elementId, index) {
      return editor().reorderElement(elementId, index);
    },

    setProperty: function(id, path, value) {
      return editor().setProperty(id, path, value);
    },

    undo: function() {
      return editor().undo();
    },

    jumpTo: function(index) {
      return editor().jumpTo(index);
    },

    redo: function() {
      return editor().redo();
    }
  };
})(typeof globalThis !== "undefined" ? globalThis : window);
