#pragma once

#include <string>

#include <rapidjson/document.h>

#include <multigauge/utils/Json.h>

namespace mg {

struct Result {
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

        return ::mg::json::toString(d);
    }

    static Result OkObject() {
        Result r;
        r.ok = true;
        r.data.SetObject();
        return r;
    }

    static Result OkArray() {
        Result r;
        r.ok = true;
        r.data.SetArray();
        return r;
    }

    static Result Error(const std::string& error) {
        Result r;
        r.ok = false;
        r.error = error;
        return r;
    }
};

inline Result OkObject() { return Result::OkObject(); }

inline Result OkArray() { return Result::OkArray(); }

inline Result Error(const std::string& error) { return Result::Error(error); }

} // namespace mg
