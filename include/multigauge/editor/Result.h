#pragma once

#include <multigauge/json/Json.h>

#include <string>

namespace mg {

struct Result {
    bool ok = false;
    json::Document data;
    std::string error;

    std::string toJson() const {
        json::Document document = json::object();
        json::Writer writer = document.writer();
        const bool written = writer.writeObject([&](json::ObjectWriter& object) {
            if (!object.write("ok", ok)) return false;
            return ok ? object.write("data", data.root()) : object.write("error", error);
        });
        return written ? document.toString() : std::string{};
    }

    static Result OkObject() { Result result; result.ok = true; result.data = json::object(); return result; }
    static Result OkArray() { Result result; result.ok = true; result.data = json::array(); return result; }
    static Result Error(const std::string& message) { Result result; result.error = message; return result; }
};

inline Result OkObject() { return Result::OkObject(); }
inline Result OkArray() { return Result::OkArray(); }
inline Result Error(const std::string& error) { return Result::Error(error); }

} // namespace mg
