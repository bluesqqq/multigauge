#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <algorithm>
#include <array>
#include <ostream>
#include <string>

#include <multigauge/value/Value.h>

namespace {

constexpr float kEpsilon = 0.001f;
constexpr mg::UnitIndex kFahrenheit = 1;
constexpr mg::UnitIndex kKelvin = 2;
constexpr mg::UnitIndex kKilopascal = 3;

mg::Value& identityValue() {
  static mg::Value value("test.identity", "Test Identity", mg::percentage, 10.0f, 90.0f);
  value.setValueBase(10.0f);
  return value;
}

mg::Value& clampValue() {
  static mg::Value value("test.clamp", "Test Clamp", mg::percentage, 10.0f, 90.0f);
  value.setValueBase(10.0f);
  return value;
}

mg::Value& temperatureValue() {
  static mg::Value value("test.temperature", "Test Temperature", mg::temperature, -40.0f, 120.0f);
  value.setValueBase(-40.0f);
  return value;
}

mg::Value& convertedClampValue() {
  static mg::Value value("test.converted-clamp", "Test Converted Clamp", mg::temperature, -40.0f, 120.0f);
  value.setValueBase(-40.0f);
  return value;
}

mg::Value& formatValue() {
  static mg::Value value("test.format", "Test Format", mg::pressure, 0.0f, 100.0f);
  value.setValueBase(0.0f);
  return value;
}

} // namespace

TEST_CASE("Value exposes identity, range, unit type, and initial value") {
  auto& value = identityValue();

  CHECK(std::string(value.id()) == "test.identity");
  CHECK(std::string(value.name()) == "Test Identity");
  CHECK(&value.unitType() == &mg::percentage);
  CHECK(value.minimumBase() == doctest::Approx(10.0f));
  CHECK(value.maximumBase() == doctest::Approx(90.0f));
  CHECK(value.valueBase() == doctest::Approx(10.0f));
}

TEST_CASE("Value handles a zero-width range") {
  mg::Value value("test.fixed", "Fixed", mg::percentage, 42.0f, 42.0f);

  CHECK(value.valueBase() == doctest::Approx(42.0f));
  CHECK(value.interpolationValue() == doctest::Approx(0.5f));
  value.setValueBase(-100.0f);
  CHECK(value.valueBase() == doctest::Approx(42.0f));
  value.setValue(999.0f, kFahrenheit);
  CHECK(value.valueBase() == doctest::Approx(42.0f));
}

TEST_CASE("Value clamps base assignments to its configured range") {
  auto& value = clampValue();

  value.setValueBase(45.0f);
  CHECK(value.valueBase() == doctest::Approx(45.0f));
  CHECK(value.interpolationValue() == doctest::Approx(0.4375f));

  value.setValueBase(-25.0f);
  CHECK(value.valueBase() == doctest::Approx(10.0f));
  CHECK(value.interpolationValue() == doctest::Approx(0.0f));

  value.setValueBase(120.0f);
  CHECK(value.valueBase() == doctest::Approx(90.0f));
  CHECK(value.interpolationValue() == doctest::Approx(1.0f));
}

TEST_CASE("Value converts to and from non-base units") {
  auto& value = temperatureValue();

  value.setValue(212.0f, kFahrenheit);

  CHECK(value.valueBase() == doctest::Approx(100.0f).epsilon(kEpsilon));
  CHECK(value.value(kFahrenheit) == doctest::Approx(212.0f).epsilon(kEpsilon));
  CHECK(value.value(kKelvin) == doctest::Approx(373.15f).epsilon(kEpsilon));
  CHECK(value.minimum(kFahrenheit) == doctest::Approx(-40.0f).epsilon(kEpsilon));
  CHECK(value.maximum(kFahrenheit) == doctest::Approx(248.0f).epsilon(kEpsilon));
}

TEST_CASE("Value clamps after converting from a non-base unit") {
  auto& value = convertedClampValue();

  value.setValue(500.0f, kFahrenheit);
  CHECK(value.valueBase() == doctest::Approx(120.0f));
  CHECK(value.value(kFahrenheit) == doctest::Approx(248.0f).epsilon(kEpsilon));
}

TEST_CASE("Value formats values through its unit type") {
  auto& value = formatValue();

  value.setValueBase(10.0f);

  CHECK(value.valueString(mg::BASE_UNIT, true) == "10.0psi");
  CHECK(value.valueString(kKilopascal, true) == "68.9kPa");
  CHECK(value.longestValueString(mg::BASE_UNIT, true) == "100.0psi");
  CHECK(value.valueString(mg::BASE_UNIT, false) == "10.0");
  CHECK(value.value(static_cast<mg::UnitIndex>(127)) == doctest::Approx(10.0f));
  CHECK(value.minimum(static_cast<mg::UnitIndex>(-1)) == doctest::Approx(0.0f));
  CHECK(value.maximum(static_cast<mg::UnitIndex>(127)) == doctest::Approx(100.0f));
}

TEST_CASE("Value registry can find built-in values and returns a fixed list") {
  const auto values = mg::Value::list();
  const std::array expectedIds{
    "n/a", "engineRPM", "engineCoolantTemp", "engineOilTemp", "transmissionTemp", "engineOilPressure",
    "transmissionFluidPressure", "fuelPressure", "boostPressure", "fuelLevel", "distanceDriven", "speed",
    "verticalAcceleration", "longitudinalAcceleration", "lateralAcceleration", "calculatedEngineLoad",
    "throttlePosition", "engineFuelRate",
  };

  REQUIRE(values.size() == expectedIds.size());
  for (std::size_t index = 0; index < values.size(); ++index) {
    const mg::Value& listed = values[index];
    CHECK(std::string(listed.id()) == expectedIds[index]);
    CHECK(mg::Value::find(listed.id()) == &listed);
  }

  CHECK(mg::Value::find("missing") == nullptr);
  CHECK(mg::Value::find("") == nullptr);
}
