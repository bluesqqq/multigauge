#include <doctest/doctest.h>

#include <ostream>
#include <multigauge/value/ValueRef.h>

TEST_CASE("ValueRef resolves, retargets, and clears value IDs") {
  mg::ValueRef reference;
  CHECK_FALSE(reference);
  CHECK(reference.id().empty());
  CHECK(reference.get() == nullptr);
  CHECK_FALSE(reference.resolve());

  reference.setId("engineRPM");
  REQUIRE(reference);
  CHECK(reference.get() == mg::Value::find("engineRPM"));
  CHECK(std::string((*reference).id()) == "engineRPM");
  CHECK(std::string(reference->name()) == "RPM");

  reference.setId("future.value");
  CHECK_FALSE(reference);
  CHECK(reference.id() == "future.value");
  CHECK_FALSE(reference.resolve());

  reference.clear();
  CHECK_FALSE(reference);
  CHECK(reference.id().empty());
}

TEST_CASE("ValueRef can be constructed from a value pointer") {
  mg::Value* rpm = mg::Value::find("engineRPM");
  REQUIRE(rpm != nullptr);

  const mg::ValueRef known(rpm);
  CHECK(known);
  CHECK(known.get() == rpm);
  CHECK(known.id() == "engineRPM");

  const mg::ValueRef empty(nullptr);
  CHECK_FALSE(empty);
  CHECK(empty.id().empty());
}

TEST_CASE("ValueRef codec preserves IDs and rejects unsupported JSON") {
  const mg::ValueRef empty;
  mg::json::Document encodedEmpty = mg::json::object(); auto emptyWriter = encodedEmpty.writer(); CHECK(mg::encodeAny(emptyWriter, empty)); CHECK(encodedEmpty.root().isNull());

  const mg::ValueRef known("engineRPM");
  mg::json::Document encodedKnown = mg::json::object(); auto knownWriter = encodedKnown.writer(); CHECK(mg::encodeAny(knownWriter, known)); std::string_view knownText; REQUIRE(encodedKnown.root().read(knownText)); CHECK(knownText == "engineRPM");

  const mg::ValueRef unknown("future.value");
  mg::json::Document encodedUnknown = mg::json::object(); auto unknownWriter = encodedUnknown.writer(); CHECK(mg::encodeAny(unknownWriter, unknown)); std::string_view unknownText; REQUIRE(encodedUnknown.root().read(unknownText)); CHECK(unknownText == "future.value");

  mg::ValueRef decoded("engineRPM");
  mg::json::Document nullValue = mg::json::parse("null");
  CHECK(mg::decodeAny(nullValue.root(), decoded));
  CHECK_FALSE(decoded);
  CHECK(decoded.id().empty());

  mg::json::Document unknownId = mg::json::parse("\"future.value\"");
  CHECK(mg::decodeAny(unknownId.root(), decoded));
  CHECK_FALSE(decoded);
  CHECK(decoded.id() == "future.value");

  mg::json::Document number = mg::json::parse("7");
  CHECK_FALSE(mg::decodeAny(number.root(), decoded));
  CHECK(decoded.id() == "future.value");
}
