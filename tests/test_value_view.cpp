#include <doctest/doctest.h>

#include <ostream>
#include <rapidjson/document.h>

#include <multigauge/properties/PropertyCodec.h>
#include <multigauge/value/ValueView.h>

namespace {

constexpr float kEpsilon = 0.001f;

class ValueRestore {
public:
  explicit ValueRestore(mg::Value& value) : value_(value), original_(value.valueBase()) {}
  ~ValueRestore() { value_.setValueBase(original_); }

private:
  mg::Value& value_;
  mg::Measurement original_;
};

rapidjson::Document documentWithViewProperties(const char* id, float minimum, float maximum, int unitIndex) {
  rapidjson::Document document;
  document.SetObject();
  auto& allocator = document.GetAllocator();
  document.AddMember("id", rapidjson::Value(id, allocator), allocator);
  document.AddMember("min", minimum, allocator);
  document.AddMember("max", maximum, allocator);
  document.AddMember("unitIndex", unitIndex, allocator);
  return document;
}

} // namespace

TEST_CASE("ValueView exposes unresolved defaults") {
  const mg::ValueView view;

  CHECK(view.valueBase() == doctest::Approx(0.0f));
  CHECK(view.value() == doctest::Approx(0.0f));
  CHECK(view.minimumBase() == doctest::Approx(0.0f));
  CHECK(view.maximumBase() == doctest::Approx(1.0f));
  CHECK(view.minimum() == doctest::Approx(0.0f));
  CHECK(view.maximum() == doctest::Approx(1.0f));
  CHECK(view.interpolationValue() == doctest::Approx(0.5f));
  CHECK(view.unit() == nullptr);
  CHECK(std::string(view.name()) == "n/a");
  CHECK(view.valueString() == "n/a");
}

TEST_CASE("ValueView applies custom limits consistently in selected units") {
  mg::Value* temperature = mg::Value::find("engineCoolantTemp");
  REQUIRE(temperature != nullptr);
  ValueRestore restore(*temperature);
  temperature->setValueBase(110.0f);

  rapidjson::Document document = documentWithViewProperties("engineCoolantTemp", 20.0f, 80.0f, 1);
  mg::ValueView view;
  REQUIRE(mg::decodeAny(document, view));

  CHECK(view.minimumBase() == doctest::Approx(20.0f));
  CHECK(view.maximumBase() == doctest::Approx(80.0f));
  CHECK(view.valueBase() == doctest::Approx(80.0f));
  CHECK(view.minimum() == doctest::Approx(68.0f).epsilon(kEpsilon));
  CHECK(view.maximum() == doctest::Approx(176.0f).epsilon(kEpsilon));
  CHECK(view.value() == doctest::Approx(176.0f).epsilon(kEpsilon));
  CHECK(view.interpolationValue() == doctest::Approx(1.0f));
  REQUIRE(view.unit() != nullptr);
  CHECK(view.unit()->abbreviation == "F");
  CHECK(view.valueString(true) == "176.00F");
  CHECK(std::string(view.name()) == "Coolant Temp");
}

TEST_CASE("ValueView resets optional properties and falls back from invalid units") {
  mg::Value* temperature = mg::Value::find("engineCoolantTemp");
  REQUIRE(temperature != nullptr);
  ValueRestore restore(*temperature);
  temperature->setValueBase(30.0f);

  rapidjson::Document document = documentWithViewProperties("engineCoolantTemp", 20.0f, 80.0f, 127);
  mg::ValueView view;
  REQUIRE(mg::decodeAny(document, view));
  REQUIRE(view.unit() != nullptr);
  CHECK(view.unit()->abbreviation == "C");
  CHECK(view.value() == doctest::Approx(30.0f));

  rapidjson::Value nullValue(rapidjson::kNullType);
  CHECK(view.setProperty("min", nullValue));
  CHECK(view.setProperty("max", nullValue));
  CHECK(view.setProperty("unitIndex", nullValue));
  CHECK(view.minimumBase() == doctest::Approx(-40.0f));
  CHECK(view.maximumBase() == doctest::Approx(120.0f));
  CHECK(view.value() == doctest::Approx(30.0f));
  CHECK_FALSE(view.setProperty("missing", nullValue));
}

TEST_CASE("ValueView supports shorthand and property-object serialization") {
  rapidjson::Document document;
  document.SetObject();
  auto& allocator = document.GetAllocator();

  mg::ValueView shorthand;
  rapidjson::Value rpm("engineRPM", allocator);
  REQUIRE(mg::Codec<mg::ValueView>::decode(rpm, shorthand));
  CHECK(std::string(shorthand.name()) == "RPM");

  rapidjson::Value encodedShorthand;
  REQUIRE(mg::encodeAny(encodedShorthand, allocator, shorthand));
  REQUIRE(encodedShorthand.IsString());
  CHECK(std::string(encodedShorthand.GetString(), encodedShorthand.GetStringLength()) == "engineRPM");

  rapidjson::Document properties = documentWithViewProperties("engineRPM", 1000.0f, 7000.0f, 0);
  mg::ValueView configured;
  REQUIRE(mg::decodeAny(properties, configured));
  rapidjson::Value encodedProperties;
  REQUIRE(mg::encodeAny(encodedProperties, allocator, configured));
  REQUIRE(encodedProperties.IsObject());
  CHECK(std::string(encodedProperties["id"].GetString(), encodedProperties["id"].GetStringLength()) == "engineRPM");
  CHECK(encodedProperties["min"].GetFloat() == doctest::Approx(1000.0f));
  CHECK(encodedProperties["max"].GetFloat() == doctest::Approx(7000.0f));
  CHECK(encodedProperties["unitIndex"].GetInt() == 0);

  mg::ValueView unknown("future.value");
  rapidjson::Value encodedUnknown;
  REQUIRE(mg::encodeAny(encodedUnknown, allocator, unknown));
  REQUIRE(encodedUnknown.IsString());
  CHECK(std::string(encodedUnknown.GetString(), encodedUnknown.GetStringLength()) == "future.value");
}

TEST_CASE("ValueView rejects unsupported shorthand JSON") {
  rapidjson::Document document;
  document.SetObject();
  mg::ValueView view("engineRPM");

  rapidjson::Value number(1);
  CHECK_FALSE(mg::Codec<mg::ValueView>::decode(number, view));
  CHECK(std::string(view.name()) == "RPM");
  CHECK_FALSE(mg::decodeAny(number, view));
}
