#include <multigauge/properties/meta/Rules.h>

namespace mg::rules {
bool writeRule(json::Writer& writer, const char* path, const char* op, const char* value) {
    return writer.writeObject([&](json::ObjectWriter& rule) { return rule.write("path", path) && rule.write("op", op) && rule.write("value", value); });
}
bool writeRule(json::Writer& writer, const char* path, const char* op, std::initializer_list<const char*> values) {
    return writer.writeObject([&](json::ObjectWriter& rule) { return rule.write("path", path) && rule.write("op", op) && rule.writeArray("value", [&](json::ArrayWriter& array) { for (const char* value : values) if (!array.write(value)) return false; return true; }); });
}
} // namespace mg::rules
