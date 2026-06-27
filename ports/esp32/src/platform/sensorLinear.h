#pragma once

#include "sensor.h"

#include <multigauge/value/Value.h>

class SensorLinear : public Sensor {
    private:
        mg::Value* value;

        float lowerVoltage;
        float upperVoltage;

        float lowerValue;
        float upperValue;

        uint8_t unitIndex;

        uint32_t pin;
    
    public:
        SensorLinear(uint32_t pin, mg::Value* value, float lowerVoltage, float upperVoltage, float lowerValue, float upperValue, int unitIndex = 0, std::string name = "Sensor");

        bool begin() override;

        void readSensor() override;
};
