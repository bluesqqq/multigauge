(function(root) {
  var ns = root.MGEditorApp = root.MGEditorApp || {};

  function getPropertiesPanel() {
    return document.getElementById("properties");
  }

  function getPropertiesJsonPanel() {
    return document.getElementById("properties-json");
  }

  function normalizeColorInputValue(value) {
    if (typeof value !== "string") {
      return "#000000";
    }

    var s = value.trim();
    if (/^#[0-9a-fA-F]{8}$/.test(s)) {
      return s.slice(0, 7);
    }

    if (/^#[0-9a-fA-F]{6}$/.test(s)) {
      return s;
    }

    if (/^[0-9a-fA-F]{6}$/.test(s)) {
      return "#" + s;
    }

    return "#000000";
  }

  function valueToText(value) {
    if (value === null || value === undefined) {
      return "";
    }

    if (typeof value === "string") {
      return value;
    }

    return JSON.stringify(value);
  }

  function commitPropertyChange(path, valueJson) {
    ns.actions.setProperty(ns.getSelectedId(), path, valueJson);
    ns.refreshEditorUI();
  }

  function metaToJsonValue(meta) {
    if (!meta) {
      return null;
    }

    if (Array.isArray(meta.properties)) {
      var out = {};
      meta.properties.forEach(function(child) {
        if (!child || !child.key) {
          return;
        }
        out[child.key] = metaToJsonValue(child);
      });
      return out;
    }

    return meta.value;
  }

  function metaListToJson(metaList) {
    var out = {};
    (Array.isArray(metaList) ? metaList : []).forEach(function(entry) {
      if (!entry || !entry.key) {
        return;
      }
      out[entry.key] = metaToJsonValue(entry);
    });
    return out;
  }

  function createPropertyControl(meta, path) {
    var value = meta.value;
    var options = Array.isArray(meta.options) ? meta.options : null;
    var widget = meta.widget || "json";

    if (options && options.length) {
      var select = document.createElement("select");
      options.forEach(function(opt) {
        var option = document.createElement("option");
        option.value = opt.value;
        option.textContent = opt.label || opt.value;
        select.appendChild(option);
      });
      if (value !== null && value !== undefined) {
        select.value = String(value);
      }
      select.addEventListener("change", function() {
        try {
          commitPropertyChange(path, JSON.stringify(select.value));
        } catch (err) {
          console.error(err);
        }
      });
      return select;
    }

    if (widget === "color-selector") {
      var color = document.createElement("input");
      color.type = "color";
      color.value = normalizeColorInputValue(value);
      color.addEventListener("input", function() {
        try {
          commitPropertyChange(path, JSON.stringify(color.value));
        } catch (err) {
          console.error(err);
        }
      });
      return color;
    }

    if (widget === "boolean") {
      var checkbox = document.createElement("input");
      checkbox.type = "checkbox";
      checkbox.checked = !!value;
      checkbox.addEventListener("change", function() {
        try {
          commitPropertyChange(path, JSON.stringify(checkbox.checked));
        } catch (err) {
          console.error(err);
        }
      });
      return checkbox;
    }

    if (widget === "number") {
      var number = document.createElement("input");
      number.type = "number";
      number.step = "any";
      number.value = valueToText(value);
      number.addEventListener("input", function() {
        if (number.value === "") {
          return;
        }
        var parsed = Number(number.value);
        if (!Number.isNaN(parsed)) {
          try {
            commitPropertyChange(path, JSON.stringify(parsed));
          } catch (err) {
            console.error(err);
          }
        }
      });
      return number;
    }

    var input = document.createElement("input");
    input.type = "text";
    input.value = valueToText(value);
    input.addEventListener("change", function() {
      try {
        if (widget === "layout-size") {
          var trimmed = input.value.trim();
          var payload = trimmed === "" ? JSON.stringify("auto") : (/^[+-]?(?:\d+\.?\d*|\.\d+)$/.test(trimmed) ? JSON.stringify(Number(trimmed)) : JSON.stringify(trimmed));
          commitPropertyChange(path, payload);
        } else if (widget === "json") {
          commitPropertyChange(path, JSON.stringify(JSON.parse(input.value)));
        } else {
          commitPropertyChange(path, JSON.stringify(input.value));
        }
      } catch (err) {
        console.error(err);
      }
    });
    return input;
  }

  function renderPropertyMeta(meta, path) {
    if (!meta) {
      return document.createTextNode("");
    }

    var fullPath = path || meta.key || "";

    if (Array.isArray(meta.properties)) {
      var fieldset = document.createElement("fieldset");
      var legend = document.createElement("legend");
      var title = meta.name || meta.key || fullPath;
      if (meta.types && meta.types.current) {
        title += " (" + meta.types.current + ")";
      }
      legend.textContent = title;
      fieldset.appendChild(legend);

      if (meta.description) {
        var description = document.createElement("div");
        description.textContent = meta.description;
        description.style.marginBottom = "8px";
        description.style.color = "#94a3b8";
        fieldset.appendChild(description);
      }

      meta.properties.forEach(function(child) {
        fieldset.appendChild(renderPropertyMeta(child, fullPath + "." + child.key));
      });

      return fieldset;
    }

    var row = document.createElement("div");
    row.className = "prop-row";

    var label = document.createElement("label");
    label.textContent = meta.name || meta.key || fullPath;
    row.appendChild(label);

    row.appendChild(createPropertyControl(meta, fullPath));

    if (meta.description) {
      var help = document.createElement("div");
      help.textContent = meta.description;
      help.style.fontSize = "11px";
      help.style.marginTop = "4px";
      help.style.color = "#94a3b8";
      row.appendChild(help);
    }

    return row;
  }

  function renderPropertiesPanel() {
    var panel = getPropertiesPanel();
    var jsonPanel = getPropertiesJsonPanel();
    if (!panel || !jsonPanel) {
      return;
    }

    panel.textContent = "";
    jsonPanel.value = "";

    var editor = ns.getEditor();
    var selectedId = ns.getSelectedId();
    if (!editor || !selectedId) {
      panel.textContent = "Select an element.";
      jsonPanel.value = "Select an element.";
      return;
    }

    var data;
    try {
      data = editor.getPropertiesMeta(selectedId, "");
    } catch (err) {
      console.error(err);
      panel.textContent = "Could not load properties.";
      jsonPanel.value = "Could not load properties.";
      return;
    }

    try {
      jsonPanel.value = JSON.stringify(metaListToJson(data.meta), null, 2);
    } catch (jsonErr) {
      console.error(jsonErr);
      jsonPanel.value = "Could not stringify properties.";
    }

    var heading = document.createElement("div");
    heading.textContent = "Selected #" + selectedId;
    heading.style.marginBottom = "10px";
    heading.style.color = "#cbd5e1";
    panel.appendChild(heading);

    var meta = Array.isArray(data.meta) ? data.meta : [];
    if (!meta.length) {
      panel.appendChild(document.createTextNode("No editable properties."));
      return;
    }

    meta.forEach(function(entry) {
      panel.appendChild(renderPropertyMeta(entry, entry.key));
    });
  }

  ns.properties = {
    render: renderPropertiesPanel
  };
})(typeof globalThis !== "undefined" ? globalThis : window);
