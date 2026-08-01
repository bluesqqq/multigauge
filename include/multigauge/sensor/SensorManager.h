#pragma once

#include <chrono>
#include <array>
#include <cstddef>
#include <string_view>

#include <multigauge/sensor/SensorProvider.h>

namespace mg {

/// Fixed-capacity coordinator and index for registered sensors.
///
/// SensorManager borrows providers and their sensors. Providers own the sensor
/// objects, and those objects must remain alive at stable addresses for as
/// long as the provider stays registered.
///
/// Registration order is preserved for enumeration and update dispatch.
class SensorManager {
public:
    static constexpr std::size_t MaxProviders = 8;
    static constexpr std::size_t MaxSensors = 64;

    /// Registers a provider and indexes all of its sensors.
    /// Returns false when validation fails or capacity is exceeded.
    [[nodiscard]] bool registerProvider(SensorProvider& provider) noexcept;

    /// Unregisters a provider by ID and removes all of its indexed sensors.
    /// Enumeration order for the remaining entries is preserved.
    [[nodiscard]] bool unregisterProvider(std::string_view providerId) noexcept;

    /// Clears every provider and sensor registration.
    void clear() noexcept;

    /// Updates every registered provider in registration order.
    void update(std::chrono::microseconds elapsed) noexcept;

    [[nodiscard]] std::size_t providerCount() const noexcept;
    [[nodiscard]] std::size_t sensorCount() const noexcept;

    /// Returns the provider at `index`, or nullptr when out of range.
    [[nodiscard]] const SensorProvider* providerAt(std::size_t index) const noexcept;

    /// Returns the sensor at `index`, or nullptr when out of range.
    [[nodiscard]] const Sensor* sensorAt(std::size_t index) const noexcept;

    /// Finds a provider by stable ID.
    [[nodiscard]] const SensorProvider* findProvider(std::string_view providerId) const noexcept;

    /// Finds a sensor by globally unique sensor ID.
    [[nodiscard]] const Sensor* findSensor(std::string_view sensorId) const noexcept;

private:
    struct ProviderEntry {
        SensorProvider* provider = nullptr;
        std::string_view id{};
    };

    struct SensorEntry {
        SensorProvider* provider = nullptr;
        const Sensor* sensor = nullptr;
        std::string_view id{};
    };

    [[nodiscard]] static bool hasDuplicatePointer(
        const SensorProvider* provider,
        const ProviderEntry* entries,
        std::size_t count
    ) noexcept;

    [[nodiscard]] static bool hasDuplicateSensorPointer(
        const Sensor* sensor,
        const SensorEntry* entries,
        std::size_t count
    ) noexcept;

    [[nodiscard]] static bool hasDuplicateSensorPointer(
        const Sensor* sensor,
        const Sensor* const* entries,
        std::size_t count
    ) noexcept;

    [[nodiscard]] static bool hasDuplicateId(
        std::string_view id,
        const ProviderEntry* entries,
        std::size_t count
    ) noexcept;

    [[nodiscard]] static bool hasDuplicateId(
        std::string_view id,
        const SensorEntry* entries,
        std::size_t count
    ) noexcept;

    std::array<ProviderEntry, MaxProviders> providers_{};
    std::array<SensorEntry, MaxSensors> sensors_{};
    std::size_t providerCount_ = 0;
    std::size_t sensorCount_ = 0;
};

} // namespace mg
