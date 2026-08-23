#include <doctest/doctest.h>

#include <multigauge/io/FileSystem.h>
#include <multigauge/sensor/Manager.h>
#include <multigauge/value/ValueRegistry.h>

#include <map>
#include <set>
#include <vector>

namespace {

class MemoryFileSystem final : public mg::io::FileSystem {
public:
    bool init() override { return true; }
    bool readBytes(const std::string& path, std::vector<std::uint8_t>& out) override {
        const auto found = files_.find(path); if (found == files_.end()) return false; out = found->second; return true;
    }
    bool writeBytes(const std::string& path, const std::uint8_t* data, size_t length) override {
        files_[path] = std::vector<std::uint8_t>(data, data + length); return true;
    }
    bool exists(const std::string& path) override { return files_.contains(path) || directories_.contains(path); }
    bool size(const std::string& path, size_t& out) override { const auto found = files_.find(path); if (found == files_.end()) return false; out = found->second.size(); return true; }
    bool remove(const std::string& path) override { return files_.erase(path) != 0 || directories_.erase(path) != 0; }
    bool rename(const std::string& from, const std::string& to) override { const auto found = files_.find(from); if (found == files_.end()) return false; files_[to] = std::move(found->second); files_.erase(found); return true; }
    bool makeDirectory(const std::string& path) override { directories_.insert(path); return true; }
    bool listDirectories(const std::string&, std::vector<std::string>& out) override { out.clear(); return true; }
private:
    std::map<std::string, std::vector<std::uint8_t>> files_;
    std::set<std::string> directories_;
};

class FakeSensor final : public mg::sensor::Sensor {
public:
    std::string_view id() const noexcept override { return "rpm"; }
    std::string_view name() const noexcept override { return "Engine RPM"; }
    const mg::UnitType* unit() const noexcept override { return &mg::revolutions; }
    mg::sensor::Range nativeRange() const noexcept override { return {0.0F, 10000.0F}; }
    mg::sensor::Reading reading() const noexcept override { return reading_; }
    mg::sensor::Reading reading_{4000.0F, mg::sensor::Status::Available};
};

class FakeProvider final : public mg::sensor::Provider {
public:
    std::string_view id() const noexcept override { return "aux-main"; }
    std::string_view name() const noexcept override { return "Auxiliary"; }
    std::string_view type() const noexcept override { return "aux"; }
    void update(std::chrono::microseconds) noexcept override { ++updates; }
    std::size_t sensorCount() const noexcept override { return 1; }
    const mg::sensor::Sensor* sensorAt(std::size_t index) const noexcept override { return index == 0 ? &sensor : nullptr; }
    bool loadConfiguration(mg::json::Reader config) override {
        std::int64_t mode = 0;
        if (!config.isObject()) return false;
        (void)config.member("mode").read(mode);
        loadedMode = static_cast<int>(mode); return true;
    }
    bool saveConfiguration(mg::json::Writer& writer) const override {
        return writer.writeObject([&](mg::json::ObjectWriter& object) { return object.write("mode", loadedMode); });
    }
    FakeSensor sensor;
    int updates = 0;
    int loadedMode = 0;
};

TEST_CASE("sensor registry borrows providers and maintains its index") {
    mg::sensor::Registry registry;
    FakeProvider provider;
    FakeProvider duplicate;

    REQUIRE(registry.registerProvider(provider));
    CHECK_FALSE(registry.registerProvider(provider));
    CHECK_FALSE(registry.registerProvider(duplicate));
    CHECK(registry.providerCount() == 1);
    CHECK(registry.sensorCount() == 1);
    CHECK(registry.providerAt(0) == &provider);
    CHECK(registry.sensorAt(0) == &provider.sensor);
    CHECK(registry.findProvider("aux-main") == &provider);
    CHECK(registry.findSensor("rpm") == &provider.sensor);

    registry.update(std::chrono::milliseconds(16));
    CHECK(provider.updates == 1);

    REQUIRE(registry.unregisterProvider("aux-main"));
    CHECK(registry.providerCount() == 0);
    CHECK(registry.sensorCount() == 0);
    CHECK(registry.findProvider("aux-main") == nullptr);
    CHECK_FALSE(registry.unregisterProvider("aux-main"));
}

TEST_CASE("sensor manager persists provider configuration and resolves bindings") {
    MemoryFileSystem fs;
    mg::sensor::Manager manager(fs, "/telemetry");
    REQUIRE(manager.load());

    FakeProvider provider;
    REQUIRE(manager.registerProvider(provider));
    const auto config = mg::json::parse(R"({"mode":7})");
    REQUIRE(config.valid());
    REQUIRE(manager.configureProvider("aux-main", config.root()).ok);
    CHECK(provider.loadedMode == 7);

    REQUIRE(manager.defineUserValue({"customRPM", "Custom RPM", "revolutions", 0.0F, 10000.0F}).ok);
    REQUIRE(manager.upsertBinding({"aux-rpm", "aux-main", "rpm", "customRPM", true, 10, std::chrono::milliseconds(100)}).ok);

    manager.update(std::chrono::milliseconds(16), std::chrono::microseconds{});
    CHECK(provider.updates == 1);
    CHECK(mg::ValueRegistry::value(mg::ValueRegistry::resolve("customRPM")) == doctest::Approx(4000.0F));

    provider.sensor.reading_.status = mg::sensor::Status::Unavailable;
    manager.update(std::chrono::milliseconds(16), std::chrono::milliseconds(101));
    CHECK_FALSE(mg::ValueRegistry::available(mg::ValueRegistry::resolve("customRPM")));
    REQUIRE(manager.save());

    mg::ValueRegistry::clearUsers(); // Simulate a fresh process before state restoration.
    mg::sensor::Manager restoredManager(fs, "/telemetry");
    REQUIRE(restoredManager.load());
    std::vector<mg::sensor::UserValueConfig> values;
    REQUIRE(restoredManager.listUserValues(values));
    CHECK(values.size() == 1);
    FakeProvider restored;
    REQUIRE(restoredManager.registerProvider(restored));
    CHECK(restored.loadedMode == 7);
}

} // namespace
