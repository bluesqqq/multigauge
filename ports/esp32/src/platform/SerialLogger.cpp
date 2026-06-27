#include "SerialLogger.h"

char SerialLogger::levelToChar(mg::io::LogLevel level) {
    switch (level) {
        case mg::io::LogLevel::Debug: return 'D';
        case mg::io::LogLevel::Info:  return 'I';
        case mg::io::LogLevel::Warn:  return 'W';
        case mg::io::LogLevel::Error: return 'E';
    }
    return '?';
}

void SerialLogger::_log(mg::io::LogLevel level, const char *tag, const char *fmt, va_list args) {
    char msg[256];
    vsnprintf(msg, sizeof(msg), fmt, args);

    Serial.print('[');
    Serial.print(levelToChar(level));
    Serial.print("] ");

    if (tag && tag[0]) {
        Serial.print(tag);
        Serial.print(": ");
    }

    Serial.println(msg);
}

SerialLogger::SerialLogger(uint32_t baud, bool waitForSerial) : baud(baud), wait(waitForSerial) {}

bool SerialLogger::init() {
    Serial.begin(baud);
    if (wait) {
        while (!Serial) { delay(10); }
    }
    return true;
}
