#pragma once

#include "sensor.h"

class SensorI2C : public Sensor {
    protected:
        uint8_t address;

        virtual uint16_t readRaw();
    
    public:
        SensorI2C(std::string name = "Sensor");

        bool begin() override;
};
