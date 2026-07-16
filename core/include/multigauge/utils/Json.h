#pragma once

#include <cstdint>
#include <string>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <multigauge/io/FileSystem.h>

namespace mg::json {

//----------[ PARSING ]----------//

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

//----------[ FILE READ / WRITE ]----------//

inline bool readJsonFile(::mg::io::FileSystem& fs, const std::string& path, rapidjson::Document& out) {
    std::string json;
    if (!fs.readText(path, json)) return false;

    out = parseJson(json);
    return !out.HasParseError();
}


inline bool writeJsonFile(::mg::io::FileSystem& fs, const std::string& path, const rapidjson::Value& value) {
    return fs.writeText(path, toString(value));
}

//----------[ MEMBER GETTERS ]----------//

inline bool getStringMember(const rapidjson::Value& object, const char* key, std::string& out) {
    const auto it = object.FindMember(key);
    if (it == object.MemberEnd() || !it->value.IsString()) return false;

    out.assign(it->value.GetString(), it->value.GetStringLength());
    return true;
}

inline const rapidjson::Value* getObjectMember(const rapidjson::Value& object, const char* key) {
    const auto it = object.FindMember(key);
    if (it == object.MemberEnd() || !it->value.IsObject()) return nullptr;
    return &it->value;
}

inline const rapidjson::Value* getArrayMember(const rapidjson::Value& object, const char* key) {
    const auto it = object.FindMember(key);
    if (it == object.MemberEnd() || !it->value.IsArray()) return nullptr;
    return &it->value;
}

inline bool getBoolMember(const rapidjson::Value& object, const char* key, bool& out) {
    const auto it = object.FindMember(key);
    if (it == object.MemberEnd() || !it->value.IsBool()) return false;

    out = it->value.GetBool();
    return true;
}

inline bool getIntMember(const rapidjson::Value& object, const char* key, int& out) {
    const auto it = object.FindMember(key);
    if (it == object.MemberEnd() || !it->value.IsInt()) return false;

    out = it->value.GetInt();
    return true;
}

inline bool getUintMember(const rapidjson::Value& object, const char* key, unsigned& out) {
    const auto it = object.FindMember(key);
    if (it == object.MemberEnd() || !it->value.IsUint()) return false;

    out = it->value.GetUint();
    return true;
}

inline bool getInt64Member(const rapidjson::Value& object, const char* key, int64_t& out) {
    const auto it = object.FindMember(key);
    if (it == object.MemberEnd() || !it->value.IsInt64()) return false;

    out = it->value.GetInt64();
    return true;
}

inline bool getUint64Member(const rapidjson::Value& object, const char* key, uint64_t& out) {
    const auto it = object.FindMember(key);
    if (it == object.MemberEnd() || !it->value.IsUint64()) return false;

    out = it->value.GetUint64();
    return true;
}

inline bool getDoubleMember(const rapidjson::Value& object, const char* key, double& out) {
    const auto it = object.FindMember(key);
    if (it == object.MemberEnd() || !it->value.IsNumber()) return false;

    out = it->value.GetDouble();
    return true;
}

inline bool getNullMember(const rapidjson::Value& object, const char* key) {
    const auto it = object.FindMember(key);
    return it != object.MemberEnd() && it->value.IsNull();
}


} // namespace mg::json
