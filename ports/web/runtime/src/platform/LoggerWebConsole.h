#pragma once

#include <multigauge/io/Logger.h>

class LoggerWebConsole : public mg::io::Logger {
    protected:
        void _log(mg::io::LogLevel level, const char* tag, const char* fmt, va_list args) override;
    public:
        bool init() override;
};
