#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

#include <multigauge/json/ReaderBackend.h>

namespace mg::json {

enum class Type : std::uint8_t { Invalid, Null, Bool, Int, Uint, Number, String, Array, Object };

class Reader;

class Reader {
public:
    using MemberVisitor = bool (*)(void*, std::string_view, Reader);

    constexpr Reader() = default;
    constexpr Reader(const ReaderBackend& backend, const void* value) noexcept : backend_(&backend), value_(value) {}
    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] Type type() const noexcept;
    [[nodiscard]] bool isNull() const noexcept;
    [[nodiscard]] bool isObject() const noexcept;
    [[nodiscard]] bool isArray() const noexcept;

    bool read(bool& out) const noexcept;
    bool read(std::int64_t& out) const noexcept;
    bool read(std::uint64_t& out) const noexcept;
    bool read(double& out) const noexcept;
    bool read(std::string_view& out) const noexcept;

    [[nodiscard]] Reader member(std::string_view key) const noexcept;
    [[nodiscard]] Reader element(std::size_t index) const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;
    bool forEachMember(MemberVisitor visitor, void* context) const noexcept;

private:
    const ReaderBackend* backend_ = nullptr;
    const void* value_ = nullptr;
};

} // namespace mg::json
