# Elements

This page is a quick guide to the built-in gauge elements available in `multigauge-core`, plus a short note on how to add your own.

## Overview

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

### Root Element

The document root is [`RootElement`](../include/multigauge/gauge/elements/RootElement.h).

- Type: none, this is implicit at the document root
- Purpose: top-level container for the whole gauge face
- Key props: `title`, `description`, `bgColor`

### Primitive Elements

#### `rectangle`

- Header: [`include/multigauge/gauge/elements/primitives/RectangleElement.h`](../include/multigauge/gauge/elements/primitives/RectangleElement.h)
- Purpose: filled/stroked rectangle
- Key props: `paint`, `radius`

#### `circle`

- Header: [`include/multigauge/gauge/elements/primitives/CircleElement.h`](../include/multigauge/gauge/elements/primitives/CircleElement.h)
- Purpose: filled/stroked circle or ellipse inside the element bounds
- Key props: `paint`

#### `text`

- Header: [`include/multigauge/gauge/elements/primitives/TextElement.h`](../include/multigauge/gauge/elements/primitives/TextElement.h)
- Purpose: draw text in the element bounds
- Key props: `text`, `paint`, `ellipses`, `hyphens`

#### `image`

- Header: [`include/multigauge/gauge/elements/primitives/ImageElement.h`](../include/multigauge/gauge/elements/primitives/ImageElement.h)
- Purpose: load and draw an image asset
- Key props: `path`

## Specialized Elements

### `horizon`

- Header: [`include/multigauge/gauge/elements/Horizon.h`](../include/multigauge/gauge/elements/Horizon.h)
- Purpose: synthetic horizon-style background/grid element
- Key props: `bgColor`, `groundColor`, `horizonColor`, `borderColor`

### `graph`

- Header: [`include/multigauge/gauge/elements/Graph.h`](../include/multigauge/gauge/elements/Graph.h)
- Purpose: time-series style value graph
- Key props: `seconds`, `bgColor`, `secondsColor`, `graphColor`, `borderColor`, `value`

## Circular Elements

### `circular-element`

- Header: [`include/multigauge/gauge/elements/circular/CircularElement.h`](../include/multigauge/gauge/elements/circular/CircularElement.h)
- Purpose: base circular container and shared circular property source
- Key props: `value`, `startAngle`, `endAngle`

`CircularElement` is useful as a parent node because child circular elements inherit `value`, `startAngle`, and `endAngle` from the nearest circular ancestor when their own values are unset.

### `circular-scale`

- Header: [`include/multigauge/gauge/elements/circular/CircularScale.h`](../include/multigauge/gauge/elements/circular/CircularScale.h)
- Purpose: draw a circular tick scale
- Key props: `ticks`, `radius`

Tick data lives under [`include/multigauge/gauge/ticks`](../include/multigauge/gauge/ticks).

### `circular-needle`

- Header: [`include/multigauge/gauge/elements/circular/CircularNeedle.h`](../include/multigauge/gauge/elements/circular/CircularNeedle.h)
- Purpose: draw a needle against a circular value range
- Key props: `paint`, `radius`

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