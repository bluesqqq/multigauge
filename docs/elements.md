# Elements

`GaugeFace` owns a tree of serializable `Element` objects. The registered built-in
types are `frame`, `rectangle`, `circle`, `text`, `image`, `graph`, `horizon`,
`circular-element`, `circular-needle`, and `circular-scale`.

Each element has a required `type`, optional `layout`, and may contain `children`.
Unknown types load as `CustomElement` so their type and properties can round-trip.
The exact document format is [gauge.schema.json](./schemas/gauge.schema.json).

To add a built-in type, derive from `Element`, give it `MG_EDITOR_NAME` and
`MG_TYPE_ID` metadata, expose serialized properties with `MG_PROPS_*`, and register
it in [`src/gauge/Element.cpp`](../src/gauge/Element.cpp).
