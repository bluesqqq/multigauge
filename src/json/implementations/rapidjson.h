#pragma once

#include <string>
#include <string_view>
#include <vector>

#include <rapidjson/document.h>

#include <multigauge/json/Document.h>

namespace mg::json::implementations::rapidjson {

using RapidValue = ::rapidjson::Value;

struct DomWriter {
    struct Frame {
        RapidValue* value;
        bool object;
        std::string key;
    };

    ::rapidjson::Document& document;
    std::vector<Frame> frames;
    bool hasRoot = false;

    bool append(RapidValue value, RapidValue** inserted = nullptr) noexcept;
};

struct Storage {
    ::rapidjson::Document document;
    std::string buffer;
    DomWriter writer{document};
};

extern const ::mg::json::ReaderBackend readerBackend;
extern const ::mg::json::WriterBackend writerBackend;
extern const ::mg::json::DocumentBackend documentBackend;

bool writeNull(void* raw) noexcept;
bool writeBool(void* raw, bool v) noexcept;
bool writeInt64(void* raw, std::int64_t v) noexcept;
bool writeUint64(void* raw, std::uint64_t v) noexcept;
bool writeDouble(void* raw, double v) noexcept;
bool writeString(void* raw, std::string_view v) noexcept;
bool beginObject(void* raw) noexcept;
bool writeKey(void* raw, std::string_view key) noexcept;
bool endObject(void* raw) noexcept;
bool beginArray(void* raw) noexcept;
bool endArray(void* raw) noexcept;

void destroy(void* raw) noexcept;
Reader root(const void* raw) noexcept;
Writer writer(void* raw) noexcept;
std::string toString(const void* raw);

} // namespace mg::json::implementations::rapidjson
