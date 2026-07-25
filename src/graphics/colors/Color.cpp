#include <multigauge/graphics/colors/Color.h>

#include <multigauge/graphics/colors/StaticColor.h>
#include <multigauge/graphics/colors/ValueColor.h>
#include <multigauge/graphics/colors/TimeColor.h>
#include <multigauge/graphics/colors/UserColor.h>

#include <utility>

namespace mg::graphics {

namespace {
    template <typename T>
    OwnedColor createColor() {
        return std::make_unique<T>();
    }

    OwnedColor createDefaultColor() {
        return std::make_unique<StaticColor>();
    }

    using ColorDescriptor = MgPolymorphicTypeDescriptor<OwnedColor>;

    static const ColorDescriptor COLOR_TYPES[] = {
        makePolymorphicTypeDescriptor<StaticColor, OwnedColor>(&createColor<StaticColor>),
        makePolymorphicTypeDescriptor<ValueColor, OwnedColor>(&createColor<ValueColor>),
        makePolymorphicTypeDescriptor<TimeColor, OwnedColor>(&createColor<TimeColor>),
        makePolymorphicTypeDescriptor<UserColor, OwnedColor>(&createColor<UserColor>),
    };
}

const Color::Registry& Color::registry() {
    static const Registry registry(COLOR_TYPES, sizeof(COLOR_TYPES) / sizeof(COLOR_TYPES[0]), &createDefaultColor);
    return registry;
}

const ColorTimeline* Color::getTimeline() const { return nullptr; }

} // namespace mg::graphics

namespace mg {

DECODE_IMPL(graphics::OwnedColor) {
    if (v.isNull()) {
        out = nullptr;
        return true;
    }

    graphics::rgba color;
    if (!Codec<graphics::rgba>::decode(v, color)) return false;

    out = std::make_unique<graphics::StaticColor>(color);
    return true;
}

ENCODE_IMPL(graphics::OwnedColor) {
    if (!v) {
        return out.null();
    }

    if (v->getType() == graphics::Color::Type::Static) {
        return Codec<graphics::rgba>::encode(out, v->getColor());
    }

    return false;
}

} // namespace mg

namespace mg::graphics {

//----------[ FILL STROKE ]----------//

Paint::Paint() : fill(nullptr), stroke(nullptr) {}

Paint::Paint(OwnedColor fill, OwnedColor stroke, float thickness) : fill(std::move(fill)), stroke(std::move(stroke)), thickness(thickness) {}

Paint Paint::blended(rgba c, float alpha) const { return Paint((fill) ? fill->blended(c, alpha) : nullptr, (stroke) ? stroke->blended(c, alpha) : nullptr, thickness); }

Paint Paint::blended(const Color &c, float alpha) const { return Paint((fill) ? fill->blended(c, alpha) : nullptr, (stroke) ? stroke->blended(c, alpha) : nullptr, thickness); }

Paint Paint::blended(const Paint &other, float alpha) const {
    OwnedColor outFill;
    OwnedColor outStroke;

    if (fill && other.fill) outFill = fill->blended(*other.fill, alpha);
    else if (fill) outFill = fill->clone();
    else if (other.fill) outFill = other.fill->clone();

    if (stroke && other.stroke) outStroke = stroke->blended(*other.stroke, alpha);
    else if (stroke) outStroke = stroke->clone();
    else if (other.stroke) outStroke = other.stroke->clone();

    return Paint(std::move(outFill), std::move(outStroke), thickness);
}

} // namespace mg::graphics
