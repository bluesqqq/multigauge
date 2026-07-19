#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <algorithm>
#include <string>

#include <multigauge/value/Value.h>

namespace {

constexpr float kEpsilon = 0.001f;
constexpr mg::UnitIndex kFahrenheit = 1;
constexpr mg::UnitIndex kKelvin = 2;
constexpr mg::UnitIndex kKilopascal = 3;

mg::Value& identityValue() {
  static mg::Value value("test.identity", "Test Identity", mg::percentage, 10.0f, 90.0f);
  value.onChange = nullptr;
  value.setValueBase(10.0f);
  return value;
}

mg::Value& clampValue() {
  static mg::Value value("test.clamp", "Test Clamp", mg::percentage, 10.0f, 90.0f);
  value.onChange = nullptr;
  value.setValueBase(10.0f);
  return value;
}

mg::Value& temperatureValue() {
  static mg::Value value("test.temperature", "Test Temperature", mg::temperature, -40.0f, 120.0f);
  value.onChange = nullptr;
  value.setValueBase(-40.0f);
  return value;
}

mg::Value& convertedClampValue() {
  static mg::Value value("test.converted-clamp", "Test Converted Clamp", mg::temperature, -40.0f, 120.0f);
  value.onChange = nullptr;
  value.setValueBase(-40.0f);
  return value;
}

mg::Value& onChangeValue() {
  static mg::Value value("test.on-change", "Test On Change", mg::percentage, 0.0f, 100.0f);
  value.onChange = nullptr;
  value.setValueBase(0.0f);
  return value;
}

mg::Value& formatValue() {
  static mg::Value value("test.format", "Test Format", mg::pressure, 0.0f, 100.0f);
  value.onChange = nullptr;
  value.setValueBase(0.0f);
  return value;
}

mg::Value& registryValue() {
  static mg::Value value("test.registry", "Test Registry", mg::percentage, 0.0f, 100.0f);
  value.onChange = nullptr;
  value.setValueBase(0.0f);
  return value;
}

} // namespace

TEST_CASE("Value exposes identity, range, unit type, and initial value") {
  auto& value = identityValue();

  CHECK(std::string(value.getId()) == "test.identity");
  CHECK(std::string(value.getName()) == "Test Identity");
  CHECK(&value.getUnitType() == &mg::percentage);
  CHECK(value.getMinimumBase() == doctest::Approx(10.0f));
  CHECK(value.getMaximumBase() == doctest::Approx(90.0f));
  CHECK(value.getValueBase() == doctest::Approx(10.0f));
  CHECK(static_cast<float>(value) == doctest::Approx(10.0f));
}

TEST_CASE("Value clamps base assignments to its configured range") {
  auto& value = clampValue();

  value.setValueBase(45.0f);
  CHECK(value.getValueBase() == doctest::Approx(45.0f));
  CHECK(value.getInterpolationValue() == doctest::Approx(0.4375f));

  value = -25.0f;
  CHECK(value.getValueBase() == doctest::Approx(10.0f));
  CHECK(value.getInterpolationValue() == doctest::Approx(0.0f));

  value.setValueBase(120.0f);
  CHECK(value.getValueBase() == doctest::Approx(90.0f));
  CHECK(value.getInterpolationValue() == doctest::Approx(1.0f));
}

TEST_CASE("Value converts to and from non-base units") {
  auto& value = temperatureValue();

  value.setValue(212.0f, kFahrenheit);

  CHECK(value.getValueBase() == doctest::Approx(100.0f).epsilon(kEpsilon));
  CHECK(value.getValue(kFahrenheit) == doctest::Approx(212.0f).epsilon(kEpsilon));
  CHECK(value.getValue(kKelvin) == doctest::Approx(373.15f).epsilon(kEpsilon));
  CHECK(value.getMinimum(kFahrenheit) == doctest::Approx(-40.0f).epsilon(kEpsilon));
  CHECK(value.getMaximum(kFahrenheit) == doctest::Approx(248.0f).epsilon(kEpsilon));
}

TEST_CASE("Value clamps after converting from a non-base unit") {
  auto& value = convertedClampValue();

  value.setValue(500.0f, kFahrenheit);
  CHECK(value.getValueBase() == doctest::Approx(120.0f));
  CHECK(value.getValue(kFahrenheit) == doctest::Approx(248.0f).epsilon(kEpsilon));
}

TEST_CASE("Value invokes onChange with the stored base value") {
  auto& value = onChangeValue();

  int callCount = 0;
  float lastValue = -1.0f;
  value.onChange = [&](float newValue) {
    ++callCount;
    lastValue = newValue;
  };

  value.setValueBase(42.0f);
  CHECK(callCount == 1);
  CHECK(lastValue == doctest::Approx(42.0f));

  value.setValueBase(150.0f);
  CHECK(callCount == 2);
  CHECK(lastValue == doctest::Approx(100.0f));
}

TEST_CASE("Value formats values through its unit type") {
  auto& value = formatValue();

  value.setValueBase(10.0f);

  CHECK(value.getValueString(mg::BASE_UNIT, true) == "10.0psi");
  CHECK(value.getValueString(mg::BASE_UNIT, false) == "10.0");
  CHECK(value.getValueString(kKilopascal, true) == "68.9kPa");
  CHECK(value.getLongestValueString(mg::BASE_UNIT, true) == "100.0psi");
}

TEST_CASE("Value registry can find values and returns a sorted list") {
  auto& value = registryValue();

  CHECK(mg::Value::find("test.registry") == &value);

  const auto values = mg::Value::list();
  const auto found = std::find(values.begin(), values.end(), &value);

  CHECK(found != values.end());
  CHECK(std::is_sorted(values.begin(), values.end(), [](const mg::Value* lhs, const mg::Value* rhs) {
    return std::string(lhs->getId()) < std::string(rhs->getId());
  }));
}
