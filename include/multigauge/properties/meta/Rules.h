#pragma once

#include <multigauge/json/Json.h>

#include <initializer_list>

namespace mg::rules {

/// @brief Writes a property rule with a single comparison value.
/// @param writer JSON writer receiving the rule object.
/// @param path Property path evaluated by the rule.
/// @param op Rule operator to apply.
/// @param value Value used by the rule.
/// @return true if the rule was written successfully; otherwise false.
bool writeRule(
    json::Writer& writer,
    const char* path,
    const char* op,
    const char* value
);

/// @brief Writes a property rule with multiple comparison values.
/// @param writer JSON writer receiving the rule object.
/// @param path Property path evaluated by the rule.
/// @param op Rule operator to apply.
/// @param values Values used by the rule.
/// @return true if the rule was written successfully; otherwise false.
bool writeRule(
    json::Writer& writer,
    const char* path,
    const char* op,
    std::initializer_list<const char*> values
);

} // namespace mg::rules
