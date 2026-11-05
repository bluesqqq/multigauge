#pragma once

#include <multigauge/io/FileSystem.h>
#include <multigauge/json/Json.h>

namespace mg::json {

/// Bounded default for settings, sensor/control configuration, manifests, and faces.
inline constexpr size_t DefaultJsonFileLimit = 256 * 1024;

inline bool readJsonFile(::mg::io::FileSystem& fs, const std::string& path, Document& out, size_t maxBytes = DefaultJsonFileLimit) {
    std::string text;
    if (!fs.readText(path, text, maxBytes)) return false;
    out = parse(text);
    return out.valid();
}

inline bool writeJsonFile(::mg::io::FileSystem& fs, const std::string& path, Reader value) {
    Document document = object();
    Writer writer = document.writer();
    if (!writer.write(value)) return false;
    return fs.writeText(path, document.toString());
}

inline bool getStringMember(Reader object, std::string_view key, std::string& out) {
    std::string_view value;
    if (!object.member(key).read(value)) return false;
    out.assign(value);
    return true;
}

inline Reader getObjectMember(Reader object, std::string_view key) {
    Reader value = object.member(key);
    return value.isObject() ? value : Reader{};
}

inline Reader getArrayMember(Reader object, std::string_view key) {
    Reader value = object.member(key);
    return value.isArray() ? value : Reader{};
}

} // namespace mg::json
