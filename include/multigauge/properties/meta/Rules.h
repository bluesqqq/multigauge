#pragma once

#include <multigauge/json/Json.h>

#include <initializer_list>

namespace mg::rules {
bool writeRule(json::Writer& writer, const char* path, const char* op, const char* value);
bool writeRule(json::Writer& writer, const char* path, const char* op, std::initializer_list<const char*> values);
} // namespace mg::rules
