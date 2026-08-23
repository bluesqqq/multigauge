#include <multigauge/sensor/SensorManager.h>

namespace mg {
namespace {

template<typename Entry>
bool hasDuplicateIdImpl(
    std::string_view id,
    const Entry* entries,
    std::size_t count
) noexcept {
    for (std::size_t i = 0; i < count; ++i) {
        if (entries[i].id == id) {
            return true;
        }
    }

    return false;
}

} // namespace

bool SensorManager::hasDuplicatePointer(
    const SensorProvider* provider,
    const ProviderEntry* entries,
    std::size_t count
) noexcept {
    for (std::size_t i = 0; i < count; ++i) {
        if (entries[i].provider == provider) {
            return true;
        }
    }

    return false;
}

bool SensorManager::hasDuplicateSensorPointer(
    const Sensor* sensor,
    const SensorEntry* entries,
    std::size_t count
) noexcept {
    for (std::size_t i = 0; i < count; ++i) {
        if (entries[i].sensor == sensor) {
            return true;
        }
    }

    return false;
}

bool SensorManager::hasDuplicateSensorPointer(
    const Sensor* sensor,
    const Sensor* const* entries,
    std::size_t count
) noexcept {
    for (std::size_t i = 0; i < count; ++i) {
        if (entries[i] == sensor) {
            return true;
        }
    }

    return false;
}

bool SensorManager::hasDuplicateId(
    std::string_view id,
    const ProviderEntry* entries,
    std::size_t count
) noexcept {
    return hasDuplicateIdImpl(id, entries, count);
}

bool SensorManager::hasDuplicateId(
    std::string_view id,
    const SensorEntry* entries,
    std::size_t count
) noexcept {
    return hasDuplicateIdImpl(id, entries, count);
}

bool SensorManager::registerProvider(SensorProvider& provider) noexcept {
    SensorProvider* providerPtr = &provider;
    const std::string_view providerId = provider.id();

    if (providerId.empty()) {
        return false;
    }

    if (providerCount_ >= MaxProviders) {
        return false;
    }

    if (hasDuplicatePointer(providerPtr, providers_.data(), providerCount_)) {
        return false;
    }

    if (hasDuplicateId(providerId, providers_.data(), providerCount_)) {
        return false;
    }

    const std::size_t sensorTotal = provider.sensorCount();
    if (sensorTotal > MaxSensors - sensorCount_) {
        return false;
    }

    std::array<const Sensor*, MaxSensors> stagedSensors{};
    std::array<std::string_view, MaxSensors> stagedIds{};

    for (std::size_t index = 0; index < sensorTotal; ++index) {
        const Sensor* sensor = provider.sensorAt(index);
        if (!sensor) {
            return false;
        }

        const std::string_view sensorId = sensor->id();
        if (sensorId.empty()) {
            return false;
        }

        if (hasDuplicateSensorPointer(sensor, stagedSensors.data(), index)) {
            return false;
        }

        for (std::size_t stagedIndex = 0; stagedIndex < index; ++stagedIndex) {
            if (stagedIds[stagedIndex] == sensorId) {
                return false;
            }
        }

        if (hasDuplicateId(sensorId, sensors_.data(), sensorCount_)) {
            return false;
        }

        if (hasDuplicateSensorPointer(sensor, sensors_.data(), sensorCount_)) {
            return false;
        }

        stagedSensors[index] = sensor;
        stagedIds[index] = sensorId;
    }

    providers_[providerCount_] = { providerPtr, providerId };
    ++providerCount_;

    for (std::size_t index = 0; index < sensorTotal; ++index) {
        sensors_[sensorCount_ + index] = {
            providerPtr,
            stagedSensors[index],
            stagedIds[index]
        };
    }

    sensorCount_ += sensorTotal;
    return true;
}

bool SensorManager::unregisterProvider(std::string_view providerId) noexcept {
    if (providerId.empty()) {
        return false;
    }

    std::size_t providerIndex = providerCount_;
    for (std::size_t index = 0; index < providerCount_; ++index) {
        if (providers_[index].id == providerId) {
            providerIndex = index;
            break;
        }
    }

    if (providerIndex == providerCount_) {
        return false;
    }

    SensorProvider* provider = providers_[providerIndex].provider;

    for (std::size_t index = providerIndex + 1; index < providerCount_; ++index) {
        providers_[index - 1] = providers_[index];
    }
    --providerCount_;

    std::size_t nextSensorIndex = 0;
    for (std::size_t index = 0; index < sensorCount_; ++index) {
        if (sensors_[index].provider == provider) {
            continue;
        }

        sensors_[nextSensorIndex++] = sensors_[index];
    }

    sensorCount_ = nextSensorIndex;
    return true;
}

void SensorManager::clear() noexcept {
    providerCount_ = 0;
    sensorCount_ = 0;
}

void SensorManager::update(std::chrono::microseconds elapsed) noexcept {
    for (std::size_t index = 0; index < providerCount_; ++index) {
        providers_[index].provider->update(elapsed);
    }
}

std::size_t SensorManager::providerCount() const noexcept {
    return providerCount_;
}

std::size_t SensorManager::sensorCount() const noexcept {
    return sensorCount_;
}

SensorProvider* SensorManager::providerAt(std::size_t index) noexcept {
    return index < providerCount_ ? providers_[index].provider : nullptr;
}

const SensorProvider* SensorManager::providerAt(std::size_t index) const noexcept {
    return index < providerCount_ ? providers_[index].provider : nullptr;
}

const Sensor* SensorManager::sensorAt(std::size_t index) const noexcept {
    return index < sensorCount_ ? sensors_[index].sensor : nullptr;
}

SensorProvider* SensorManager::findProvider(std::string_view providerId) noexcept {
    if (providerId.empty()) return nullptr;
    for (std::size_t index = 0; index < providerCount_; ++index) {
        if (providers_[index].id == providerId) return providers_[index].provider;
    }
    return nullptr;
}

const SensorProvider* SensorManager::findProvider(std::string_view providerId) const noexcept {
    if (providerId.empty()) return nullptr;
    for (std::size_t index = 0; index < providerCount_; ++index) {
        if (providers_[index].id == providerId) return providers_[index].provider;
    }
    return nullptr;
}

const Sensor* SensorManager::findSensor(std::string_view sensorId) const noexcept {
    if (sensorId.empty()) {
        return nullptr;
    }

    for (std::size_t index = 0; index < sensorCount_; ++index) {
        if (sensors_[index].id == sensorId) {
            return sensors_[index].sensor;
        }
    }

    return nullptr;
}

} // namespace mg
