#include <doctest/doctest.h>

#include <string>
#include <vector>

#include <multigauge/value/UnitType.h>

namespace {

constexpr float kEpsilon = 0.001f;

} // namespace

TEST_CASE("UnitType always includes the base unit first") {
  mg::UnitType torque(
    "newton-meter",
    "Nm",
    1,
    {
      {"foot-pound", "ft-lb", 0.737562f, 0.0f, 2},
    }
  );

  const auto& units = torque.getUnits();

  REQUIRE(units.size() == 2);
  CHECK(std::string(units[0].name) == "newton-meter");
  CHECK(std::string(units[0].abbreviation) == "Nm");
  CHECK(units[0].factor == doctest::Approx(1.0f));
  CHECK(units[0].offset == doctest::Approx(0.0f));
  CHECK(units[0].decimalPlaces == 1);
  CHECK(&torque.getBaseUnit() == &units[0]);
}

TEST_CASE("UnitType resolves the configured default unit") {
  mg::UnitType distanceLike(
    "meter",
    "m",
    2,
    {
      {"foot", "ft", 3.28084f, 0.0f, 2},
      {"mile", "mi", 0.00062137f, 0.0f, 1},
    },
    2
  );

  CHECK(std::string(distanceLike.getDefaultUnit().name) == "mile");
  CHECK(std::string(distanceLike.getUnit(mg::DEFAULT_UNIT).name) == "mile");
  CHECK(std::string(distanceLike.getUnit(-99).name) == "mile");
  CHECK(std::string(distanceLike.getUnit(99).name) == "mile");

  distanceLike.setDefaultUnit(1);
  CHECK(std::string(distanceLike.getDefaultUnit().name) == "foot");

  distanceLike.setDefaultUnit(99);
  CHECK(std::string(distanceLike.getDefaultUnit().name) == "meter");
}

TEST_CASE("UnitType converts between base and conversion units") {
  mg::UnitType temperatureLike(
    "celsius",
    "C",
    2,
    {
      {"fahrenheit", "F", 1.8f, 32.0f, 2},
      {"kelvin", "K", 1.0f, 273.15f, 2},
    }
  );

  const int celsius = 0;
  const int fahrenheit = 1;
  const int kelvin = 2;

  CHECK(temperatureLike.convertFromBase(100.0f, fahrenheit) == doctest::Approx(212.0f).epsilon(kEpsilon));
  CHECK(temperatureLike.convertToBase(212.0f, fahrenheit) == doctest::Approx(100.0f).epsilon(kEpsilon));
  CHECK(temperatureLike.convert(32.0f, fahrenheit, celsius) == doctest::Approx(0.0f).epsilon(kEpsilon));
  CHECK(temperatureLike.convert(0.0f, celsius, kelvin) == doctest::Approx(273.15f).epsilon(kEpsilon));
}

TEST_CASE("UnitType falls back to default unit for invalid conversion indexes") {
  mg::UnitType distanceLike(
    "meter",
    "m",
    2,
    {
      {"foot", "ft", 3.28084f, 0.0f, 2},
    },
    1
  );

  CHECK(distanceLike.convertFromBase(1.0f, 999) == doctest::Approx(3.28084f).epsilon(kEpsilon));
  CHECK(distanceLike.convertToBase(3.28084f, 999) == doctest::Approx(1.0f).epsilon(kEpsilon));
}

TEST_CASE("UnitType formats values with unit decimal places and optional abbreviation") {
  mg::UnitType pressureLike(
    "psi",
    "psi",
    1,
    {
      {"kpa", "kpa", 6.89476f, 0.0f, 1},
      {"bar", "bar", 0.0689476f, 0.0f, 4},
    }
  );

  CHECK(pressureLike.getValueString(10.0f, 0, true) == "10.0psi");
  CHECK(pressureLike.getValueString(10.0f, 0, false) == "10.0");
  CHECK(pressureLike.getValueString(68.9476f, 1, true) == "68.9kpa");
  CHECK(pressureLike.getValueString(0.689476f, 2, true) == "0.6895bar");
}

TEST_CASE("UnitType lists unit names and abbreviations in index order") {
  mg::UnitType speedLike(
    "kilometer-per-hour",
    "km/h",
    2,
    {
      {"mile-per-hour", "mph", 0.621371f, 0.0f, 1},
    }
  );

  const std::vector<const char*> names = speedLike.listUnitStrings(false);
  const std::vector<const char*> abbreviations = speedLike.listUnitStrings(true);

  REQUIRE(names.size() == 2);
  REQUIRE(abbreviations.size() == 2);
  CHECK(std::string(names[0]) == "kilometer-per-hour");
  CHECK(std::string(names[1]) == "mile-per-hour");
  CHECK(std::string(abbreviations[0]) == "km/h");
  CHECK(std::string(abbreviations[1]) == "mph");
}

TEST_CASE("UnitType finds units by name and abbreviation") {
  mg::UnitType speedLike(
    "kilometer-per-hour",
    "km/h",
    2,
    {
      {"mile-per-hour", "mph", 0.621371f, 0.0f, 1},
    }
  );

  CHECK(speedLike.getIndexFromName("kilometer-per-hour") == 0);
  CHECK(speedLike.getIndexFromName("mile-per-hour") == 1);
  CHECK(speedLike.getIndexFromName("missing") == mg::DEFAULT_UNIT);
  CHECK(speedLike.getIndexFromName(nullptr) == mg::DEFAULT_UNIT);

  CHECK(speedLike.getIndexFromAbbreviation("km/h") == 0);
  CHECK(speedLike.getIndexFromAbbreviation("mph") == 1);
  CHECK(speedLike.getIndexFromAbbreviation("missing") == mg::DEFAULT_UNIT);
  CHECK(speedLike.getIndexFromAbbreviation(nullptr) == mg::DEFAULT_UNIT);
}

TEST_CASE("UnitType find resolves known global unit types and falls back to percentage") {
  CHECK(&mg::UnitType::find("temperature") == &mg::temperature);
  CHECK(&mg::UnitType::find("pressure") == &mg::pressure);
  CHECK(&mg::UnitType::find("velocity") == &mg::velocity);
  CHECK(&mg::UnitType::find(nullptr) == &mg::percentage);
  CHECK(&mg::UnitType::find("missing") == &mg::percentage);
}
