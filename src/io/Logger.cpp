#include "Logger.h"

bool Logger::init() { return true; }

void Logger::log(LogLevel level, const char *tag, const char *fmt, ...) {
    if ((uint8_t) level < (uint8_t)minLevel) return;
    
    va_list args;
    va_start(args, fmt);
    _log(level, tag, fmt, args);
    va_end(args);
}

void Logger::setMinLevel(LogLevel l) { minLevel = l; }


static Logger* g_logger = nullptr;
Logger* getLogger() { return g_logger; }
void setLogger(Logger& logger) { g_logger = &logger; }