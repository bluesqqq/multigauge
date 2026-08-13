#pragma once

#include <cstdint>
#include <string_view>

namespace mg::json {

struct WriterBackend {
    bool (*writeNull)(void*) noexcept;
    bool (*writeBool)(void*, bool) noexcept;
    bool (*writeInt64)(void*, std::int64_t) noexcept;
    bool (*writeUint64)(void*, std::uint64_t) noexcept;
    bool (*writeDouble)(void*, double) noexcept;
    bool (*writeString)(void*, std::string_view) noexcept;
    bool (*beginObject)(void*) noexcept;
    bool (*writeKey)(void*, std::string_view) noexcept;
    bool (*endObject)(void*) noexcept;
    bool (*beginArray)(void*) noexcept;
    bool (*endArray)(void*) noexcept;
};

} // namespace mg::json
