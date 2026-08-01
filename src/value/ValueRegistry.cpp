#include <multigauge/value/ValueRegistry.h>

#include <etl/string.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>

namespace mg {
namespace {

struct UserValue {
    etl::string<ValueRegistry::MaxUserIdLength> id;
    etl::string<ValueRegistry::MaxUserNameLength> name;
    const UnitType* unit = nullptr;
    Measurement minimum = 0.0F;
    Measurement maximum = 0.0F;
    Measurement measurement = std::numeric_limits<Measurement>::quiet_NaN();
    std::uint16_t generation = 0;
    bool occupied = false;
};

constexpr Measurement unavailable = std::numeric_limits<Measurement>::quiet_NaN();

const std::array<ValueDefinition, ValueRegistry::BuiltInCount> builtInDefinitions{{
    {"n/a",                       "n/a",                         &percentage,    0.0F,   100.0F},
    {"engineRPM",                 "RPM",                         &revolutions,   0.0F,  8000.0F},
    {"engineCoolantTemp",         "Coolant Temp",                &temperature,  -40.0F,  120.0F},
    {"engineOilTemp",             "Oil Temp",                    &temperature,  -40.0F,  120.0F},
    {"transmissionTemp",          "Transmission Temp",           &temperature,  -40.0F,  120.0F},
    {"engineOilPressure",         "Oil Pressure",                &pressure,      0.0F,   100.0F},
    {"transmissionFluidPressure", "Transmission Fluid Pressure", &pressure,      0.0F,   100.0F},
    {"fuelPressure",              "Fuel Pressure",               &pressure,      0.0F,   100.0F},
    {"boostPressure",             "Boost Pressure",              &pressure,      0.0F,   100.0F},
    {"fuelLevel",                 "Fuel Level",                  &volume,        0.0F,    12.0F},
    {"distanceDriven",            "Distance Driven",             &distance,      0.0F, 999999.0F},
    {"speed",                     "Speed",                       &velocity,      0.0F,   160.0F},
    {"verticalAcceleration",      "Vertical Accel",              &acceleration, -4.0F,     4.0F},
    {"longitudinalAcceleration",  "Longitudinal Accel",          &acceleration, -4.0F,     4.0F},
    {"lateralAcceleration",       "Lateral Accel",               &acceleration, -4.0F,     4.0F},
    {"calculatedEngineLoad",      "Engine Load",                 &percentage,    0.0F,   100.0F},
    {"throttlePosition",          "Throttle Position",           &percentage,    0.0F,   100.0F},
    {"engineFuelRate",            "Engine Fuel Rate",            &volumePerTime, 0.0F,  3212.75F},
}};

std::array<Measurement, ValueRegistry::BuiltInCount> builtInMeasurements = [] {
    std::array<Measurement, ValueRegistry::BuiltInCount> values{};
    for (std::size_t index = 0; index < values.size(); ++index) values[index] = builtInDefinitions[index].minimum;
    return values;
}();
std::array<UserValue, ValueRegistry::MaxUserValues> userValues{};
std::size_t userValueCount = 0;

bool isBuiltIn(ValueHandle handle) noexcept {
    return handle.isBuiltIn() && handle.builtInId() < ValueRegistry::BuiltInCount;
}

UserValue* findUser(ValueHandle handle) noexcept {
    if (!handle.isUser() || handle.userSlot() >= userValues.size()) {
        return nullptr;
    }

    UserValue& value = userValues[handle.userSlot()];
    return value.occupied && value.generation == handle.userGeneration() ? &value : nullptr;
}

Measurement* measurement(ValueHandle handle) noexcept {
    if (isBuiltIn(handle)) {
        return &builtInMeasurements[handle.builtInId()];
    }

    UserValue* user = findUser(handle);
    return user ? &user->measurement : nullptr;
}

void clearUser(UserValue& value) noexcept {
    value.id.clear();
    value.name.clear();
    value.unit = nullptr;
    value.minimum = value.maximum = 0.0F;
    value.measurement = unavailable;
    value.occupied = false;
    value.generation = static_cast<std::uint16_t>((value.generation + 1U) & 0x07FFU);
}

std::string_view stringView(const auto& value) noexcept {
    return {value.data(), value.size()};
}

}

ValueHandle ValueRegistry::resolve(std::string_view valueId) noexcept {
    if (valueId.empty()) {
        return ValueHandle::invalid();
    }

    for (std::size_t i = 0; i < BuiltInCount; ++i) {
        if (builtInDefinitions[i].id == valueId) {
            return ValueHandle::builtIn(static_cast<ValueHandle::Storage>(i));
        }
    }

    for (std::size_t i = 0; i < userValues.size(); ++i) {
        if (userValues[i].occupied && stringView(userValues[i].id) == valueId) {
            return ValueHandle::user(static_cast<std::uint8_t>(i), userValues[i].generation);
        }
    }

    return ValueHandle::invalid();
}

bool ValueRegistry::exists(ValueHandle handle) noexcept {
    return isBuiltIn(handle) || findUser(handle) != nullptr;
}

std::string_view ValueRegistry::id(ValueHandle handle) noexcept {
    if (isBuiltIn(handle)) {
        return builtInDefinitions[handle.builtInId()].id;
    }

    UserValue* user = findUser(handle);
    return user ? stringView(user->id) : std::string_view{};
}

std::string_view ValueRegistry::name(ValueHandle handle) noexcept {
    if (isBuiltIn(handle)) {
        return builtInDefinitions[handle.builtInId()].name;
    }

    UserValue* user = findUser(handle);
    return user ? stringView(user->name) : std::string_view{};
}

const UnitType* ValueRegistry::unit(ValueHandle handle) noexcept {
    if (isBuiltIn(handle)) {
        return builtInDefinitions[handle.builtInId()].unit;
    }

    UserValue* user = findUser(handle);
    return user ? user->unit : nullptr;
}

Measurement ValueRegistry::minimum(ValueHandle handle) noexcept {
    if (isBuiltIn(handle)) {
        return builtInDefinitions[handle.builtInId()].minimum;
    }

    UserValue* user = findUser(handle);
    return user ? user->minimum : 0.0F;
}

Measurement ValueRegistry::maximum(ValueHandle handle) noexcept {
    if (isBuiltIn(handle)) {
        return builtInDefinitions[handle.builtInId()].maximum;
    }

    UserValue* user = findUser(handle);
    return user ? user->maximum : 1.0F;
}

Measurement ValueRegistry::value(ValueHandle handle) noexcept {
    Measurement* v = measurement(handle);
    return v ? *v : unavailable;
}

Measurement ValueRegistry::value(BuiltInValue value) noexcept {
    return ValueRegistry::value(handle(value));
}

bool ValueRegistry::available(ValueHandle handle) noexcept {
    Measurement* v = measurement(handle);
    return v && !std::isnan(*v);
}

bool ValueRegistry::set(ValueHandle handle, Measurement newValue) noexcept {
    Measurement* destination = measurement(handle);
    if (!destination) {
        return false;
    }

    *destination = std::clamp(newValue, minimum(handle), maximum(handle));
    return true;
}

bool ValueRegistry::set(BuiltInValue value, Measurement newValue) noexcept {
    return set(handle(value), newValue);
}

bool ValueRegistry::invalidate(ValueHandle handle) noexcept {
    Measurement* destination = measurement(handle);
    if (!destination) {
        return false;
    }

    *destination = unavailable;
    return true;
}

bool ValueRegistry::invalidate(BuiltInValue value) noexcept {
    return invalidate(handle(value));
}

const ValueDefinition* ValueRegistry::definition(BuiltInValue value) noexcept {
    const auto index = static_cast<std::size_t>(value);
    return index < BuiltInCount ? &builtInDefinitions[index] : nullptr;
}

ValueHandle ValueRegistry::add(std::string_view valueId, std::string_view valueName, const UnitType& valueUnit, Measurement minimumValue, Measurement maximumValue) noexcept {
    if (valueId.empty() || valueId.size() > MaxUserIdLength || valueName.size() > MaxUserNameLength ||
        minimumValue > maximumValue || full() || resolve(valueId).valid()) {
        return ValueHandle::invalid();
    }

    for (std::size_t i = 0; i < userValues.size(); ++i) {
        if (!userValues[i].occupied) {
            UserValue& user = userValues[i];
            user.id.assign(valueId.data(), valueId.size());
            user.name.assign(valueName.data(), valueName.size());
            user.unit = &valueUnit;
            user.minimum = minimumValue;
            user.maximum = maximumValue;
            user.measurement = unavailable;
            user.occupied = true;
            ++userValueCount;
            return ValueHandle::user(static_cast<std::uint8_t>(i), user.generation);
        }
    }

    return ValueHandle::invalid();
}

bool ValueRegistry::remove(ValueHandle handle) noexcept {
    UserValue* user = findUser(handle);
    if (!user) {
        return false;
    }

    clearUser(*user);
    --userValueCount;
    return true;
}

bool ValueRegistry::remove(std::string_view valueId) noexcept {
    return remove(resolve(valueId));
}

void ValueRegistry::clearUsers() noexcept {
    for (UserValue& user : userValues) {
        if (user.occupied) {
            clearUser(user);
        }
    }

    userValueCount = 0;
}

std::size_t ValueRegistry::userCount() noexcept {
    return userValueCount;
}

std::size_t ValueRegistry::size() noexcept {
    return BuiltInCount + userValueCount;
}

bool ValueRegistry::full() noexcept {
    return userValueCount >= MaxUserValues;
}

ValueHandle ValueRegistry::userHandleAt(std::size_t slot) noexcept {
    if (slot >= userValues.size() || !userValues[slot].occupied) {
        return ValueHandle::invalid();
    }

    return ValueHandle::user(static_cast<std::uint8_t>(slot), userValues[slot].generation);
}

} // namespace mg
