#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

#include <multigauge/json/WriterBackend.h>
#include <multigauge/json/Reader.h>

namespace mg::json {

class ObjectWriter;
class ArrayWriter;

class Writer {
public:
    Writer() = default;
    Writer(const Writer&) = delete;
    Writer& operator=(const Writer&) = delete;
    Writer(Writer&&) noexcept = default;
    Writer& operator=(Writer&&) noexcept = default;

    Writer(const WriterBackend& backend, void* sink) noexcept : backend_(&backend), sink_(sink) {}

    [[nodiscard]] bool valid() const noexcept { return backend_ && sink_; }
    bool null() noexcept { return valid() && backend_->writeNull(sink_); }
    bool write(bool value) noexcept { return valid() && backend_->writeBool(sink_, value); }
    bool write(std::int64_t value) noexcept { return valid() && backend_->writeInt64(sink_, value); }
    bool write(std::uint64_t value) noexcept { return valid() && backend_->writeUint64(sink_, value); }
    bool write(double value) noexcept { return valid() && backend_->writeDouble(sink_, value); }
    bool write(std::string_view value) noexcept { return valid() && backend_->writeString(sink_, value); }
    bool write(const char* value) noexcept { return value ? write(std::string_view(value)) : null(); }
    bool write(int value) noexcept { return write(static_cast<std::int64_t>(value)); }
    bool write(float value) noexcept { return write(static_cast<double>(value)); }
    bool write(const std::string& value) noexcept { return write(std::string_view(value)); }
    bool write(Reader value) noexcept;

    template <typename Fn>
    bool writeObject(Fn&& fn);

    template <typename Fn>
    bool writeArray(Fn&& fn);

private:
    friend class ObjectWriter;
    friend class ArrayWriter;
    bool beginObject() noexcept;
    bool key(std::string_view value) noexcept;
    bool endObject() noexcept;
    bool beginArray() noexcept;
    bool endArray() noexcept;

    const WriterBackend* backend_ = nullptr;
    void* sink_ = nullptr;
};

} // namespace mg::json

#include <multigauge/json/ObjectWriter.h>
#include <multigauge/json/ArrayWriter.h>
#include <multigauge/json/ObjectWriterTemplates.inl>
#include <multigauge/json/ArrayWriterTemplates.inl>
#include <multigauge/json/WriterTemplates.inl>
