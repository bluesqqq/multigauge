#include <doctest/doctest.h>

#include <ostream>
#include <rapidjson/document.h>

#include <multigauge/value/ValueRef.h>

namespace {

rapidjson::Value encode(const mg::ValueRef& value, rapidjson::Document& document) {
  rapidjson::Value encoded;
  CHECK(mg::Codec<mg::ValueRef>::encode(encoded, document.GetAllocator(), value));
  return encoded;
}

} // namespace

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
  rapidjson::Document document;
  document.SetObject();

  const mg::ValueRef empty;
  const rapidjson::Value encodedEmpty = encode(empty, document);
  CHECK(encodedEmpty.IsNull());

  const mg::ValueRef known("engineRPM");
  const rapidjson::Value encodedKnown = encode(known, document);
  REQUIRE(encodedKnown.IsString());
  CHECK(std::string_view(encodedKnown.GetString(), encodedKnown.GetStringLength()) == "engineRPM");

  const mg::ValueRef unknown("future.value");
  const rapidjson::Value encodedUnknown = encode(unknown, document);
  REQUIRE(encodedUnknown.IsString());
  CHECK(std::string_view(encodedUnknown.GetString(), encodedUnknown.GetStringLength()) == "future.value");

  mg::ValueRef decoded("engineRPM");
  rapidjson::Value nullValue(rapidjson::kNullType);
  CHECK(mg::Codec<mg::ValueRef>::decode(nullValue, decoded));
  CHECK_FALSE(decoded);
  CHECK(decoded.id().empty());

  rapidjson::Value unknownId("future.value", document.GetAllocator());
  CHECK(mg::Codec<mg::ValueRef>::decode(unknownId, decoded));
  CHECK_FALSE(decoded);
  CHECK(decoded.id() == "future.value");

  rapidjson::Value number(7);
  CHECK_FALSE(mg::Codec<mg::ValueRef>::decode(number, decoded));
  CHECK(decoded.id() == "future.value");
}
