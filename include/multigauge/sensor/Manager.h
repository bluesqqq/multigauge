#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <multigauge/editor/Result.h>
#include <multigauge/sensor/Types.h>
#include <multigauge/sensor/Registry.h>

namespace mg::io { class FileSystem; }

namespace mg::sensor {

/// @brief Persisted settings for one provider.
struct ProviderConfig {
    std::string id;
    std::string type;
    bool enabled = true;
    json::Document config = json::object();
};

/// @brief Coordinates sensor providers, values, bindings, and persistence.
class Manager {
public:
    /// @brief Creates the sensor system. The supplied filesystem and its configured
    explicit Manager(io::FileSystem& fs, std::string dataRoot);

    //----------[ PERSISTENCE ]----------//

    /// @brief Replaces persisted user values, provider configuration, and bindings.
    /// @return True when state was loaded or no state file exists.
    [[nodiscard]] bool load();

    /// @brief Writes changed sensor configuration to persistent state.
    /// @return True when state is already current or was saved successfully.
    [[nodiscard]] bool save();

    //----------[ PROVIDERS ]----------//

    /// @brief Takes ownership of a provider and applies its saved configuration.
    /// @param provider The provider to register. Ownership is retained only on success.
    /// @return True when the provider is configured and registered.
    [[nodiscard]] bool registerProvider(std::unique_ptr<Provider> provider);

    /// @brief Destroys a registered provider while retaining its persisted configuration.
    /// @param providerId The stable provider identifier.
    /// @return True when a registered provider was removed.
    [[nodiscard]] bool unregisterProvider(std::string_view providerId);

    /// @brief Finds a registered provider by identifier.
    /// @param providerId The stable provider identifier.
    /// @return The borrowed provider, or nullptr when it is not registered.
    [[nodiscard]] const Provider* findProvider(std::string_view providerId) const noexcept;

    /// @brief Finds a sensor supplied by a registered provider.
    /// @param providerId The stable provider identifier.
    /// @param sensorId The stable sensor identifier.
    /// @return The borrowed sensor, or nullptr when it is not available.
    [[nodiscard]] const Sensor* findSensor(std::string_view providerId, std::string_view sensorId) const noexcept;

    /// @brief Applies and persists a provider configuration.
    /// @param providerId The registered provider identifier.
    /// @param config The provider-specific configuration object.
    /// @return A successful result when configuration and reindexing succeed.
    [[nodiscard]] Result configureProvider(std::string_view providerId, json::Reader config);

    /// @brief Returns the current provider-specific configuration.
    /// @param providerId The provider identifier.
    /// @return A result containing the configuration object on success.
    [[nodiscard]] Result providerConfiguration(std::string_view providerId) const;

    /// @brief Enables or disables provider update dispatch.
    /// @param providerId The provider identifier.
    /// @param enabled True to update the provider; false to suspend it.
    /// @return A successful result when the provider configuration exists.
    [[nodiscard]] Result setProviderEnabled(std::string_view providerId, bool enabled);

    //----------[ USER VALUES ]----------//

    /// @brief Defines a user value in the global ValueRegistry.
    /// @param value Value definition to add.
    /// @return A successful result when the definition is valid and added.
    [[nodiscard]] Result defineUserValue(const UserValueConfig& value);

    /// @brief Removes an unbound user value. Returns false when a binding still
    /// references it, preventing an invalid persisted configuration.
    /// @param valueId The user-value identifier.
    /// @return True when the unbound user value was removed.
    [[nodiscard]] bool removeUserValue(std::string_view valueId);

    /// @brief Enumerates all user value definitions.
    /// @param out Destination vector, cleared before values are appended.
    /// @return True after enumeration completes.
    [[nodiscard]] bool listUserValues(std::vector<UserValueConfig>& out) const;

    //----------[ BINDINGS ]----------//

    /// @brief Creates or replaces a sensor-to-value binding.
    /// @param binding The binding to validate and store.
    /// @return A successful result when the binding is valid.
    [[nodiscard]] Result upsertBinding(const SensorBinding& binding);

    /// @brief Removes a binding by identifier.
    /// @param bindingId The binding identifier.
    /// @return True when a binding was removed.
    [[nodiscard]] bool removeBinding(std::string_view bindingId);

    /// @brief Enumerates all stored bindings.
    /// @param out Destination vector, cleared before bindings are appended.
    /// @return True after enumeration completes.
    [[nodiscard]] bool listBindings(std::vector<SensorBinding>& out) const;

    //----------[ UPDATE ]----------//

    /// @brief Updates enabled providers and resolves bindings into ValueRegistry.
    /// @param elapsed Time since the previous update.
    /// @param now Monotonic time used to evaluate reading staleness.
    void update(std::chrono::microseconds elapsed, std::chrono::microseconds now);

private:
    //----------[ RUNTIME STATE ]----------//

    struct BindingRuntime {
        SensorBinding binding;
        std::chrono::microseconds lastSample{};
        std::uint32_t lastSequence = 0;
        bool hasSample = false;
    };

    //----------[ INTERNAL OPERATIONS ]----------//

    /// @brief Validates and commits a persisted state document.
    /// @param root The root JSON object to load.
    /// @return True when the document is valid and committed.
    [[nodiscard]] bool loadDocument(json::Reader root);

    /// @brief Serializes the current state into a JSON writer.
    /// @param writer Destination writer.
    /// @return True when serialization succeeds.
    [[nodiscard]] bool writeDocument(json::Writer& writer) const;

    /// @brief Checks whether a binding references valid stored state.
    /// @param binding Binding to validate.
    /// @return True when the binding is valid.
    [[nodiscard]] bool validateBinding(const SensorBinding& binding) const;

    /// @brief Selects fresh, highest-priority readings for each target value.
    /// @param now Monotonic time used to evaluate staleness.
    void evaluateBindings(std::chrono::microseconds now);

    /// @brief Finds mutable persisted configuration for a provider.
    /// @param id Provider identifier.
    /// @return The configuration, or nullptr when it is not stored.
    [[nodiscard]] ProviderConfig* providerConfig(std::string_view id) noexcept;

    /// @brief Finds persisted configuration for a provider.
    /// @param id Provider identifier.
    /// @return The configuration, or nullptr when it is not stored.
    [[nodiscard]] const ProviderConfig* providerConfig(std::string_view id) const noexcept;

    //----------[ STORED STATE ]----------//
    
    io::FileSystem& fs_;
    std::string dataRoot_;
    std::vector<std::unique_ptr<Provider>> ownedProviders_;
    Registry registry_;
    std::vector<ProviderConfig> providers_;
    std::vector<BindingRuntime> bindings_;
    bool dirty_ = false;
};

} // namespace mg::sensor
