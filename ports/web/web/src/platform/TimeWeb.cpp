#include "TimeWeb.h"
#include <emscripten.h>

uint64_t TimeWeb::getMillis() {
    return (uint64_t)emscripten_get_now();
}

uint64_t TimeWeb::getMicros() {
    double ms = emscripten_get_now();
    return (uint64_t)(ms * 1000.0);
}