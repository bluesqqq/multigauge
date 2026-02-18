#pragma once

#include <cstdarg>
#include <cstdint>

enum class LogLevel : uint8_t {
    Debug,
    Info,
    Warn,
    Error
};

class Logger {
    private:
        LogLevel minLevel = LogLevel::Debug;

    protected:
        virtual void _log(LogLevel level, const char* tag, const char* fmt, va_list args) = 0;

    public:
        virtual ~Logger() = default;

        virtual bool init();

        void log(LogLevel level, const char* tag, const char* fmt, ...);

        void setMinLevel(LogLevel l);
};

Logger* getLogger();
void setLogger(Logger& logger);

#define LOG_DEBUG(tag, fmt, ...)  do { if (auto* _l = getLogger()) _l->log(LogLevel::Debug, tag, fmt, ##__VA_ARGS__); } while (0)
#define LOG_INFO(tag,  fmt, ...)  do { if (auto* _l = getLogger()) _l->log(LogLevel::Info,  tag, fmt, ##__VA_ARGS__); } while (0)
#define LOG_WARN(tag,  fmt, ...)  do { if (auto* _l = getLogger()) _l->log(LogLevel::Warn,  tag, fmt, ##__VA_ARGS__); } while (0)
#define LOG_ERROR(tag, fmt, ...)  do { if (auto* _l = getLogger()) _l->log(LogLevel::Error, tag, fmt, ##__VA_ARGS__); } while (0)