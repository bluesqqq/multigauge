#pragma once

#include <rapidjson/document.h>
#include <string>

template<typename T>
struct Codec;

//----------[ PRIMITIVE TYPES ]----------//

// bool
template<>
struct Codec<bool> {
    static bool decode(const rapidjson::Value& v, bool& out) {
        if (!v.IsBool()) return false;
        out = v.GetBool();
        return true;
    }
};

// int
template<>
struct Codec<int> {
    static bool decode(const rapidjson::Value& v, int& out) {
        if (!v.IsNumber()) return false;
        out = v.GetInt();
        return true;
    }
};

// float
template<>
struct Codec<float> {
    static bool decode(const rapidjson::Value& v, float& out) {
        if (!v.IsNumber()) return false;
        out = v.GetFloat();
        return true;
    }
};

// string
template<>
struct Codec<std::string> {
    static bool decode(const rapidjson::Value& v, std::string& out) {
        if (!v.IsString()) return false;
        out.assign(v.GetString(), v.GetStringLength());
        return true;
    }
};

// const char *
template<>
struct Codec<const char*> {
    static bool decode(const rapidjson::Value& v, const char*& out) {
        if (!v.IsString()) return false;
        out = v.GetString();
        return true;
    }
};

template<typename T>
bool set(const rapidjson::Value::ConstObject& o, const char* key, T& out) {
    auto it = o.FindMember(key);
    if (it == o.MemberEnd()) return false;
    return Codec<T>::decode(it->value, out);
}

//----------[ CUSTOM TYPES ]----------//

template<>
struct Codec<rgba> {
    static bool decode(const rapidjson::Value& v, rgba& out) {
        if (v.IsString()) {
            rgba tmp(v.GetString());
            out = tmp;
            return true;
        }

        return false;
    }
};