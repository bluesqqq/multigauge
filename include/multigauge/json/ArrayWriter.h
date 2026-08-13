#pragma once

#include <string>
#include <string_view>
#include <utility>

#include <multigauge/json/Reader.h>

namespace mg::json {

class Writer;

class ArrayWriter {
public:
    explicit ArrayWriter(Writer& writer) noexcept : writer_(writer) {}
    Writer& writer() noexcept { return writer_; }
    bool write(bool value) noexcept;
    bool write(int value) noexcept;
    bool write(std::int64_t value) noexcept;
    bool write(std::uint64_t value) noexcept;
    bool write(float value) noexcept;
    bool write(double value) noexcept;
    bool write(std::string_view value) noexcept;
    bool write(const std::string& value) noexcept;
    bool write(const char* value) noexcept;
    bool write(Reader value) noexcept;

    template <typename Fn>
    bool writeObject(Fn&& fn);
    template <typename Fn>
    bool writeArray(Fn&& fn);

private:
    Writer& writer_;
};

} // namespace mg::json
