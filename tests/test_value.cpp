#include <ostream>

#include <doctest/doctest.h>

#include <multigauge/value/ValueRegistry.h>

TEST_CASE("ValueRegistry exposes stable built-ins and clamps measurements") {
    const auto rpm = mg::ValueRegistry::handle(mg::BuiltInValue::engineRPM);
    CHECK(mg::ValueRegistry::id(rpm) == "engineRPM");
    CHECK(mg::ValueRegistry::name(rpm) == "RPM");
    REQUIRE(mg::ValueRegistry::set(rpm, 9000.0F));
    CHECK(mg::ValueRegistry::value(rpm) == doctest::Approx(8000.0F));
    REQUIRE(mg::ValueRegistry::set(rpm, -1.0F));
    CHECK(mg::ValueRegistry::value(rpm) == doctest::Approx(0.0F));
    CHECK(mg::ValueRegistry::available(rpm));
    REQUIRE(mg::ValueRegistry::invalidate(rpm));
    CHECK_FALSE(mg::ValueRegistry::available(rpm));
    REQUIRE(mg::ValueRegistry::set(rpm, mg::ValueRegistry::minimum(rpm)));
}

TEST_CASE("ValueRegistry keeps bounded user definitions and invalidates stale handles") {
    mg::ValueRegistry::clearUsers();
    const auto handle = mg::ValueRegistry::add("customRPM", "Custom RPM", mg::revolutions, 10.0F, 90.0F);
    REQUIRE(handle.valid());
    CHECK(handle.isUser());
    CHECK(mg::ValueRegistry::resolve("customRPM") == handle);
    REQUIRE(mg::ValueRegistry::set(handle, 100.0F));
    CHECK(mg::ValueRegistry::value(handle) == doctest::Approx(90.0F));
    REQUIRE(mg::ValueRegistry::remove(handle));
    CHECK_FALSE(mg::ValueRegistry::exists(handle));
    const auto replacement = mg::ValueRegistry::add("customRPM", "Custom RPM", mg::revolutions, 0.0F, 100.0F);
    REQUIRE(replacement.valid());
    CHECK(replacement != handle);
    mg::ValueRegistry::clearUsers();
}
