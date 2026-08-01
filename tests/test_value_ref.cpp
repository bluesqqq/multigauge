#include <ostream>

#include <doctest/doctest.h>

#include <multigauge/value/ValueRef.h>

TEST_CASE("ValueRef is a compact registry handle") {
    static_assert(sizeof(mg::ValueRef) == sizeof(mg::ValueHandle));

    const mg::ValueRef rpm(mg::BuiltInValue::engineRPM);
    REQUIRE(rpm);
    CHECK(rpm.id() == "engineRPM");
    CHECK(rpm.name() == "RPM");

    mg::ValueRef missing("future.value");
    CHECK_FALSE(missing);
    CHECK(missing.id().empty());
}

TEST_CASE("ValueRef codec requires a registered value") {
    mg::ValueRef decoded;
    const auto known = mg::json::parse("\"engineRPM\"");
    REQUIRE(mg::decodeAny(known.root(), decoded));
    CHECK(decoded);

    const auto unknown = mg::json::parse("\"future.value\"");
    CHECK_FALSE(mg::decodeAny(unknown.root(), decoded));
    CHECK_FALSE(decoded);
}
