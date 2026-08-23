#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

#include <multigauge/editor/Result.h>
#include <multigauge/sensor/Types.h>
#include <multigauge/sensor/Registry.h>

namespace mg::io { class FileSystem; }

namespace mg::sensor {

struct ProviderConfig {
    std::string id;
    std::string type;
    bool enabled = true;
    json::Document config = json::object();
};

class Manager {
public:
    explicit Manager(io::FileSystem& fs, std::string dataRoot);

    //----------[ PERSISTENCE ]----------//

    [[nodiscard]] bool load();
    [[nodiscard]] bool save();

    //----------[ PROVIDERS ]----------//

    [[nodiscard]] bool registerProvider(Provider& provider);
    [[nodiscard]] bool unregisterProvider(std::string_view providerId);
    [[nodiscard]] const Provider* findProvider(std::string_view providerId) const noexcept;
    [[nodiscard]] const Sensor* findSensor(std::string_view providerId, std::string_view sensorId) const noexcept;
    [[nodiscard]] Result configureProvider(std::string_view providerId, json::Reader config);
    [[nodiscard]] Result providerConfiguration(std::string_view providerId) const;
    [[nodiscard]] Result setProviderEnabled(std::string_view providerId, bool enabled);

    //----------[ USER VALUES ]----------//

    [[nodiscard]] Result defineUserValue(const UserValueConfig& value);
    [[nodiscard]] bool removeUserValue(std::string_view valueId);
    [[nodiscard]] bool listUserValues(std::vector<UserValueConfig>& out) const;

    //----------[ BINDINGS ]----------//

    [[nodiscard]] Result upsertBinding(const SensorBinding& binding);
    [[nodiscard]] bool removeBinding(std::string_view bindingId);
    [[nodiscard]] bool listBindings(std::vector<SensorBinding>& out) const;

    //----------[ UPDATE ]----------//

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

    [[nodiscard]] bool loadDocument(json::Reader root);
    [[nodiscard]] bool writeDocument(json::Writer& writer) const;
    [[nodiscard]] bool validateBinding(const SensorBinding& binding) const;
    void evaluateBindings(std::chrono::microseconds now);
    [[nodiscard]] ProviderConfig* providerConfig(std::string_view id) noexcept;
    [[nodiscard]] const ProviderConfig* providerConfig(std::string_view id) const noexcept;

    //----------[ STORED STATE ]----------//
    
    io::FileSystem& fs_;
    std::string dataRoot_;
    Registry registry_;
    std::vector<ProviderConfig> providers_;
    std::vector<BindingRuntime> bindings_;
    bool dirty_ = false;
};

} // namespace mg::sensor
