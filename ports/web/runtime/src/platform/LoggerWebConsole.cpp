#include "LoggerWebConsole.h"

#include <cstdio>
#include <string>

#include <emscripten.h>

using mg::io::LogLevel;

static const char* levelToString(LogLevel l) {
    switch (l) {
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info:  return "INFO";
        case LogLevel::Warn:  return "WARN";
        case LogLevel::Error: return "ERROR";
        default: return "?";
    }
}

EM_JS(void, mg_console_log, (int level, const char* cstr), {
    const s = UTF8ToString(cstr);
    if (level === 0) console.debug(s);
    else if (level === 1) console.info(s);
    else if (level === 2) console.warn(s);
    else console.error(s);
});

bool LoggerWebConsole::init() { return true; }

void LoggerWebConsole::_log(LogLevel level, const char* tag, const char* fmt, va_list args) {
    char msgBuf[1024];
    std::vsnprintf(msgBuf, sizeof(msgBuf), fmt, args);

    char finalBuf[1200];
    std::snprintf(finalBuf, sizeof(finalBuf), "[%s] %s: %s", levelToString(level), tag ? tag : "log", msgBuf);

    mg_console_log((int)level, finalBuf);
}
