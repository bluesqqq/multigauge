#pragma once

#include <multigauge/io/Logger.h>

namespace mg {

Logger* getLogger();

void setLogger(Logger* logger);

}

#define LOG_DEBUG(tag, fmt, ...)  do { if (auto* _l = mg::getLogger()) _l->log(LogLevel::Debug, tag, fmt, ##__VA_ARGS__); } while (0)
#define LOG_INFO(tag,  fmt, ...)  do { if (auto* _l = mg::getLogger()) _l->log(LogLevel::Info,  tag, fmt, ##__VA_ARGS__); } while (0)
#define LOG_WARN(tag,  fmt, ...)  do { if (auto* _l = mg::getLogger()) _l->log(LogLevel::Warn,  tag, fmt, ##__VA_ARGS__); } while (0)
#define LOG_ERROR(tag, fmt, ...)  do { if (auto* _l = mg::getLogger()) _l->log(LogLevel::Error, tag, fmt, ##__VA_ARGS__); } while (0)