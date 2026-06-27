#include <multigauge/value/Value.h>

#include <multigauge/io/Log.h>

#include <algorithm>

namespace mg {

std::unordered_map<std::string, Value*> Value::registry;

Value::Value(const char* id, const char* name, const UnitType& unitType, float minimumValue, float maximumValue) 
    : id(id), name(name), unitType(unitType), value(minimumValue), minimumValue(minimumValue), maximumValue(maximumValue) { 
    registry[id] = this;
}

Value *Value::find(const std::string& id) {
    auto it = registry.find(id);
    if (it != registry.end()) return it->second;

    LOG_WARN("Value::find", "Unknown value id '%s' (returning nullptr)", id.c_str());

    return nullptr;
}

std::vector<const Value*> Value::list() {
    std::vector<const Value*> values;
    values.reserve(registry.size());

    for (const auto& entry : registry) {
        if (entry.second) {
            values.push_back(entry.second);
        }
    }

    std::sort(values.begin(), values.end(), [](const Value* lhs, const Value* rhs) {
        if (!lhs || !rhs) return lhs < rhs;
        return std::string(lhs->getId()) < std::string(rhs->getId());
    });

    return values;
}

Value::operator float() const { return getValueBase(); }

Value &Value::operator=(float newValue) {
    setValueBase(newValue);
    return *this;
}

float Value::getValueBase() const { return value; }

void Value::setValueBase(float newValue) { 
    value = std::clamp(newValue, minimumValue, maximumValue);
    if (onChange) onChange(value);
}

float Value::getMinimumBase() const { return minimumValue; }

float Value::getMaximumBase() const { return maximumValue; }

float Value::getValue(int index) const { return unitType.convertFromBase(value, index); }

void Value::setValue(float newValue, int index) {
    value = std::clamp(unitType.convertToBase(newValue, index), minimumValue, maximumValue);
    if (onChange) onChange(value);
}

float Value::getMinimum(int index) const { return unitType.convertFromBase(minimumValue, index); }

float Value::getMaximum(int index) const { return unitType.convertFromBase(maximumValue, index); }

const char *Value::getId() const { return id; }

const char *Value::getName() const { return name; }

const UnitType& Value::getUnitType() const { return unitType; }

float Value::getInterpolationValue() const { return (value - minimumValue) / (maximumValue - minimumValue); }

std::string Value::getValueString(int index, bool abbreviation) const { return unitType.getValueString(getValue(index), index, abbreviation); }

std::string Value::getLongestValueString(int index, bool abbreviation) {
    std::string minimumString = unitType.getValueString(getMinimum(index), index, abbreviation);
    std::string maximumString = unitType.getValueString(getMaximum(index), index, abbreviation);
    return (maximumString.length() > minimumString.length()) ? maximumString : minimumString;
}

Value dummy("n/a", "n/a", percentage, 0.0f, 100.0f);
Value engineRPM("engineRPM", "RPM", revolutions, 0.0f, 8000.0f);
Value engineCoolantTemp("engineCoolantTemp", "Coolant Temp", temperature, -40.0f, 120.0f);
Value engineOilTemp("engineOilTemp", "Oil Temp", temperature, -40.0f, 120.0f);
Value transmissionTemp("transmissionTemp", "Transmission Temp", temperature, -40.0f, 120.0f);
Value engineOilPressure("engineOilPressure", "Oil Pressure", pressure, 0.0f, 100.0f);
Value transmissionFluidPressure("transmissionFluidPressure", "Transmission Fluid Pressure", pressure, 0.0f, 100.0f);
Value fuelPressure("fuelPressure", "Fuel Pressure", pressure, 0.0f, 100.0f);
Value boostPressure("boostPressure", "Boost Pressure", pressure, 0.0f, 100.0f);
Value fuelLevel("fuelLevel", "Fuel Level", volume, 0.0f, 12.0f);
Value distanceDriven("distanceDriven", "Distance Driven", distance, 0.0f, 999999.0f);
Value speed("speed", "Speed", velocity, 0.0f, 160.0f);
Value verticalAcceleration("verticalAcceleration", "Vertical Accel", acceleration, -4.0f, 4.0f);
Value longitudinalAcceleration("longitudinalAcceleration", "Longitudinal Accel", acceleration, -4.0f, 4.0f);
Value lateralAcceleration("lateralAcceleration", "Lateral Accel", acceleration, -4.0f, 4.0f);
Value calculatedEngineLoad("calculatedEngineLoad", "Engine Load", percentage, 0.0f, 100.0f);
Value throttlePosition("throttlePosition", "Throttle Position", percentage, 0.0f, 100.0f);
Value engineFuelRate("engineFuelRate", "Engine Fuel Rate", volumePerTime, 0.0f, 3212.75f);

//Value engineRunTimeSinceStart("Run Time Since Start", time, 0.0f, 65535.0f); // 65535 is max defined by OBD2 protocol

}
