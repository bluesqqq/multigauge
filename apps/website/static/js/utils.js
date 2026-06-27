export function map(x, inMin, inMax, outMin, outMax) {
    if (inMax === inMin) { return -1; }
    return ((x - inMin) * (outMax - outMin)) / (inMax - inMin) + outMin;
}

export function lerp(start, end, amount) { return start + (end - start) * amount; }

export function loadFromRegistry(registry, json) {
    if (!json || !json.type) throw new Error("Invalid JSON: missing type");

    const objectClass = registry.get(json.type);
    if (!objectClass) throw new Error(`Unknown type: ${json.type}`);

    if (Object.prototype.hasOwnProperty.call(objectClass, 'fromJSON')) return objectClass.fromJSON(json);

    return Object.assign(new objectClass(), json);
}

export function isNestedWithin(child, parent, seen = new WeakSet()) {
    if (typeof child !== 'object' || child === null || typeof parent !== 'object' || parent === null) return false;

    if (child === parent) { return false; }

    if (seen.has(parent)) return false; // Prevents recursive loops on circular references
    seen.add(parent);

    if (Array.isArray(parent)) { // Arrays
        for (const item of parent) {
            if (item === child || isNestedWithin(child, item, seen)) {
                return true;
            }
        }
    } else { // Objects
        for (const key in parent) {
            if (Object.hasOwn(parent, key)) {
                const value = parent[key];
                if (value === child || isNestedWithin(child, value, seen)) {
                    return true;
                }
            }
        }
    }

    return false;
}

export function findNestedInstancesOfType(root, Type, found = new Set(), seen = new WeakSet()) {
    if (typeof root !== 'object' || root === null) return found;

    if (seen.has(root)) return found;
    seen.add(root);

    if (root instanceof Type) {
        found.add(root);
    }

    if (Array.isArray(root)) {
        for (const item of root) {
            findNestedInstancesOfType(item, Type, found, seen);
        }
    } else {
        for (const key in root) {
            if (Object.hasOwn(root, key)) {
                findNestedInstancesOfType(root[key], Type, found, seen);
            }
        }
    }

    return found;
}

export function onNodeRemoved(node, callback) {
    const observer = new MutationObserver(() => {
        if (!document.body.contains(node)) {
            observer.disconnect();
            callback();
        }
    });
    observer.observe(document.body, { childList: true, subtree: true });
}

export function roundTo(value, decimalPlaces) {
    const factor = Math.pow(10, decimalPlaces);
    return Math.round(value * factor) / factor;
}

export function exportCanvas(canvas) {
    const image = canvas.toDataURL('image/png'); // Default is PNG

    const link = document.createElement('a');
    link.download = 'gauge.png';
    link.href = image;
    link.click();
}

export function floorDivisible(n, factor, offset) { return Math.floor(n - offset) / factor * factor + offset; }
export function ceilDivisible(n, factor, offset) { return Math.ceil(n - offset) / factor * factor + offset; }
export function inRange(n, min, max) { return (n >= min && n <= max); }

function componentToHex(c) {
  var hex = c.toString(16);
  return hex.length == 1 ? "0" + hex : hex;
}

export function rgbToHex(r, g, b) { return "#" + componentToHex(r) + componentToHex(g) + componentToHex(b); }

export function hexToRgb(hex) {
  var result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);
  return result ? {
    r: parseInt(result[1], 16),
    g: parseInt(result[2], 16),
    b: parseInt(result[3], 16)
  } : null;
}

export function rgbToHsl(r, g, b) {
  r /= 255, g /= 255, b /= 255;

  var max = Math.max(r, g, b), min = Math.min(r, g, b);
  var h, s, l = (max + min) / 2;

  if (max == min) {
    h = s = 0; // achromatic
  } else {
    var d = max - min;
    s = l > 0.5 ? d / (2 - max - min) : d / (max + min);

    switch (max) {
      case r: h = (g - b) / d + (g < b ? 6 : 0); break;
      case g: h = (b - r) / d + 2; break;
      case b: h = (r - g) / d + 4; break;
    }

    h /= 6;
  }

  return [ h, s, l ];
}

export function hslToRgb(h, s, l) {
  var r, g, b;

  if (s == 0) {
    r = g = b = l; // achromatic
  } else {
    function hue2rgb(p, q, t) {
      if (t < 0) t += 1;
      if (t > 1) t -= 1;
      if (t < 1/6) return p + (q - p) * 6 * t;
      if (t < 1/2) return q;
      if (t < 2/3) return p + (q - p) * (2/3 - t) * 6;
      return p;
    }

    var q = l < 0.5 ? l * (1 + s) : l + s - l * s;
    var p = 2 * l - q;

    r = hue2rgb(p, q, h + 1/3);
    g = hue2rgb(p, q, h);
    b = hue2rgb(p, q, h - 1/3);
  }

  return [ r * 255, g * 255, b * 255 ];
}

export function hexToHsl(hex) {
  const rgb = hexToRgb(hex);
  if (!rgb) return null;
  return rgbToHsl(rgb.r, rgb.g, rgb.b);
}

export function hslToHex(h, s, l) {
  const [r, g, b] = hslToRgb(h, s, l);
  return rgbToHex(Math.round(r), Math.round(g), Math.round(b));
}

export function rgbToHsv(r, g, b) {
  r /= 255, g /= 255, b /= 255;

  var max = Math.max(r, g, b), min = Math.min(r, g, b);
  var h, s, v = max;

  var d = max - min;
  s = max == 0 ? 0 : d / max;

  if (max == min) {
    h = 0; // achromatic
  } else {
    switch (max) {
      case r: h = (g - b) / d + (g < b ? 6 : 0); break;
      case g: h = (b - r) / d + 2; break;
      case b: h = (r - g) / d + 4; break;
    }

    h /= 6;
  }

  return [ h, s, v ];
}

export function hsvToRgb(h, s, v) {
  var r, g, b;

  var i = Math.floor(h * 6);
  var f = h * 6 - i;
  var p = v * (1 - s);
  var q = v * (1 - f * s);
  var t = v * (1 - (1 - f) * s);

  switch (i % 6) {
    case 0: r = v, g = t, b = p; break;
    case 1: r = q, g = v, b = p; break;
    case 2: r = p, g = v, b = t; break;
    case 3: r = p, g = q, b = v; break;
    case 4: r = t, g = p, b = v; break;
    case 5: r = v, g = p, b = q; break;
  }

  return [ r * 255, g * 255, b * 255 ];
}

export function hsvToHex(h, s, v) {
  const [r, g, b] = hsvToRgb(h, s, v);
  return rgbToHex(Math.round(r), Math.round(g), Math.round(b));
}

export function hexToHsv(hex) {
  const rgb = hexToRgb(hex);
  if (!rgb) return null;
  return rgbToHsv(rgb.r, rgb.g, rgb.b);
}

export function arcLengthToAngleRadians(arcLength, radius) {
  if (radius == 0) return 0;
  return arcLength / radius;
}