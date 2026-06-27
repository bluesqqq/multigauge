(function(root) {
  var ns = root.MGEditorApp = root.MGEditorApp || {};

  function getHierarchyPanel() {
    return document.getElementById("hierarchy");
  }

  function getHierarchyJsonPanel() {
    return document.getElementById("hierarchy-json");
  }

  function getUndoButton() {
    return document.getElementById("undo-btn");
  }

  function getRedoButton() {
    return document.getElementById("redo-btn");
  }

  function defaultChildJson() {
    var palette = ["#ef4444", "#22c55e", "#3b82f6", "#f59e0b"];
    var color = palette[ns.getSelectedId() % palette.length] || "#22c55e";
    return {
      type: "rectangle",
      style: {
        width: 96,
        height: 96
      },
      paint: {
        fill: color
      },
      radius: 8,
      children: []
    };
  }

  function makeHierarchyAction(label, enabled, onClick) {
    var button = document.createElement("button");
    button.type = "button";
    button.className = "hier-action";
    button.textContent = label;
    button.disabled = !enabled;
    button.addEventListener("click", function(event) {
      event.stopPropagation();
      if (!button.disabled) {
        onClick();
      }
    });
    return button;
  }

  function renderHierarchyNode(nodes, nodeId, parentId, index) {
    var node = nodes[String(nodeId)];
    var li = document.createElement("li");
    var row = document.createElement("div");
    row.className = "hier-row";

    var button = document.createElement("button");
    var kind = node && node.kind ? node.kind : "node";
    var type = node ? (node.name || node.type || kind) : "node";
    var label = kind + ":" + type;

    button.type = "button";
    button.textContent = label + " #" + nodeId;
    button.className = "hier-label" + (nodeId === ns.getSelectedId() ? " selected" : "");
    button.addEventListener("click", function() {
      ns.selectNode(nodeId);
    });
    row.appendChild(button);

    var actions = document.createElement("div");
    actions.className = "hier-actions";

    var isElement = !!node && node.kind === "element";
    var canAddChild = !!node;
    var canPaste = !!node;
    var canDelete = isElement;
    var canCopy = isElement;
    var canMoveUp = isElement && parentId !== null && index > 0;
    var parentNode = parentId !== null ? nodes[String(parentId)] : null;
    var canMoveDown = isElement && parentNode && Array.isArray(parentNode.children) && index < parentNode.children.length - 1;

    actions.appendChild(makeHierarchyAction("Add child", canAddChild, function() {
      try {
        var result = ns.actions.createElement(nodeId, defaultChildJson());
        ns.selectNode(result && typeof result.id === "number" ? result.id : nodeId);
      } catch (err) {
        console.error(err);
      }
      ns.refreshEditorUI();
    }));

    actions.appendChild(makeHierarchyAction("Delete", canDelete, function() {
      try {
        var wasSelected = ns.getSelectedId() === nodeId;
        ns.actions.removeElement(nodeId);
        if (wasSelected) {
          ns.setSelectedId(parentId || 0);
        }
      } catch (err) {
        console.error(err);
      }
      ns.refreshEditorUI();
    }));

    actions.appendChild(makeHierarchyAction("Copy", canCopy, function() {
      try {
        ns.actions.copyElement(nodeId);
      } catch (err) {
        console.error(err);
      }
      ns.refreshEditorUI();
    }));

    actions.appendChild(makeHierarchyAction("Paste", canPaste, function() {
      try {
        var children = node && Array.isArray(node.children) ? node.children.length : 0;
        var result = ns.actions.pasteElement(nodeId, children);
        ns.selectNode(result && typeof result.id === "number" ? result.id : nodeId);
      } catch (err) {
        console.error(err);
      }
      ns.refreshEditorUI();
    }));

    actions.appendChild(makeHierarchyAction("Up", canMoveUp, function() {
      try {
        ns.actions.reorderElement(nodeId, index - 1);
      } catch (err) {
        console.error(err);
      }
      ns.refreshEditorUI();
    }));

    actions.appendChild(makeHierarchyAction("Down", canMoveDown, function() {
      try {
        ns.actions.reorderElement(nodeId, index + 1);
      } catch (err) {
        console.error(err);
      }
      ns.refreshEditorUI();
    }));

    row.appendChild(actions);
    li.appendChild(row);

    var children = node && Array.isArray(node.children) ? node.children : [];
    if (children.length) {
      var list = document.createElement("ul");
      children.forEach(function(childId, childIndex) {
        list.appendChild(renderHierarchyNode(nodes, childId, nodeId, childIndex));
      });
      li.appendChild(list);
    }

    return li;
  }

  function ensureValidSelection() {
    if (!ns.getEditor() || !ns.getSelectedId()) {
      return;
    }

    try {
      if (!ns.getEditor().hasNode(ns.getSelectedId())) {
        ns.clearSelection();
      }
    } catch (err) {
      console.error(err);
      ns.clearSelection();
    }
  }

  function refreshHistoryControls() {
    var editor = ns.getEditor();
    if (!editor) {
      return;
    }

    var undoButton = getUndoButton();
    var redoButton = getRedoButton();
    if (!undoButton || !redoButton) {
      return;
    }

    try {
      var history = editor.getHistory();
      undoButton.disabled = !history.canUndo;
      redoButton.disabled = !history.canRedo;
    } catch (err) {
      console.error(err);
      undoButton.disabled = true;
      redoButton.disabled = true;
    }
  }

  function renderHierarchyPanel() {
    var panel = getHierarchyPanel();
    var jsonPanel = getHierarchyJsonPanel();
    if (!panel || !jsonPanel) {
      return;
    }

    panel.textContent = "";
    jsonPanel.textContent = "";

    var editor = ns.getEditor();
    if (!editor) {
      panel.textContent = "No editor available.";
      jsonPanel.textContent = "No editor available.";
      return;
    }

    var data;
    try {
      data = editor.getHierarchy();
    } catch (err) {
      console.error(err);
      panel.textContent = "Could not load hierarchy.";
      jsonPanel.textContent = "Could not load hierarchy.";
      return;
    }

    try {
      jsonPanel.value = JSON.stringify(data, null, 2);
    } catch (jsonErr) {
      console.error(jsonErr);
      jsonPanel.value = "Could not stringify hierarchy.";
    }

    var roots = Array.isArray(data.roots) ? data.roots : [];
    var nodes = data.nodes || {};

    if (!roots.length) {
      panel.textContent = "No elements.";
    }

    var list = document.createElement("ul");
    roots.forEach(function(rootId) {
      list.appendChild(renderHierarchyNode(nodes, rootId, null, 0));
    });
    if (roots.length) {
      panel.appendChild(list);
    }
  }

  function selectNode(id) {
    ns.setSelectedId(id);
    ns.refreshEditorUI();
  }

  function refreshEditorUI() {
    ensureValidSelection();
    refreshHistoryControls();
    renderHierarchyPanel();
    if (ns.properties && typeof ns.properties.render === "function") {
      ns.properties.render();
    }
  }

  function initEditorUI(options) {
    options = options || {};

    ns.setRuntime(options.runtime || null);
    ns.setEditor(options.editor || null);
    ns.setCanvas(options.canvas || null);
    ns.setFace(options.face || null);
    ns.setSelectedId(options.selectedId || 0);

    if (ns.canvas && typeof ns.canvas.bind === "function" && options.canvas) {
      ns.canvas.bind(options.canvas);
    }

    var undoButton = getUndoButton();
    var redoButton = getRedoButton();
    if (undoButton) {
      undoButton.onclick = function() {
        try {
          ns.actions.undo();
        } catch (err) {
          console.error(err);
        }
        refreshEditorUI();
      };
    }
    if (redoButton) {
      redoButton.onclick = function() {
        try {
          ns.actions.redo();
        } catch (err) {
          console.error(err);
        }
        refreshEditorUI();
      };
    }

    refreshEditorUI();
  }

  ns.selectNode = selectNode;
  ns.refreshEditorUI = refreshEditorUI;
  ns.initEditorUI = initEditorUI;
  ns.hierarchy = {
    render: renderHierarchyPanel,
    refresh: refreshEditorUI,
    init: initEditorUI
  };
})(typeof globalThis !== "undefined" ? globalThis : window);
