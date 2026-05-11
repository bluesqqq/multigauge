#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
namespace mg::utils {

size_t fastFloatToString(float value, uint8_t decimalPlaces, char* buf, size_t bufSize);

std::string floatToString(float value, uint8_t decimalPlaces);

template <typename T>
inline T lerp(T a, T b, float t) { return a + (b - a) * t; }

inline float mapf(float x, float inMin, float inMax, float outMin, float outMax) { return (inMin == inMax) ? outMin : (x - inMin) * (outMax - outMin) / (inMax - inMin) + outMin; }

float floorDivisible(float n, float factor, float offset = 0);

float ceilDivisible(float n, float factor, float offset = 0);

inline bool inRange(float v, float lo, float hi) { return (v >= lo) && (v <= hi); }

} // namespace mg::utils
