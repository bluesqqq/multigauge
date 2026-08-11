#include <multigauge/graphics/colors/Color.h>

#include <multigauge/graphics/colors/StaticColor.h>
#include <multigauge/graphics/colors/TimeColor.h>
#include <multigauge/graphics/colors/UserColor.h>
#include <multigauge/graphics/colors/ValueColor.h>

#include <atomic>
#include <utility>

namespace mg::graphics {

namespace {
std::atomic_uint32_t nextColorId{1};

std::uint32_t allocateColorId() noexcept {
    const std::uint32_t id = nextColorId.fetch_add(1, std::memory_order_relaxed);
    return id != 0 ? id : nextColorId.fetch_add(1, std::memory_order_relaxed);
}

template <typename T>
OwnedColor createColor() { return std::make_unique<T>(); }

OwnedColor createDefaultColor(std::string_view) { return std::make_unique<StaticColor>(); }

using ColorDescriptor = MgPolymorphicTypeDescriptor<OwnedColor>;
const ColorDescriptor colorTypes[] = {
    makePolymorphicTypeDescriptor<StaticColor, OwnedColor>(&createColor<StaticColor>),
    makePolymorphicTypeDescriptor<ValueColor, OwnedColor>(&createColor<ValueColor>),
    makePolymorphicTypeDescriptor<TimeColor, OwnedColor>(&createColor<TimeColor>),
    makePolymorphicTypeDescriptor<UserColor, OwnedColor>(&createColor<UserColor>),
};
}

Color::Color() noexcept : id_(allocateColorId()) {}
Color::Color(const Color&) noexcept : id_(allocateColorId()) {}

const Color::Registry& Color::registry() {
    static const Registry registry(colorTypes, &createDefaultColor);
    return registry;
}

Paint::Paint() = default;
Paint::Paint(OwnedColor fill, OwnedColor stroke, float thickness)
    : fill(std::move(fill)), stroke(std::move(stroke)), thickness(thickness) {}

} // namespace mg::graphics

namespace mg {

DECODE_IMPL(graphics::OwnedColor) {
    if (v.isNull()) { out = nullptr; return true; }
    graphics::rgba legacy;
    if (Codec<graphics::rgba>::decode(v, legacy)) {
        out = std::make_unique<graphics::StaticColor>(legacy);
        return true;
    }
    if (!v.isObject()) return false;
    std::string_view type;
    if (!v.member(TYPE_KEY).read(type) || !graphics::Color::registry().find(type)) return false;
    return decodePolymorphicOwned<graphics::Color>(v, out);
}

ENCODE_IMPL(graphics::OwnedColor) {
    if (!v) return out.null();
    if (std::string_view(v->typeId()) == graphics::StaticColor::staticTypeId()) {
        return Codec<graphics::rgba>::encode(out, static_cast<const graphics::StaticColor&>(*v).value());
    }
    return encodePolymorphicOwned(out, v);
}

} // namespace mg
