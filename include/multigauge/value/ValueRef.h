#pragma once

#include <multigauge/properties/Codec.h>
#include <multigauge/properties/WidgetTraits.h>
#include <multigauge/value/ValueRegistry.h>

namespace mg {

/// Serializable two-byte reference to a registry value.
/// @note Decode succeeds only after the referenced value has been registered.
class ValueRef {
    CODEC_FRIEND(ValueRef)

public:
    /* ----- CONSTRUCTORS ----- */

    ValueRef() noexcept;

    explicit ValueRef(ValueHandle handle) noexcept;

    explicit ValueRef(BuiltInValue value) noexcept;
    
    explicit ValueRef(std::string_view id) noexcept;

    /* ----- REFERENCE MANAGEMENT ----- */

    [[nodiscard]]
    bool resolve(std::string_view id) noexcept;

    void clear() noexcept;

    [[nodiscard]]
    ValueHandle handle() const noexcept;

    void setHandle(ValueHandle handle) noexcept;

    /* ----- METADATA ----- */

    [[nodiscard]]
    std::string_view id() const noexcept;

    [[nodiscard]]
    std::string_view name() const noexcept;

    [[nodiscard]]
    const UnitType* unit() const noexcept;

    [[nodiscard]]
    Measurement minimum() const noexcept;

    [[nodiscard]]
    Measurement maximum() const noexcept;

    /* ----- VALUE ACCESS ----- */

    [[nodiscard]]
    Measurement value() const noexcept;

    [[nodiscard]]
    bool available() const noexcept;

    /* ----- VALUE MUTATION ----- */

    [[nodiscard]]
    bool setValue(Measurement value) noexcept;

    [[nodiscard]]
    bool invalidate() noexcept;

    /* ----- OPERATORS ----- */

    [[nodiscard]]
    explicit operator bool() const noexcept;

    friend bool operator==(
        const ValueRef& lhs,
        const ValueRef& rhs
    ) noexcept;

private:
    /* ----- DATA ----- */

    ValueHandle handle_{};
};

template<>
struct MgPropWidgetTraits<ValueRef> { static constexpr const char* value = "value"; };

CODEC_BEGIN(ValueRef)
    DECODE() {
        if (v.isNull()) {
            out.clear();
            return true;
        }

        std::string_view id;
        if (!v.read(id)) return false;
        
        return out.resolve(id);
    }

    ENCODE() {
        return v.id().empty() ? out.null() : out.write(v.id());
    }
CODEC_END()

} // namespace mg
