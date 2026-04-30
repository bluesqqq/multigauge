# Elements

This page indexes the built-in gauge elements available in `multigauge-core`.

## Overview

![Element preview](../assets/placeholder.jpg)

Every visible part of a gauge is an `Element` or a subclass of it. A gauge face is made from a tree of elements, where each element has:

- layout through [`Layout`](../include/multigauge/gauge/Layout.h)
- element-specific properties in `props`
- optional child elements

Element JSON generally looks like this:

```json
{
  "type": "rectangle",
  "style": {
    "width": "100%",
    "height": 40
  },
  "props": {},
  "children": []
}
```

The `style` block is shared across all elements and is backed by Yoga layout.

## Built-In Elements

### Primitive Elements

- [rectangle](elements/rectangle.md)
- [circle](elements/circle.md)
- [text](elements/text.md)
- [image](elements/image.md)

### Specialized Elements

- [horizon](elements/horizon.md)
- [graph](elements/graph.md)

### Circular Elements

- [circular-element](elements/circular-element.md)
- [circular-scale](elements/circular-scale.md)
- [circular-needle](elements/circular-needle.md)

## Implementing Your Own Element

Custom elements derive from [`Element`](../include/multigauge/gauge/Element.h) or from one of the existing element subclasses.

The usual steps are:

1. Create a header under [`include/multigauge/gauge/elements`](../include/multigauge/gauge/elements) and a `.cpp` under [`src/gauge/elements`](../src/gauge/elements)
2. Add `MG_EDITOR_NAME(...)` and `MG_TYPE_ID("...")` to define the editor label and JSON type name
3. Add properties with the `MG_PROPS_*` macros so the element can load/save cleanly
4. Override `init(...)` if the element needs assets or setup
5. Override `draw(...)` to render the element
6. Override `update(...)` if the element has time-based behavior
7. Register the new type in [`src/gauge/Element.cpp`](../src/gauge/Element.cpp) inside `ELEMENT_TYPES`

Once registered, the new element can be loaded from JSON by its `type` value.

## Notes

- Unknown `type` values currently fall back to the base `Element`
- The base `Element` itself does not draw anything; it mainly provides layout and child management
