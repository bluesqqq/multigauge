#include <multigauge/sensor/Manager.h>
#include "../AppPaths.h"
#include <multigauge/io/FileSystem.h>
#include <multigauge/utils/Json.h>
#include <multigauge/value/ValueRegistry.h>

#include <algorithm>
#include <cctype>
#include <limits>

namespace mg::sensor {

namespace {

bool id(std::string_view s) {
    return !s.empty() && s.size() <= 63 && std::isalnum((unsigned char) s.front()) && std::all_of(s.begin(), s.end(), [](char c) {
        return std::isalnum((unsigned char) c) || c == '.' || c == '_' || c == '-';
    });
}
bool copy(json::Reader in , json::Document & out) {
    if (!in.isObject()) return false;
    out = json::object();
    return out.writer().write(in);
}
bool str(json::Reader o, std::string_view k, std::string & v) {
    return json::getStringMember(o, k, v);
}

}

Manager::Manager(io::FileSystem & fs, std::string root): fs_(fs), dataRoot_(std::move(root)) {}

bool Manager::load() {
    ValueRegistry::clearUsers();
    const auto p = paths::statePath(dataRoot_);
    if (!fs_.exists(p)) return true;
    json::Document d;
    return json::readJsonFile(fs_, p, d) && loadDocument(d.root());
}

bool Manager::save() {
    if (!dirty_) return true;
    json::Document d = json::object();
    auto w = d.writer();
    if (!writeDocument(w)) return false;
    const auto p = paths::statePath(dataRoot_),
        t = p + ".tmp";
    if (!json::writeJsonFile(fs_, t, d.root()) || !fs_.rename(t, p)) {
        (void) fs_.remove(t);
        return false;
    }
    dirty_ = false;
    return true;
}

bool Manager::loadDocument(json::Reader r) {
    std::uint64_t v = 0;
    auto values = json::getArrayMember(r, "userValues"), providers = json::getArrayMember(r, "providers"), bindings = json::getArrayMember(r, "bindings");
    if (!r.isObject() || !r.member("version").read(v) || v != 1 || !values.valid() || !providers.valid() || !bindings.valid() || values.size() > ValueRegistry::MaxUserValues || providers.size() > Registry::MaxProviders || bindings.size() > Registry::MaxSensors) return false;
    for (size_t i = 0; i < values.size(); ++i) {
        UserValueConfig x;
        auto e = values.element(i);
        double lo, hi;
        if (!e.isObject() || !str(e, "id", x.id) || !str(e, "name", x.name) || !str(e, "unitType", x.unitType) || !e.member("minimum").read(lo) || !e.member("maximum").read(hi)) return false;
        x.minimum = (float) lo;
        x.maximum = (float) hi;
        auto * u = UnitType::find(x.unitType);
        if (!u || x.minimum > x.maximum || !ValueRegistry::add(x.id, x.name, * u, x.minimum, x.maximum).valid()) return false;
    }
    for (size_t i = 0; i < providers.size(); ++i) {
        ProviderConfig x;
        auto e = providers.element(i);
        if (!e.isObject() || !str(e, "id", x.id) || !str(e, "type", x.type) || !e.member("enabled").read(x.enabled) || !id(x.id) || !id(x.type) || !copy(e.member("config"), x.config)) return false;
        providers_.push_back(std::move(x));
    }
    for (size_t i = 0; i < bindings.size(); ++i) {
        BindingRuntime x;
        auto e = bindings.element(i);
        std::uint64_t p = 0, s = 0;
        if (!e.isObject() || !str(e, "id", x.binding.id) || !str(e, "providerId", x.binding.providerId) || !str(e, "sensorId", x.binding.sensorId) || !str(e, "valueId", x.binding.valueId) || !e.member("enabled").read(x.binding.enabled) || !e.member("priority").read(p) || !e.member("staleAfterMs").read(s) || p > 255 || s > 3600000) return false;
        x.binding.priority = (uint8_t) p;
        x.binding.staleAfter = std::chrono::milliseconds(s);
        if (!validateBinding(x.binding) || !providerConfig(x.binding.providerId)) return false;
        bindings_.push_back(std::move(x));
    }
    dirty_ = false;
    return true;
}

bool Manager::writeDocument(json::Writer & w) const {
    return w.writeObject([ & ](json::ObjectWriter & o) {
        return o.write("version", 1) && o.writeArray("userValues", [ & ](json::ArrayWriter & a) {
            bool ok = true;
            ValueRegistry::forEachUser([ & ](ValueHandle h) {
                auto * u = ValueRegistry::unit(h);
                ok = ok && u && a.writeObject([ & ](json::ObjectWriter & x) {
                    return x.write("id", ValueRegistry::id(h)) && x.write("name", ValueRegistry::name(h)) && x.write("unitType", u -> name()) && x.write("minimum", ValueRegistry::minimum(h)) && x.write("maximum", ValueRegistry::maximum(h));
                });
            });
            return ok;
        }) && o.writeArray("providers", [ & ](json::ArrayWriter & a) {
            for (auto & c: providers_)
                if (!a.writeObject([ & ](json::ObjectWriter & x) {
                        auto * p = registry_.findProvider(c.id);
                        return x.write("id", c.id) && x.write("type", c.type) && x.write("enabled", c.enabled) && x.writeValue("config", [ & ](json::Writer & n) {
                            return p ? p -> saveConfiguration(n) : n.write(c.config.root());
                        });
                    })) return false;
            return true;
        }) && o.writeArray("bindings", [ & ](json::ArrayWriter & a) {
            for (auto & r: bindings_) {
                auto & b = r.binding;
                if (!a.writeObject([ & ](json::ObjectWriter & x) {
                        return x.write("id", b.id) && x.write("providerId", b.providerId) && x.write("sensorId", b.sensorId) && x.write("valueId", b.valueId) && x.write("enabled", b.enabled) && x.write("priority", (uint64_t) b.priority) && x.write("staleAfterMs", (uint64_t) b.staleAfter.count());
                    })) return false;
            }
            return true;
        });
    });
}

bool Manager::registerProvider(Provider & p) {
    auto * c = providerConfig(p.id());
    if (!c) {
        if (providers_.size() >= Registry::MaxProviders || !id(p.id()) || !id(p.type())) return false;
        providers_.push_back({
            std::string(p.id()),
            std::string(p.type())
        });
        c = & providers_.back();
        dirty_ = true;
    }
    return c -> type == p.type() && p.loadConfiguration(c -> config.root()) && registry_.registerProvider(p);
}

bool Manager::unregisterProvider(std::string_view i) {
    return registry_.unregisterProvider(i);
}

const Provider * Manager::findProvider(std::string_view i) const noexcept {
    return registry_.findProvider(i);
}

const Sensor * Manager::findSensor(std::string_view p, std::string_view s) const noexcept {
    auto * x = registry_.findProvider(p);
    if (!x) return nullptr;
    for (size_t i = 0; i < x -> sensorCount(); ++i) {
        auto * q = x -> sensorAt(i);
        if (q && q -> id() == s) return q;
    }
    return nullptr;
}

Result Manager::configureProvider(std::string_view i, json::Reader c) {
    auto * s = providerConfig(i);
    auto * p = registry_.findProvider(i);
    json::Document d;
    if (!s || !p || !copy(c, d)) return Error("Provider is not registered or configuration is invalid");
    if (!registry_.unregisterProvider(i) || !p -> loadConfiguration(d.root()) || !registry_.registerProvider( * p)) return Error("Provider rejected configuration");
    s -> config = std::move(d);
    dirty_ = true;
    return OkObject();
}

Result Manager::providerConfiguration(std::string_view i) const {
    auto * c = providerConfig(i);
    if (!c) return Error("Provider configuration not found");
    Result r;
    r.ok = true;
    auto w = r.data.writer();
    if (!w.write(c -> config.root())) return Error("Failed to copy provider configuration");
    return r;
}

Result Manager::setProviderEnabled(std::string_view i, bool e) {
    auto * c = providerConfig(i);
    if (!c) return Error("Provider configuration not found");
    c -> enabled = e;
    dirty_ = true;
    return OkObject();
}

Result Manager::defineUserValue(const UserValueConfig & v) {
    auto * u = UnitType::find(v.unitType);
    if (!u || v.minimum > v.maximum || !ValueRegistry::add(v.id, v.name, * u, v.minimum, v.maximum).valid()) return Error("Invalid user value");
    dirty_ = true;
    return OkObject();
}

bool Manager::removeUserValue(std::string_view i) {
    bool r = ValueRegistry::remove(i);
    dirty_ |= r;
    return r;
}

bool Manager::listUserValues(std::vector < UserValueConfig > & o) const {
    o.clear();
    ValueRegistry::forEachUser([ & ](ValueHandle h) {
        auto * u = ValueRegistry::unit(h);
        if (u) o.push_back({
            std::string(ValueRegistry::id(h)),
            std::string(ValueRegistry::name(h)),
            std::string(u -> name()),
            ValueRegistry::minimum(h),
            ValueRegistry::maximum(h)
        });
    });
    return true;
}

bool Manager::validateBinding(const SensorBinding & b) const {
    return id(b.id) && id(b.providerId) && !b.sensorId.empty() && !b.valueId.empty() && ValueRegistry::resolve(b.valueId).valid() && b.staleAfter.count() >= 0;
}

Result Manager::upsertBinding(const SensorBinding & b) {
    if (!validateBinding(b) || !providerConfig(b.providerId)) return Error("Invalid binding");
    auto i = std::find_if(bindings_.begin(), bindings_.end(), [ & ](auto & x) {
        return x.binding.id == b.id;
    });
    if (i == bindings_.end()) bindings_.push_back({
        b
    });
    else {
        i -> binding = b;
        i -> hasSample = false;
    }
    dirty_ = true;
    return OkObject();
}

bool Manager::removeBinding(std::string_view i) {
    auto x = std::find_if(bindings_.begin(), bindings_.end(), [ & ](auto & b) {
        return b.binding.id == i;
    });
    if (x == bindings_.end()) return false;
    bindings_.erase(x);
    dirty_ = true;
    return true;
}

bool Manager::listBindings(std::vector < SensorBinding > & o) const {
    o.clear();
    for (auto & x: bindings_) o.push_back(x.binding);
    return true;
}

void Manager::update(std::chrono::microseconds e, std::chrono::microseconds n) {
    for (size_t i = 0; i < registry_.providerCount(); ++i) {
        auto * p = registry_.providerAt(i);
        auto * c = p ? providerConfig(p -> id()) : nullptr;
        if (p && c && c -> enabled) p -> update(e);
    }
    evaluateBindings(n);
}

void Manager::evaluateBindings(std::chrono::microseconds n) {
    for (auto & r: bindings_) {
        auto * s = findSensor(r.binding.providerId, r.binding.sensorId);
        if (s && s -> reading().available() && (!r.hasSample || s -> reading().sequence != r.lastSequence)) {
            r.lastSequence = s -> reading().sequence;
            r.lastSample = n;
            r.hasSample = true;
        }
    }
    for (size_t i = 0; i < bindings_.size(); ++i) {
        auto & t = bindings_[i].binding.valueId;
        if (!bindings_[i].binding.enabled) continue;
        bool first = true;
        for (size_t j = 0; j < i; ++j)
            if (bindings_[j].binding.enabled && bindings_[j].binding.valueId == t) first = false;
        if (!first) continue;
        (void) ValueRegistry::invalidate(ValueRegistry::resolve(t));
        const BindingRuntime * best = nullptr;
        for (auto & r: bindings_) {
            auto & b = r.binding;
            auto * s = findSensor(b.providerId, b.sensorId);
            if (!b.enabled || b.valueId != t || !s || !r.hasSample || n - r.lastSample > b.staleAfter || s -> unit() != ValueRegistry::unit(ValueRegistry::resolve(t))) continue;
            if (!best || b.priority > best -> binding.priority || (b.priority == best -> binding.priority && b.id < best -> binding.id)) best = & r;
        }
        if (best) {
            auto * s = findSensor(best -> binding.providerId, best -> binding.sensorId);
            (void) ValueRegistry::set(ValueRegistry::resolve(t), s -> unit() -> convertToBase(s -> reading().value, s -> unitIndex()));
        }
    }
}

ProviderConfig * Manager::providerConfig(std::string_view i) noexcept {
    auto x = std::find_if(providers_.begin(), providers_.end(), [ & ](auto & c) {
        return c.id == i;
    });
    return x == providers_.end() ? nullptr : & * x;
}

const ProviderConfig * Manager::providerConfig(std::string_view i) const noexcept {
    auto x = std::find_if(providers_.begin(), providers_.end(), [ & ](auto & c) {
        return c.id == i;
    });
    return x == providers_.end() ? nullptr : & * x;
}

} // namespace mg::sensor