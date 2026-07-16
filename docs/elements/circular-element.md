# Circular Element

![Circular element preview](../../assets/placeholder.jpg)

The circular element provides shared circular layout and property behavior for circular descendants.

## Header

[`core/include/multigauge/gauge/elements/circular/CircularElement.h`](../../core/include/multigauge/gauge/elements/circular/CircularElement.h)

## Properties

### `value`
Numeric value used by circular children that inherit from this element.

### `startAngle`
Starting angle of the circular range.

### `endAngle`
Ending angle of the circular range.

`CircularElement` is useful as a parent node because child circular elements inherit `value`, `startAngle`, and `endAngle` from the nearest circular ancestor when their own values are unset.
