#include "sensorLinear.h"

#include <Arduino.h>

SensorLinear::SensorLinear(uint32_t pin, mg::Value* value, float lowerVoltage, float upperVoltage, float lowerValue, float upperValue, int unitIndex, std::string name)
    : Sensor(name), pin(pin), value(value),
      lowerVoltage(lowerVoltage), upperVoltage(upperVoltage),
      lowerValue(lowerValue), upperValue(upperValue),
      unitIndex(unitIndex) {}

bool SensorLinear::begin() { 
    pinMode(pin, INPUT);
    return true;
}

void SensorLinear::readSensor() {
    if (value == nullptr) return;

    int rawValue = analogRead(pin); // (ESP32 range 0-4095)

    float sensorVoltage = (rawValue / 4095.0f) * 3.3f; // Convert to voltage (0-3.3V)

    float newValue = lowerValue;
    float denom = (upperVoltage - lowerVoltage);
    if (denom != 0.0f) {
        newValue = lowerValue +
            (sensorVoltage - lowerVoltage) * (upperValue - lowerValue) / denom;
    }

    value->setValue(newValue, unitIndex);
}
