#include <multigauge/gauge/Element.h>

#include <multigauge/gauge/elements/CustomElement.h>
#include <multigauge/gauge/elements/FrameElement.h>
#include <multigauge/gauge/elements/Graph.h>
#include <multigauge/gauge/elements/Horizon.h>
#include <multigauge/gauge/elements/circular/CircularElement.h>
#include <multigauge/gauge/elements/circular/CircularNeedle.h>
#include <multigauge/gauge/elements/circular/CircularScale.h>
#include <multigauge/gauge/elements/primitives/CircleElement.h>
#include <multigauge/gauge/elements/primitives/ImageElement.h>
#include <multigauge/gauge/elements/primitives/RectangleElement.h>
#include <multigauge/gauge/elements/primitives/TextElement.h>

namespace mg::gauge {

namespace {

using Owned = Element::OwnedElement;

template <typename T> Owned create() {
    return std::make_unique<T>();
}

Owned createCustom(std::string_view type) {
    return std::make_unique<CustomElement>(std::string(type));
}

using Descriptor = MgPolymorphicTypeDescriptor<Owned>;
constexpr Descriptor types[] = {
    makePolymorphicTypeDescriptor<FrameElement, Owned>(&create<FrameElement>),
    makePolymorphicTypeDescriptor<RectangleElement, Owned>(&create<RectangleElement>),
    makePolymorphicTypeDescriptor<CircleElement, Owned>(&create<CircleElement>),
    makePolymorphicTypeDescriptor<TextElement, Owned>(&create<TextElement>),
    makePolymorphicTypeDescriptor<ImageElement, Owned>(&create<ImageElement>),
    makePolymorphicTypeDescriptor<CircularElement, Owned>(&create<CircularElement>),
    makePolymorphicTypeDescriptor<CircularNeedle, Owned>(&create<CircularNeedle>),
    makePolymorphicTypeDescriptor<CircularScale, Owned>(&create<CircularScale>),
    makePolymorphicTypeDescriptor<Graph, Owned>(&create<Graph>),
    makePolymorphicTypeDescriptor<Horizon, Owned>(&create<Horizon>),
};

} // namespace

const Element::Registry& Element::registry() {
    static const Registry registry(types, &createCustom);
    return registry;
}

} // namespace mg::gauge

namespace mg {

DECODE_IMPL(gauge::Element::OwnedElement) {
    if (v.isNull()) {
        out.reset();
        return true;
    }

    if (!v.isObject()) return false;

    std::string_view type;
    if (!v.member(TYPE_KEY).read(type) || type.empty()) return false;
    auto element = gauge::Element::registry().create(type);

    if (!element || !element->loadProperties(v)) return false;

    out = std::move(element);
    return true;
}

ENCODE_IMPL(gauge::Element::OwnedElement) {
    return v ? v->saveProperties(out) : out.null();
}

} // namespace mg
