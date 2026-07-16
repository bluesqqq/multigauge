#pragma once

#include <Arduino.h>
#include <multigauge/io/Logger.h>
#include <cstdarg>
#include <cstdio>

class SerialLogger final : public mg::io::Logger {
    private:
        uint32_t baud;
        bool wait;

        static char levelToChar(mg::io::LogLevel level);
    
    protected:
        void _log(mg::io::LogLevel level, const char* tag, const char* fmt, va_list args) override;

    public:
        explicit SerialLogger(uint32_t baud = 115200, bool waitForSerial = false);

        bool init() override;
};
