#include <doctest/doctest.h>

#include <multigauge/graphics/colors/Color.h>

#include <string_view>

namespace {

template <typename Registry>
void checkRegistry(const Registry& registry) {
    REQUIRE(registry.size() > 0);

    for (const auto& descriptor : registry) {
        REQUIRE(descriptor.id != nullptr);
        CHECK(std::string_view(descriptor.id).size() > 0);
        CHECK(descriptor.create != nullptr);
        CHECK(registry.find(descriptor.id) == &descriptor);

        for (const auto& other : registry) {
            if (&descriptor == &other) continue;
            CHECK(std::string_view(descriptor.id) != std::string_view(other.id));
        }
    }
}

} // namespace

TEST_CASE("polymorphic registries expose unique, constructible type descriptors") {
    checkRegistry(mg::graphics::Color::registry());
}
