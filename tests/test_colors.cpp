#include <doctest/doctest.h>

#include <multigauge/graphics/colors/ColorResolver.h>
#include <multigauge/graphics/colors/StaticColor.h>
#include <multigauge/graphics/colors/ValueColor.h>
#include <multigauge/json/Json.h>
#include <multigauge/properties/Codec.h>
#include <multigauge/value/Value.h>

namespace {

using namespace mg::graphics;

class RecursiveColor final : public Color {
public:
    OwnedColor clone() const override { return std::make_unique<RecursiveColor>(); }

protected:
    rgba resolveUncached(const ColorResolver::Frame& frame) const noexcept override {
        return frame.resolve(*this);
    }
};

ColorResolver::Frame makeFrame(ColorFrame& frame, ColorResolver& resolver, UserPalette& palette) {
    frame.refresh(std::chrono::microseconds{}, palette);
    return resolver.beginFrame(frame);
}

} // namespace

TEST_CASE("color resolver requires a live frame token") {
    StaticColor color(rgba{1, 2, 3, 4});
    ColorResolver resolver;
    const rgba fallback = ColorResolver::Frame{}.resolve(color);
    CHECK(fallback.r == 0);
    CHECK(fallback.g == 0);
    CHECK(fallback.b == 0);
    CHECK(fallback.a == 0);
}

TEST_CASE("color resolver detects recursive definitions deterministically") {
    ColorFrame frame;
    ColorResolver resolver;
    UserPalette palette;
    const auto token = makeFrame(frame, resolver, palette);
    RecursiveColor color;
    const rgba resolved = token.resolve(color);
    CHECK(resolved.r == 0);
    CHECK(resolved.g == 0);
    CHECK(resolved.b == 0);
    CHECK(resolved.a == 0);
}

TEST_CASE("color resolver never allocates while resolving") {
    ColorFrame frame;
    UserPalette palette;
    ColorResolver resolver(1);
    const auto token = makeFrame(frame, resolver, palette);
    StaticColor first(rgba{1, 2, 3, 255});
    StaticColor second(rgba{4, 5, 6, 255});

    CHECK(token.resolve(first).r == 1);
    const rgba exhausted = token.resolve(second);
    CHECK(exhausted.r == 0);
    CHECK(exhausted.a == 0);
}

TEST_CASE("value colors use an indexed frame snapshot and normalized ramps") {
    mg::Value* rpm = mg::Value::find("engineRPM");
    REQUIRE(rpm != nullptr);

    ColorTimeline ramp;
    REQUIRE(ramp.addKeyframe(rgba{0, 0, 0, 255}, 0.0F));
    REQUIRE(ramp.addKeyframe(rgba{255, 0, 0, 255}, 1.0F));
    ValueColor color(rpm, std::move(ramp));

    rpm->setValueBase(rpm->minimumBase());
    ColorFrame frame;
    ColorResolver resolver;
    UserPalette palette;
    auto token = makeFrame(frame, resolver, palette);
    const rgba first = token.resolve(color);

    // Mutation after the snapshot cannot affect this frame.
    rpm->setValueBase(rpm->maximumBase());
    const rgba sameFrame = token.resolve(color);
    CHECK(sameFrame.r == first.r);

    token = makeFrame(frame, resolver, palette);
    const rgba nextFrame = token.resolve(color);
    CHECK(first.r == 0);
    CHECK(nextFrame.r == 255);
    rpm->setValueBase(rpm->minimumBase());
}

TEST_CASE("color ramps serialize complete normalized nested definitions") {
    const mg::json::Document input = mg::json::parse(R"({"keyframes":[{"pos":0,"color":"#000000FF"},{"pos":1,"color":{"type":"user","slot":"secondary"}}]})");
    REQUIRE(input.valid());

    ColorTimeline ramp;
    REQUIRE(mg::Codec<ColorTimeline>::decode(input.root(), ramp));
    CHECK(ramp.size() == 2);
    CHECK(ramp.valid());

    mg::json::Document output = mg::json::object();
    auto writer = output.writer();
    REQUIRE(mg::Codec<ColorTimeline>::encode(writer, ramp));
    CHECK(output.root().member("keyframes").isArray());
    CHECK(output.root().member("keyframes").size() == 2);
}
