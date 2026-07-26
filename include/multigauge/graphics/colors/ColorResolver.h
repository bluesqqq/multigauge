#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include <multigauge/graphics/colors/ColorFrame.h>

namespace mg::graphics {

class Color;

/// Resolves immutable Color definitions against one explicit ColorFrame.
/// A Frame token becomes invalid as soon as the resolver begins another frame.
class ColorResolver {
public:
    /// `capacity` is the maximum number of distinct color definitions that can
    /// be resolved in one frame without a fallback. Storage is allocated here,
    /// never by resolve().
    explicit ColorResolver(std::size_t capacity = 64);

    class Frame {
    public:
        Frame() = default;
        [[nodiscard]] bool valid() const noexcept;
        [[nodiscard]] const ColorFrame* data() const noexcept;
        [[nodiscard]] rgba resolve(const Color& color) const noexcept;

    private:
        friend class ColorResolver;
        Frame(ColorResolver* resolver, std::uint64_t generation) noexcept : resolver_(resolver), generation_(generation) {}

        ColorResolver* resolver_ = nullptr;
        std::uint64_t generation_ = 0;
    };

    [[nodiscard]] Frame beginFrame(const ColorFrame& frame) noexcept;
    /// Call outside rendering when a package needs a larger per-frame budget.
    void reserve(std::size_t colorCount) { cache_.reserve(colorCount); }

private:
    enum class State : std::uint8_t { Resolving, Resolved };
    struct CacheEntry {
        std::uint32_t id = 0;
        const Color* color = nullptr;
        std::uint64_t generation = 0;
        State state = State::Resolved;
        rgba value{};
    };

    [[nodiscard]] rgba resolve(const Frame& frame, const Color& color) noexcept;

    const ColorFrame* frame_ = nullptr;
    std::uint64_t generation_ = 0;
    std::vector<CacheEntry> cache_;
};

} // namespace mg::graphics
