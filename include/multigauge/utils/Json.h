#pragma once

#include <string>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace mg::json {

inline rapidjson::Document parseJson(const std::string& json) {
    rapidjson::Document doc;
    doc.Parse(json.c_str());
    return doc;
}

inline std::string toString(const rapidjson::Value& value) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    value.Accept(writer);
    return buffer.GetString();
}

inline std::string toString(const rapidjson::Document& doc) {
    return toString(static_cast<const rapidjson::Value&>(doc));
}

} // namespace mg::json
