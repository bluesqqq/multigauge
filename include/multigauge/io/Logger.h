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