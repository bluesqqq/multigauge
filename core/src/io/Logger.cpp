#include <multigauge/io/Logger.h>

namespace mg::io {

bool Logger::init() { return true; }

void Logger::log(LogLevel level, const char *tag, const char *fmt, ...) {
    if ((uint8_t) level < (uint8_t)minLevel) return;
    
    va_list args;
    va_start(args, fmt);
    _log(level, tag, fmt, args);
    va_end(args);
} // namespace mg::io

void Logger::setMinLevel(LogLevel l) { minLevel = l; }

}
