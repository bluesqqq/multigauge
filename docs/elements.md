# Elements

Elements are the serializable, drawable parts of a `GaugeFace`. Each element has shared
`style` and `children` properties plus its own properties. See
[gauge.schema.json](./schemas/gauge.schema.json) for the JSON contract.

Built-in types:

- Primitives: `rectangle`, `circle`, `text`, `image`
- Specialized: `horizon`, `graph`
- Circular: `circular-element`, `circular-scale`, `circular-needle`

To add an element, derive from [`Element`](../include/multigauge/gauge/Element.h),
define its properties, and register its type in
[`src/gauge/Element.cpp`](../src/gauge/Element.cpp). The `MG_TYPE_ID` string is part
of the serialized format.
