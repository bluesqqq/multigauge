#pragma once

#include <string>

#include <multigauge/properties/Codec.h>
#include <multigauge/properties/WidgetTraits.h>
#include <multigauge/value/Value.h>

namespace mg {

class ValueRef {
    CODEC_FRIEND(ValueRef)

    private:
        std::string id;
        Value* ptr = nullptr;

    public:
        ValueRef() = default;

        explicit ValueRef(const char* newId) : id(newId ? newId : "") { resolve(); }
        explicit ValueRef(const std::string& newId) : id(newId) { resolve(); }
        explicit ValueRef(Value* value) : id(value ? std::string(value->id()) : ""), ptr(value) {}

        void resolve() { ptr = id.empty() ? nullptr : Value::find(id); }

        const std::string& getId() const { return id; }

        void setId(const std::string& newId) {
            id = newId;
            resolve();
        }

        Value* get() const { return ptr; }
        Value& operator*() const { return *ptr; }
        Value* operator->() const { return ptr; }
        explicit operator bool() const { return ptr != nullptr; }
};

template <>
struct MgPropWidgetTraits<ValueRef> {
    static constexpr const char* value = "value";
};

CODEC_BEGIN(ValueRef)
    DECODE() {
        if (v.IsNull()) {
            out = ValueRef();
            return true;
        }

        if (!v.IsString()) return false;

        out = ValueRef(std::string(v.GetString(), v.GetStringLength()));
        return true;
    }

    ENCODE() {
        if (v.getId().empty()) {
            out.SetNull();
            return true;
        }

        out.SetString(v.getId().c_str(), static_cast<rapidjson::SizeType>(v.getId().size()), a);
        return true;
    }
CODEC_END()

} // namespace mg
