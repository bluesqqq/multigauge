#pragma once

#include <multigauge/io/Logger.h>

namespace mg::io {

Logger* getLogger();

void setLogger(Logger* logger);

} // namespace mg::io

#define LOG_DEBUG(tag, fmt, ...)  do { if (auto* _l = mg::io::getLogger()) _l->log(mg::io::LogLevel::Debug, tag, fmt, ##__VA_ARGS__); } while (0)
#define LOG_INFO(tag,  fmt, ...)  do { if (auto* _l = mg::io::getLogger()) _l->log(mg::io::LogLevel::Info,  tag, fmt, ##__VA_ARGS__); } while (0)
#define LOG_WARN(tag,  fmt, ...)  do { if (auto* _l = mg::io::getLogger()) _l->log(mg::io::LogLevel::Warn,  tag, fmt, ##__VA_ARGS__); } while (0)
#define LOG_ERROR(tag, fmt, ...)  do { if (auto* _l = mg::io::getLogger()) _l->log(mg::io::LogLevel::Error, tag, fmt, ##__VA_ARGS__); } while (0)
