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
    Measurement minimum,
    Measurement maximum
) noexcept : id_(id),
             name_(name),
             unitType_(unitType),
             value_(minimum),
             minimum_(minimum),
             maximum_(maximum) {}

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

Measurement Value::valueBase() const noexcept { return value_; }

void Value::setValueBase(Measurement newValue) noexcept { 
    value_ = std::clamp(newValue, minimum_, maximum_);
}

Measurement Value::minimumBase() const noexcept { return minimum_; }

Measurement Value::maximumBase() const noexcept { return maximum_; }

Measurement Value::value(UnitIndex index) const noexcept { return unitType_.convertFromBase(value_, index); }

void Value::setValue(Measurement newValue, UnitIndex index) noexcept {
    value_ = std::clamp(unitType_.convertToBase(newValue, index), minimum_, maximum_);
}

Measurement Value::minimum(UnitIndex index) const noexcept { return unitType_.convertFromBase(minimum_, index); }

Measurement Value::maximum(UnitIndex index) const noexcept { return unitType_.convertFromBase(maximum_, index); }

std::string_view Value::id() const noexcept { return id_; }

std::string_view Value::name() const noexcept { return name_; }

const UnitType& Value::unitType() const noexcept { return unitType_; }

float Value::interpolationValue() const noexcept {
    const Measurement range = maximum_ - minimum_;
    return range == 0.0F ? 0.5F : (value_ - minimum_) / range;
}

std::string Value::valueString(UnitIndex index, bool abbreviation) const { return unitType_.formatValue(value(index), index, abbreviation); }

std::string Value::longestValueString(UnitIndex index, bool abbreviation) const {
    std::string minimumString = unitType_.formatValue(minimum(index), index, abbreviation);
    std::string maximumString = unitType_.formatValue(maximum(index), index, abbreviation);
    return (maximumString.length() > minimumString.length()) ? maximumString : minimumString;
}

}
