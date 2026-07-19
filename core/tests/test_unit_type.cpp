#include <doctest/doctest.h>

#include <array>
#include <string>

#include <multigauge/value/UnitType.h>

namespace {

constexpr float kEpsilon = 0.001f;

constexpr std::array torqueUnits{
  mg::Unit{"newton-meter", "Nm", 1.0f, 0.0f, 1},
  mg::Unit{"foot-pound", "ft-lb", 0.737562f, 0.0f, 2},
};

constexpr std::array distanceUnits{
  mg::Unit{"meter", "m", 1.0f, 0.0f, 2},
  mg::Unit{"foot", "ft", 3.28084f, 0.0f, 2},
  mg::Unit{"mile", "mi", 0.00062137f, 0.0f, 1},
};

constexpr std::array temperatureUnits{
  mg::Unit{"celsius", "C", 1.0f, 0.0f, 2},
  mg::Unit{"fahrenheit", "F", 1.8f, 32.0f, 2},
  mg::Unit{"kelvin", "K", 1.0f, 273.15f, 2},
};

constexpr std::array pressureUnits{
  mg::Unit{"psi", "psi", 1.0f, 0.0f, 1},
  mg::Unit{"kpa", "kpa", 6.89476f, 0.0f, 1},
  mg::Unit{"bar", "bar", 0.0689476f, 0.0f, 4},
};

constexpr std::array speedUnits{
  mg::Unit{"kilometer-per-hour", "km/h", 1.0f, 0.0f, 2},
  mg::Unit{"mile-per-hour", "mph", 0.621371f, 0.0f, 1},
};

} // namespace

TEST_CASE("UnitType always includes the base unit first") {
  mg::UnitType torque(
    "torque",
    std::span<const mg::Unit>{torqueUnits}
  );

  const auto units = torque.units();

  REQUIRE(units.size() == 2);
  CHECK(&units[0] == &torqueUnits[0]);
  CHECK(std::string(units[0].name) == "newton-meter");
  CHECK(std::string(units[0].abbreviation) == "Nm");
  CHECK(units[0].factor == doctest::Approx(1.0f));
  CHECK(units[0].offset == doctest::Approx(0.0f));
  CHECK(units[0].decimalPlaces == 1);
  CHECK(&torque.baseUnit() == &units[0]);
}

TEST_CASE("UnitType falls back to the base unit for invalid indexes") {
  mg::UnitType distanceLike(
    "distance-like",
    std::span<const mg::Unit>{distanceUnits}
  );

  CHECK(std::string(distanceLike.unit(mg::DEFAULT_UNIT).name) == "meter");
  CHECK(std::string(distanceLike.unit(-99).name) == "meter");
  CHECK(std::string(distanceLike.unit(99).name) == "meter");
}

TEST_CASE("UnitType converts between base and conversion units") {
  mg::UnitType temperatureLike(
    "temperature-like",
    std::span<const mg::Unit>{temperatureUnits}
  );

  const mg::UnitIndex celsius = 0;
  const mg::UnitIndex fahrenheit = 1;
  const mg::UnitIndex kelvin = 2;

  CHECK(temperatureLike.convertFromBase(100.0f, fahrenheit) == doctest::Approx(212.0f).epsilon(kEpsilon));
  CHECK(temperatureLike.convertToBase(212.0f, fahrenheit) == doctest::Approx(100.0f).epsilon(kEpsilon));
  CHECK(temperatureLike.convert(32.0f, fahrenheit, celsius) == doctest::Approx(0.0f).epsilon(kEpsilon));
  CHECK(temperatureLike.convert(0.0f, celsius, kelvin) == doctest::Approx(273.15f).epsilon(kEpsilon));
}

TEST_CASE("UnitType falls back to base unit for invalid conversion indexes") {
  mg::UnitType distanceLike(
    "distance-like",
    std::span<const mg::Unit>(distanceUnits.data(), 2)
  );

  CHECK(distanceLike.convertFromBase(1.0f, 999) == doctest::Approx(1.0f).epsilon(kEpsilon));
  CHECK(distanceLike.convertToBase(3.28084f, 999) == doctest::Approx(3.28084f).epsilon(kEpsilon));
}

TEST_CASE("UnitType formats values with unit decimal places and optional abbreviation") {
  mg::UnitType pressureLike(
    "pressure-like",
    std::span<const mg::Unit>{pressureUnits}
  );

  CHECK(pressureLike.formatValue(10.0f, 0, true) == "10.0psi");
  CHECK(pressureLike.formatValue(10.0f, 0, false) == "10.0");
  CHECK(pressureLike.formatValue(68.9476f, 1, true) == "68.9kpa");
  CHECK(pressureLike.formatValue(0.689476f, 2, true) == "0.6895bar");
}

TEST_CASE("UnitType exposes unit names and abbreviations in index order") {
  mg::UnitType speedLike(
    "speed-like",
    std::span<const mg::Unit>{speedUnits}
  );

  const auto units = speedLike.units();

  REQUIRE(units.size() == 2);
  CHECK(std::string(units[0].name) == "kilometer-per-hour");
  CHECK(std::string(units[1].name) == "mile-per-hour");
  CHECK(std::string(units[0].abbreviation) == "km/h");
  CHECK(std::string(units[1].abbreviation) == "mph");
}

TEST_CASE("UnitType find resolves known global unit types and returns nullptr for missing names") {
  CHECK(mg::UnitType::find("temperature") == &mg::temperature);
  CHECK(mg::UnitType::find("pressure") == &mg::pressure);
  CHECK(mg::UnitType::find("velocity") == &mg::velocity);
  CHECK(mg::UnitType::find("missing") == nullptr);
}
