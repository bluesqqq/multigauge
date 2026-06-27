#include "sensorI2C.h"

#include <Arduino.h>
#include <Wire.h>

bool SensorI2C::begin() {
    Wire.begin();
    Wire.beginTransmission(address);
    return Wire.endTransmission() == 0;
}
