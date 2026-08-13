#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace mg::json {

enum class Type : std::uint8_t;
class Reader;

struct ReaderBackend {
    Type (*type)(const void*) noexcept;
    bool (*readBool)(const void*, bool&) noexcept;
    bool (*readInt64)(const void*, std::int64_t&) noexcept;
    bool (*readUint64)(const void*, std::uint64_t&) noexcept;
    bool (*readDouble)(const void*, double&) noexcept;
    bool (*readString)(const void*, std::string_view&) noexcept;
    Reader (*member)(const void*, std::string_view) noexcept;
    Reader (*element)(const void*, std::size_t) noexcept;
    std::size_t (*size)(const void*) noexcept;
    bool (*forEachMember)(const void*, bool (*)(void*, std::string_view, Reader), void*) noexcept;
};

} // namespace mg::json
