#pragma once

#include <multigauge/io/Log.h>

#include <rapidjson/document.h>

#include <optional>
#include <type_traits>
#include <vector>

const char* rjTypeName(const rapidjson::Value& v);

bool setInt(const rapidjson::Value::ConstObject& o, const char* k, int& out);
bool setFloat(const rapidjson::Value::ConstObject& o, const char* k, float& out);
bool setBool(const rapidjson::Value::ConstObject& o, const char* k, bool& out);
bool setCString(const rapidjson::Value::ConstObject& o, const char* k, const char*& out);
template <class T>
bool setObj(const rapidjson::Value::ConstObject& o, const char* k, T& out);
template <class T>
bool setObjVector(const rapidjson::Value::ConstObject& o, const char* k, std::vector<T>& out);
template <class T>
bool setValue(const rapidjson::Value::ConstObject& o, const char* k, T& out);


bool setOptInt(const rapidjson::Value::ConstObject& o, const char* k, std::optional<int>& out);
bool setOptFloat(const rapidjson::Value::ConstObject& o, const char* k, std::optional<float>& out);
template <class T>
bool setOptObj(const rapidjson::Value::ConstObject& o, const char* k, std::optional<T>& out);

template <class T>
bool setObj(const rapidjson::Value::ConstObject& o, const char* k, T& out) {
    static_assert(std::is_constructible_v<T, rapidjson::Value::ConstObject>, "T must be constructible from rapidjson::Value::ConstObject");

    auto it = o.FindMember(k);
    if (it == o.MemberEnd()) return false;
    if (!it->value.IsObject()) {
        LOG_WARN("setObj", "Key '%s' expected object but got %s", k, rjTypeName(it->value));
        return false;
    }
    out = T(it->value.GetObject());
    return true;
}

template <class T>
bool setObjVector(const rapidjson::Value::ConstObject& o, const char* k, std::vector<T>& out) {
    static_assert(std::is_constructible_v<T, rapidjson::Value::ConstObject> || std::is_constructible_v<T, const rapidjson::Value::ConstObject&>, "T must be constructible from rapidjson::Value::ConstObject");

    auto it = o.FindMember(k);
    if (it == o.MemberEnd()) return false;
    if (!it->value.IsArray()) {
        LOG_WARN("setObjVector", "Key '%s' expected array but got %s", k, rjTypeName(it->value));
        return false;
    }

    const auto arr = it->value.GetArray();
    out.clear();
    out.reserve(arr.Size());

    bool hadBad = false;
    for (rapidjson::SizeType i = 0; i < arr.Size(); ++i) {
        const auto& v = arr[i];
        if (!v.IsObject()) {
            hadBad = true;
            LOG_WARN("setObjVector", "Key '%s'[%u] expected object but got %s", k, (unsigned)i, rjTypeName(v));
            continue;
        }
        out.emplace_back(v.GetObject());
    }

    if (hadBad) LOG_WARN("setObjVector", "Key '%s' array contained non-objects; some entries were skipped", k);

    return true;
}

template <class T>
bool setValue(const rapidjson::Value::ConstObject& o, const char* k, T& out) {
    static_assert(std::is_constructible_v<T, const rapidjson::Value&>, "T must be constructible from const rapidjson::Value&");

    auto it = o.FindMember(k);
    if (it == o.MemberEnd()) return false;

    const rapidjson::Value& v = it->value;
    if (v.IsNull()) return false;

    out = T(v);
    return true;
}

template <class T>
bool setOptObj(const rapidjson::Value::ConstObject& o, const char* k, std::optional<T>& out) {
    auto it = o.FindMember(k);
    if (it == o.MemberEnd()) return false;

    const auto& v = it->value;
    if (v.IsNull()) { out.reset(); return true; }

    if (!v.IsObject()) {
        LOG_WARN("setOptObj", "Key '%s' expected object|null but got %s", k, rjTypeName(v));
        return false;
    }

    out = T(v.GetObject());
    return true;
}
