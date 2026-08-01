#pragma once

#include <multigauge/value/UnitType.h>
#include <multigauge/value/ValueHandle.h>
#include <multigauge/value/ValueDefinition.h>

#include <cstddef>
#include <string_view>

namespace mg {

/* ----- BUILT-IN VALUES ----- */

/// Built-in value IDs.
enum class BuiltInValue : ValueHandle::Storage {
    notApplicable = 0,
    engineRPM,
    engineCoolantTemp,
    engineOilTemp,
    transmissionTemp,
    engineOilPressure,
    transmissionFluidPressure,
    fuelPressure,
    boostPressure,
    fuelLevel,
    distanceDriven,
    speed,
    verticalAcceleration,
    longitudinalAcceleration,
    lateralAcceleration,
    calculatedEngineLoad,
    throttlePosition,
    engineFuelRate,
    count
};

/* ----- VALUE REGISTRY ----- */

/// Fixed-capacity registry for built-in and application-created measurements.
class ValueRegistry {
public:
    /* ----- CONSTANTS ----- */

    static constexpr std::size_t MaxUserValues = 16;
    static constexpr std::size_t MaxUserIdLength = 31;
    static constexpr std::size_t MaxUserNameLength = 31;
    static constexpr std::size_t BuiltInCount =
        static_cast<std::size_t>(BuiltInValue::count);

    /* ----- HANDLE CREATION ----- */

    [[nodiscard]]
    static constexpr ValueHandle handle(BuiltInValue value) noexcept {
        return ValueHandle::builtIn(
            static_cast<ValueHandle::Storage>(value)
        );
    }

    /* ----- LOOKUP ----- */

    [[nodiscard]]
    static ValueHandle resolve(std::string_view id) noexcept;

    [[nodiscard]]
    static bool exists(ValueHandle handle) noexcept;

    /* ----- METADATA ----- */

    [[nodiscard]]
    static std::string_view id(ValueHandle handle) noexcept;

    [[nodiscard]]
    static std::string_view name(ValueHandle handle) noexcept;

    [[nodiscard]]
    static const UnitType* unit(ValueHandle handle) noexcept;

    [[nodiscard]]
    static Measurement minimum(ValueHandle handle) noexcept;

    [[nodiscard]]
    static Measurement maximum(ValueHandle handle) noexcept;

    [[nodiscard]]
    static const ValueDefinition* definition(
        BuiltInValue value
    ) noexcept;

    /* ----- VALUE ACCESS ----- */

    [[nodiscard]]
    static Measurement value(ValueHandle handle) noexcept;

    [[nodiscard]]
    static Measurement value(BuiltInValue value) noexcept;

    [[nodiscard]]
    static bool available(ValueHandle handle) noexcept;

    /* ----- VALUE MUTATION ----- */

    [[nodiscard]]
    static bool set(
        ValueHandle handle,
        Measurement value
    ) noexcept;

    [[nodiscard]]
    static bool set(
        BuiltInValue builtIn,
        Measurement measurement
    ) noexcept;

    [[nodiscard]]
    static bool invalidate(ValueHandle handle) noexcept;

    [[nodiscard]]
    static bool invalidate(BuiltInValue value) noexcept;

    /* ----- USER VALUE MANAGEMENT ----- */

    /// Adds a user value. The UnitType must outlive the registry entry.
    [[nodiscard]]
    static ValueHandle add(
        std::string_view id,
        std::string_view name,
        const UnitType& unit,
        Measurement minimum,
        Measurement maximum
    ) noexcept;

    [[nodiscard]]
    static bool remove(ValueHandle handle) noexcept;

    [[nodiscard]]
    static bool remove(std::string_view id) noexcept;

    static void clearUsers() noexcept;

    /* ----- CAPACITY QUERIES ----- */

    [[nodiscard]]
    static constexpr std::size_t builtInCount() noexcept {
        return BuiltInCount;
    }

    [[nodiscard]]
    static std::size_t userCount() noexcept;

    [[nodiscard]]
    static std::size_t size() noexcept;

    [[nodiscard]]
    static bool full() noexcept;

    /* ----- ITERATION ----- */

    template<typename Callback>
    static void forEachBuiltIn(Callback&& callback) {
        for (std::size_t index = 0; index < BuiltInCount; ++index) {
            callback(
                ValueHandle::builtIn(
                    static_cast<ValueHandle::Storage>(index)
                )
            );
        }
    }

    template<typename Callback>
    static void forEachUser(Callback&& callback) {
        for (std::size_t slot = 0; slot < MaxUserValues; ++slot) {
            const ValueHandle current = userHandleAt(slot);

            if (current.valid()) {
                callback(current);
            }
        }
    }

    template<typename Callback>
    static void forEach(Callback&& callback) {
        forEachBuiltIn(callback);
        forEachUser(callback);
    }

private:
    /* ----- INTERNAL HELPERS ----- */

    [[nodiscard]]
    static ValueHandle userHandleAt(std::size_t slot) noexcept;
};

} // namespace mg