#include <multigauge/value/Value.h>

#include <multigauge/io/Log.h>

#include <algorithm>
#include <array>

namespace mg {

namespace {

std::array<Value, 18> values{{
    Value{"n/a",                       "n/a",                         percentage,    0.0f,   100.0f},
    Value{"engineRPM",                 "RPM",                         revolutions,   0.0f,   8000.0f},
    Value{"engineCoolantTemp",         "Coolant Temp",                temperature,   -40.0f, 120.0f},
    Value{"engineOilTemp",             "Oil Temp",                    temperature,   -40.0f, 120.0f},
    Value{"transmissionTemp",          "Transmission Temp",           temperature,   -40.0f, 120.0f},
    Value{"engineOilPressure",         "Oil Pressure",                pressure,      0.0f,   100.0f},
    Value{"transmissionFluidPressure", "Transmission Fluid Pressure", pressure,      0.0f,   100.0f},
    Value{"fuelPressure",              "Fuel Pressure",               pressure,      0.0f,   100.0f},
    Value{"boostPressure",             "Boost Pressure",              pressure,      0.0f,   100.0f},
    Value{"fuelLevel",                 "Fuel Level",                  volume,        0.0f,   12.0f},
    Value{"distanceDriven",            "Distance Driven",             distance,      0.0f,   999999.0f},
    Value{"speed",                     "Speed",                       velocity,      0.0f,   160.0f},
    Value{"verticalAcceleration",      "Vertical Accel",              acceleration,  -4.0f,  4.0f},
    Value{"longitudinalAcceleration",  "Longitudinal Accel",          acceleration,  -4.0f,  4.0f},
    Value{"lateralAcceleration",       "Lateral Accel",               acceleration,  -4.0f,  4.0f},
    Value{"calculatedEngineLoad",      "Engine Load",                 percentage,    0.0f,   100.0f},
    Value{"throttlePosition",          "Throttle Position",           percentage,    0.0f,   100.0f},
    Value{"engineFuelRate",            "Engine Fuel Rate",            volumePerTime, 0.0f,   3212.75f}
}};

}

Value::Value(
    std::string_view id,
    std::string_view name,
    const UnitType& unitType,
    float minimumValue,
    float maximumValue
) noexcept : id_(id),
             name_(name),
             unitType_(unitType),
             value(minimumValue),
             minimumValue(minimumValue),
             maximumValue(maximumValue) {}

Value *Value::find(std::string_view id) noexcept {
    for (auto& value : values) {
        if (value.id() == id) return &value;
    }

    LOG_WARN(
        "Value::find",
        "Unknown value id '%.*s' (returning nullptr)",
        static_cast<int>(id.size()),
        id.data()
    );

    return nullptr;
}

std::span<const Value> Value::list() noexcept {
    return values;
}

Value::operator float() const {
    return getValueBase();
}

Value &Value::operator=(float newValue) {
    setValueBase(newValue);
    return *this;
}

float Value::getValueBase() const noexcept { return value; }

void Value::setValueBase(float newValue) noexcept { 
    value = std::clamp(newValue, minimumValue, maximumValue);
}

float Value::getMinimumBase() const noexcept { return minimumValue; }

float Value::getMaximumBase() const noexcept { return maximumValue; }

float Value::getValue(int index) const { return unitType_.convertFromBase(value, index); }

void Value::setValue(float newValue, int index) {
    value = std::clamp(unitType_.convertToBase(newValue, index), minimumValue, maximumValue);
}

float Value::getMinimum(int index) const { return unitType_.convertFromBase(minimumValue, index); }

float Value::getMaximum(int index) const { return unitType_.convertFromBase(maximumValue, index); }

std::string_view Value::id() const { return id_; }

std::string_view Value::name() const { return name_; }

const UnitType& Value::unitType() const noexcept { return unitType_; }

float Value::getInterpolationValue() const { return (value - minimumValue) / (maximumValue - minimumValue); }

std::string Value::getValueString(int index, bool abbreviation) const { return unitType_.formatValue(getValue(index), index, abbreviation); }

std::string Value::getLongestValueString(int index, bool abbreviation) {
    std::string minimumString = unitType_.formatValue(getMinimum(index), index, abbreviation);
    std::string maximumString = unitType_.formatValue(getMaximum(index), index, abbreviation);
    return (maximumString.length() > minimumString.length()) ? maximumString : minimumString;
}

}
