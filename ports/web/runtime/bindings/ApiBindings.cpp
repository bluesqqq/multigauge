#include "ApiBindings.h"

#include <utility>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace mg::bindings {

const char* storeString(std::string value) {
    static thread_local std::string storage;
    storage = std::move(value);
    return storage.c_str();
}

const char* storeString(const char* value) {
    static thread_local std::string storage;
    storage = value ? value : "";
    return storage.c_str();
}

namespace {

const char* clipboardKindName(mg::editor::ClipboardState::Kind kind) {
    switch (kind) {
        case mg::editor::ClipboardState::Kind::Face: return "face";
        case mg::editor::ClipboardState::Kind::Element: return "element";
        case mg::editor::ClipboardState::Kind::Empty:
        default: return "empty";
    }
}

}

std::string toJson(const mg::editor::ClipboardSummary& summary) {
    rapidjson::Document document;
    document.SetObject();

    auto& allocator = document.GetAllocator();
    document.AddMember("kind", rapidjson::Value(clipboardKindName(summary.kind), allocator), allocator);
    document.AddMember("hasValue", summary.hasValue(), allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);
    return buffer.GetString();
}

}
