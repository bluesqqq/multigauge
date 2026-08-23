#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <multigauge/value/Unit.h>

namespace mg::sensor {

/// @brief Definition of a user-created value.
struct UserValueConfig {
    std::string id;
    std::string name;
    std::string unitType;
    Measurement minimum = 0.0F;
    Measurement maximum = 0.0F;
};

/// @brief Maps one provider sensor to a value.
struct SensorBinding {
    std::string id;
    std::string providerId;
    std::string sensorId;
    std::string valueId;
    bool enabled = true;
    std::uint8_t priority = 0;
    std::chrono::milliseconds staleAfter{1000};
};

} // namespace mg::sensor
