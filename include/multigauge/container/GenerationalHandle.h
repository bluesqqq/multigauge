#pragma once

#include <cstdint>
#include <limits>
#include <type_traits>

namespace mg {

template<typename Tag, typename Storage = std::uint32_t, unsigned SlotBits = 16>
class GenerationalHandle {
public:
    /* ----- TYPES ----- */

    using StorageType = Storage;

    /* ----- CONSTRUCTOR ----- */

    constexpr GenerationalHandle() noexcept = default;

    /* ----- FACTORIES ----- */

    [[nodiscard]]
    static constexpr GenerationalHandle make(
        std::uint32_t slot,
        std::uint32_t generation
    ) noexcept {
        return GenerationalHandle{
            static_cast<Storage>(
                (static_cast<Storage>(generation) << SlotBits) |
                (static_cast<Storage>(slot) & SlotMask)
            )
        };
    }

    [[nodiscard]]
    static constexpr GenerationalHandle invalid() noexcept {
        return {};
    }

    /* ----- STATE QUERIES ----- */

    [[nodiscard]]
    constexpr bool valid() const noexcept {
        return value_ != InvalidValue;
    }

    /* ----- ACCESSORS ----- */

    [[nodiscard]]
    constexpr std::uint32_t slot() const noexcept {
        return static_cast<std::uint32_t>(value_ & SlotMask);
    }

    [[nodiscard]]
    constexpr std::uint32_t generation() const noexcept {
        return static_cast<std::uint32_t>(value_ >> SlotBits);
    }

    /* ----- MUTATION ----- */

    constexpr void clear() noexcept {
        value_ = InvalidValue;
    }

    /* ----- OPERATORS ----- */

    friend constexpr bool operator==(
        GenerationalHandle lhs,
        GenerationalHandle rhs
    ) noexcept = default;

private:
    /* ----- CONSTANTS ----- */

    static_assert(std::is_unsigned_v<Storage>);
    static_assert(SlotBits > 0);
    static_assert(SlotBits < std::numeric_limits<Storage>::digits);

    static constexpr Storage InvalidValue = 0;

    static constexpr Storage SlotMask =
        static_cast<Storage>(
            (Storage{1} << SlotBits) - Storage{1}
        );

    /* ----- PRIVATE CONSTRUCTOR ----- */

    explicit constexpr GenerationalHandle(Storage value) noexcept
        : value_(value) {}

    /* ----- DATA ----- */

    Storage value_ = InvalidValue;
};

} // namespace mg