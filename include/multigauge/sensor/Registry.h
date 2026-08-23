#pragma once

#include <chrono>
#include <array>
#include <cstddef>
#include <string_view>

#include <multigauge/sensor/Provider.h>

namespace mg::sensor {

/// @brief Fixed-capacity index of registered providers and sensors.
class Registry {
public:
    static constexpr std::size_t MaxProviders = 8;
    static constexpr std::size_t MaxSensors = 64;

    //----------[ PROVIDER REGISTRATION ]----------//
    
    /// @brief Registers a provider and indexes all of its sensors.
    /// @param provider The provider to borrow.
    /// @return True on success; false when validation fails or capacity is exceeded.
    [[nodiscard]] bool registerProvider(Provider& provider) noexcept;

    /// @brief Unregisters a provider by ID and removes all of its indexed sensors.
    /// @param providerId The stable provider identifier.
    /// @return True when a registered provider was removed.
    [[nodiscard]] bool unregisterProvider(std::string_view providerId) noexcept;

    /// @brief Reindexes a registered provider after its configuration changes.
    /// The refresh is atomic: failure leaves the existing index unchanged.
    /// @param provider The already registered provider to reindex.
    /// @return True when the new sensor set is valid and indexed.
    [[nodiscard]] bool refreshProvider(Provider& provider) noexcept;

    /// @brief Clears every provider and sensor registration.
    void clear() noexcept;

    //----------[ UPDATE ]----------//

    /// @brief Updates every registered provider in registration order.
    /// @param elapsed Time since the previous update.
    void update(std::chrono::microseconds elapsed) noexcept;

    //----------[ ENUMERATION ]----------//
    /// @brief Returns the number of registered providers.
    /// @return The registered provider count.
    [[nodiscard]] std::size_t providerCount() const noexcept;

    /// @brief Returns the number of indexed sensors.
    /// @return The indexed sensor count.
    [[nodiscard]] std::size_t sensorCount() const noexcept;

    /// @brief Returns a borrowed provider by index.
    /// @param index The zero-based provider index.
    /// @return The provider, or nullptr when index is out of range.
    [[nodiscard]] Provider* providerAt(std::size_t index) noexcept;

    /// @brief Returns a borrowed provider by index.
    /// @param index The zero-based provider index.
    /// @return The provider, or nullptr when index is out of range.
    [[nodiscard]] const Provider* providerAt(std::size_t index) const noexcept;

    /// @brief Returns a borrowed sensor by index.
    /// @param index The zero-based sensor index.
    /// @return The sensor, or nullptr when index is out of range.
    [[nodiscard]] const Sensor* sensorAt(std::size_t index) const noexcept;

    //----------[ LOOKUP ]----------//

    /// @brief Finds a provider by stable identifier.
    /// @param providerId The stable provider identifier.
    /// @return The borrowed provider, or nullptr when it is not registered.
    [[nodiscard]] Provider* findProvider(std::string_view providerId) noexcept;

    /// @brief Finds a provider by stable identifier.
    /// @param providerId The stable provider identifier.
    /// @return The borrowed provider, or nullptr when it is not registered.
    [[nodiscard]] const Provider* findProvider(std::string_view providerId) const noexcept;

    /// @brief Finds a sensor by its globally unique identifier.
    /// @param sensorId The stable sensor identifier.
    /// @return The borrowed sensor, or nullptr when it is not indexed.
    [[nodiscard]] const Sensor* findSensor(std::string_view sensorId) const noexcept;

private:
    //----------[ INDEX ENTRIES ]----------//

    struct ProviderEntry {
        Provider* provider = nullptr;
        std::string_view id{};
    };

    struct SensorEntry {
        Provider* provider = nullptr;
        const Sensor* sensor = nullptr;
        std::string_view id{};
    };

    //----------[ VALIDATION ]----------//

    /// @brief Checks provider-pointer uniqueness in an entry range.
    /// @param provider Provider pointer to find.
    /// @param entries Entries to inspect.
    /// @param count Number of entries to inspect.
    /// @return True when the pointer already exists.
    [[nodiscard]] static bool hasDuplicatePointer(
        const Provider* provider,
        const ProviderEntry* entries,
        std::size_t count
    ) noexcept;

    /// @brief Checks sensor-pointer uniqueness in registry entries.
    /// @param sensor Sensor pointer to find.
    /// @param entries Entries to inspect.
    /// @param count Number of entries to inspect.
    /// @return True when the pointer already exists.
    [[nodiscard]] static bool hasDuplicateSensorPointer(
        const Sensor* sensor,
        const SensorEntry* entries,
        std::size_t count
    ) noexcept;

    /// @brief Checks sensor-pointer uniqueness in a pointer range.
    /// @param sensor Sensor pointer to find.
    /// @param entries Sensor pointers to inspect.
    /// @param count Number of entries to inspect.
    /// @return True when the pointer already exists.
    [[nodiscard]] static bool hasDuplicateSensorPointer(
        const Sensor* sensor,
        const Sensor* const* entries,
        std::size_t count
    ) noexcept;

    /// @brief Checks identifier uniqueness in provider entries.
    /// @param id Identifier to find.
    /// @param entries Entries to inspect.
    /// @param count Number of entries to inspect.
    /// @return True when the identifier already exists.
    [[nodiscard]] static bool hasDuplicateId(
        std::string_view id,
        const ProviderEntry* entries,
        std::size_t count
    ) noexcept;

    /// @brief Checks identifier uniqueness in sensor entries.
    /// @param id Identifier to find.
    /// @param entries Entries to inspect.
    /// @param count Number of entries to inspect.
    /// @return True when the identifier already exists.
    [[nodiscard]] static bool hasDuplicateId(
        std::string_view id,
        const SensorEntry* entries,
        std::size_t count
    ) noexcept;

    //----------[ STORAGE ]----------//
    
    std::array<ProviderEntry, MaxProviders> providers_{};
    std::array<SensorEntry, MaxSensors> sensors_{};
    std::size_t providerCount_ = 0;
    std::size_t sensorCount_ = 0;
};

} // namespace mg::sensor
