#pragma once

#include <string>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

struct EditorResult {
    bool ok = false;
    rapidjson::Document data;
    std::string error;

    std::string toJson() const {
        rapidjson::Document d;
        d.SetObject();
        auto& a = d.GetAllocator();

        d.AddMember("ok", ok, a);

        if (ok) {
            rapidjson::Value dataCopy;
            dataCopy.CopyFrom(data, a);
            d.AddMember("data", std::move(dataCopy), a);
        } else {
            d.AddMember("error", rapidjson::Value(error.c_str(), a), a);
        }

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        d.Accept(writer);
        return buffer.GetString();
    }

    static EditorResult OkObject() {
        EditorResult r;
        r.ok = true;
        r.data.SetObject();
        return r;
    }

    static EditorResult OkArray() {
        EditorResult r;
        r.ok = true;
        r.data.SetArray();
        return r;
    }

    static EditorResult Error(const std::string& error) {
        EditorResult r;
        r.ok = false;
        r.error = error;
        return r;
    }
};

inline EditorResult OkObject() {
    return EditorResult::OkObject();
}

inline EditorResult OkArray() {
    return EditorResult::OkArray();
}

inline EditorResult Error(const std::string& error) {
    return EditorResult::Error(error);
}