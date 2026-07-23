#pragma once

#include <string>
#include <string_view>

#include <multigauge/properties/Codec.h>
#include <multigauge/properties/WidgetTraits.h>
#include <multigauge/value/Value.h>

namespace mg {

class ValueRef {
    CODEC_FRIEND(ValueRef)

    private:
        std::string id_;
        Value* value_ = nullptr;

    public:
        ValueRef() noexcept = default;

        explicit ValueRef(std::string_view id) : id_(id) { resolve(); }
        explicit ValueRef(Value* value) : id_(value ? std::string(value->id()) : ""), value_(value) {}

        bool resolve() noexcept {
            value_ = id_.empty() ? nullptr : Value::find(id_);
            return value_ != nullptr;
        }

        void clear() noexcept {
            id_.clear();
            value_ = nullptr;
        }

        [[nodiscard]]
        const std::string& id() const noexcept { return id_; }

        void setId(std::string_view newId) {
            id_ = newId;
            resolve();
        }

        [[nodiscard]]
        Value* get() const noexcept { return value_; }

        [[nodiscard]]
        Value& operator*() const noexcept { return *value_; }

        [[nodiscard]]
        Value* operator->() const noexcept { return value_; }

        [[nodiscard]]
        explicit operator bool() const noexcept { return value_ != nullptr; }
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

        out = ValueRef(std::string_view(v.GetString(), v.GetStringLength()));
        return true;
    }

    ENCODE() {
        if (v.id().empty()) {
            out.SetNull();
            return true;
        }

        out.SetString(v.id().c_str(), static_cast<rapidjson::SizeType>(v.id().size()), a);
        return true;
    }
CODEC_END()

} // namespace mg
