#include <doctest/doctest.h>

#include <ostream>

#include <multigauge/value/ValueView.h>

namespace {
mg::json::Document viewDocument(const char* id, float minimum, float maximum, int unitIndex) {
    mg::json::Document document = mg::json::object();
    auto writer = document.writer();
    writer.writeObject([&](mg::json::ObjectWriter& object) { return object.write("id", id) && object.write("min", minimum) && object.write("max", maximum) && object.write("unitIndex", unitIndex); });
    return document;
}
}

TEST_CASE("ValueView applies serialized limits") {
    mg::Value* temperature = mg::Value::find("engineCoolantTemp");
    REQUIRE(temperature != nullptr);
    const mg::Measurement original = temperature->valueBase();
    temperature->setValueBase(110.0f);
    auto document = viewDocument("engineCoolantTemp", 20.0f, 80.0f, 1);
    mg::ValueView view;
    REQUIRE(mg::decodeAny(document.root(), view));
    CHECK(view.minimumBase() == doctest::Approx(20.0f));
    CHECK(view.maximumBase() == doctest::Approx(80.0f));
    CHECK(view.unit()->abbreviation == "F");
    temperature->setValueBase(original);
}

TEST_CASE("ValueView restores optional defaults and rejects unsupported values") {
    mg::Value* temperature = mg::Value::find("engineCoolantTemp");
    REQUIRE(temperature != nullptr);
    auto document = viewDocument("engineCoolantTemp", 20.0f, 80.0f, 127);
    mg::ValueView view;
    REQUIRE(mg::decodeAny(document.root(), view));
    REQUIRE(view.unit() != nullptr);
    CHECK(view.unit()->abbreviation == "C");

    const auto nullValue = mg::json::parse("null");
    CHECK(view.setProperty("min", nullValue.root()));
    CHECK(view.setProperty("max", nullValue.root()));
    CHECK(view.setProperty("unitIndex", nullValue.root()));
    CHECK(view.minimumBase() == doctest::Approx(-40.0f));
    CHECK(view.maximumBase() == doctest::Approx(120.0f));

    const auto number = mg::json::parse("1");
    CHECK_FALSE(mg::decodeAny(number.root(), view));
}

TEST_CASE("ValueView supports shorthand and serialization") {
    auto shorthand = mg::json::parse("\"engineRPM\"");
    mg::ValueView view;
    REQUIRE(mg::decodeAny(shorthand.root(), view));
    auto encoded = mg::json::object();
    auto writer = encoded.writer();
    REQUIRE(mg::encodeAny(writer, view));
    std::string_view id;
    REQUIRE(encoded.root().read(id));
    CHECK(id == "engineRPM");
}
