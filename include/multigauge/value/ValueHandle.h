#pragma once

#include <cstdint>

namespace mg {

class ValueRegistry; // Forward declaration

class ValueHandle {
public:
    /* ----- TYPES ----- */

    using Storage = std::uint16_t;

    /* ----- CONSTRUCTOR ----- */

    constexpr ValueHandle() noexcept = default;

    /* ----- FACTORIES ----- */

    [[nodiscard]]
    static constexpr ValueHandle invalid() noexcept {
        return ValueHandle{};
    }

    [[nodiscard]]
    static constexpr ValueHandle builtIn(Storage id) noexcept {
        return ValueHandle{id};
    }

    [[nodiscard]]
    static constexpr ValueHandle user(
        std::uint8_t slot,
        std::uint16_t generation
    ) noexcept {
        return ValueHandle{
            static_cast<Storage>(
                UserFlag |
                ((generation & GenerationMask) << GenerationShift) |
                (slot & UserSlotMask)
            )
        };
    }

    /* ----- STATE QUERIES ----- */

    [[nodiscard]]
    constexpr bool valid() const noexcept {
        return value_ != InvalidValue;
    }

    [[nodiscard]]
    constexpr bool isBuiltIn() const noexcept {
        return valid() && (value_ & UserFlag) == 0;
    }

    [[nodiscard]]
    constexpr bool isUser() const noexcept {
        return valid() && (value_ & UserFlag) != 0;
    }

    /* ----- ACCESSORS ----- */

    [[nodiscard]]
    constexpr std::uint16_t builtInId() const noexcept {
        return value_;
    }

    [[nodiscard]]
    constexpr std::uint8_t userSlot() const noexcept {
        return static_cast<std::uint8_t>(value_ & UserSlotMask);
    }

    [[nodiscard]]
    constexpr std::uint16_t userGeneration() const noexcept {
        return static_cast<std::uint16_t>(
            (value_ >> GenerationShift) & GenerationMask
        );
    }

    /* ----- MUTATION ----- */

    constexpr void clear() noexcept {
        value_ = InvalidValue;
    }

    /* ----- OPERATORS ----- */

    friend constexpr bool operator==(
        ValueHandle lhs,
        ValueHandle rhs
    ) noexcept = default;

private:
    /* ----- ENCODING CONSTANTS ----- */

    static constexpr Storage InvalidValue = 0xFFFF;
    static constexpr Storage UserFlag = 0x8000;
    static constexpr Storage UserSlotMask = 0x000F;
    static constexpr Storage GenerationMask = 0x07FF;
    static constexpr unsigned GenerationShift = 4;

    /* ----- PRIVATE CONSTRUCTOR ----- */

    explicit constexpr ValueHandle(Storage value) noexcept
        : value_(value) {}

    /* ----- DATA ----- */

    Storage value_ = InvalidValue;

    /* ----- FRIENDS ----- */

    friend class ValueRegistry;
};

} // namespace mg