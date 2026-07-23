#include <doctest/doctest.h>

#include <array>
#include <cstdint>
#include <ostream>
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

constexpr std::array zeroFactorUnits{
  mg::Unit{"base", "", 1.0f, 0.0f, 0},
  mg::Unit{"identity", "id", 0.0f, 12.0f, 3},
};

} // namespace

TEST_CASE("Unit exposes stable defaults and unit aliases") {
  constexpr mg::Unit unit;

  CHECK(unit.name.empty());
  CHECK(unit.abbreviation.empty());
  CHECK(unit.factor == doctest::Approx(1.0f));
  CHECK(unit.offset == doctest::Approx(0.0f));
  CHECK(unit.decimalPlaces == 0);
  CHECK(mg::BASE_UNIT == 0);
  CHECK(sizeof(mg::UnitIndex) == sizeof(std::int8_t));
}

TEST_CASE("UnitType retains its name and unit table") {
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
  CHECK(std::string(torque.name()) == "torque");
}

TEST_CASE("UnitType lookup validates indexes and falls back to base conversions") {
  mg::UnitType distanceLike(
    "distance-like",
    std::span<const mg::Unit>{distanceUnits}
  );

  constexpr auto kNegative = static_cast<mg::UnitIndex>(-1);
  constexpr auto kOutOfRange = static_cast<mg::UnitIndex>(127);

  CHECK(distanceLike.unit(kNegative) == nullptr);
  CHECK(distanceLike.unit(kOutOfRange) == nullptr);
  CHECK(std::string(distanceLike.baseUnit().name) == "meter");
  CHECK(distanceLike.convertFromBase(12.0f, kNegative) == doctest::Approx(12.0f));
  CHECK(distanceLike.convertToBase(12.0f, kOutOfRange) == doctest::Approx(12.0f));
  CHECK(distanceLike.formatValue(12.0f, kNegative, true) == "12.00m");
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

  CHECK(distanceLike.convert(3.5f, 1, 1) == doctest::Approx(3.5f).epsilon(kEpsilon));
  CHECK(distanceLike.convert(3.5f, static_cast<mg::UnitIndex>(127), static_cast<mg::UnitIndex>(127)) == doctest::Approx(3.5f).epsilon(kEpsilon));
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

TEST_CASE("UnitType handles zero factors as identity conversions") {
  mg::UnitType zeroFactor("zero-factor", std::span<const mg::Unit>{zeroFactorUnits});

  CHECK(zeroFactor.convertToBase(42.0f, 1) == doctest::Approx(42.0f));
  CHECK(zeroFactor.convertFromBase(42.0f, 1) == doctest::Approx(12.0f));
  CHECK(zeroFactor.convert(42.0f, 1, 0) == doctest::Approx(42.0f));
  CHECK(zeroFactor.formatValue(42.0f, 1, false) == "42.000");
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
  const std::array expected{
    std::pair{"temperature", &mg::temperature},
    std::pair{"distance", &mg::distance},
    std::pair{"pressure", &mg::pressure},
    std::pair{"velocity", &mg::velocity},
    std::pair{"acceleration", &mg::acceleration},
    std::pair{"volume", &mg::volume},
    std::pair{"volumePerTime", &mg::volumePerTime},
    std::pair{"revolutions", &mg::revolutions},
    std::pair{"angle", &mg::angle},
    std::pair{"percentage", &mg::percentage},
  };

  for (const auto& [name, type] : expected) {
    CHECK(mg::UnitType::find(name) == type);
    CHECK(std::string(type->name()) == name);
    CHECK(type->unit(mg::BASE_UNIT) == &type->baseUnit());
  }

  CHECK(mg::UnitType::find("missing") == nullptr);
  CHECK(mg::UnitType::find("") == nullptr);
  CHECK(mg::UnitType::find("Temperature") == nullptr);
}
