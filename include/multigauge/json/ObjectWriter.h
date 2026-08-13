#pragma once

#include <string>
#include <string_view>
#include <utility>

#include <multigauge/json/Reader.h>

namespace mg::json {

class Writer;

class ObjectWriter {
public:
    explicit ObjectWriter(Writer& writer) noexcept : writer_(writer) {}
    Writer& writer() noexcept { return writer_; }
    bool write(std::string_view key, bool value) noexcept;
    bool write(std::string_view key, int value) noexcept;
    bool write(std::string_view key, std::int64_t value) noexcept;
    bool write(std::string_view key, std::uint64_t value) noexcept;
    bool write(std::string_view key, float value) noexcept;
    bool write(std::string_view key, double value) noexcept;
    bool write(std::string_view key, std::string_view value) noexcept;
    bool write(std::string_view key, const std::string& value) noexcept;
    bool write(std::string_view key, const char* value) noexcept;
    bool write(std::string_view key, Reader value) noexcept;

    template <typename Fn>
    bool writeValue(std::string_view key, Fn&& fn);

    template <typename Fn>
    bool writeObject(std::string_view key, Fn&& fn);
    template <typename Fn>
    bool writeArray(std::string_view key, Fn&& fn);

private:
    Writer& writer_;
};

} // namespace mg::json
