#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

namespace mg::json {

enum class Type : std::uint8_t { Invalid, Null, Bool, Int, Uint, Number, String, Array, Object };

class Reader;
class Writer;
class ObjectWriter;
class ArrayWriter;

/// Backend operations for a non-owning JSON value view.
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

/// A small, non-owning view of a JSON value. String views borrow source storage.
class Reader {
public:
    using MemberVisitor = bool (*)(void*, std::string_view, Reader);

    constexpr Reader() = default;
    constexpr Reader(const ReaderBackend& backend, const void* value) noexcept : backend_(&backend), value_(value) {}

    [[nodiscard]] constexpr bool valid() const noexcept { return backend_ && value_; }
    [[nodiscard]] Type type() const noexcept { return valid() ? backend_->type(value_) : Type::Invalid; }
    [[nodiscard]] bool isNull() const noexcept { return type() == Type::Null; }
    [[nodiscard]] bool isObject() const noexcept { return type() == Type::Object; }
    [[nodiscard]] bool isArray() const noexcept { return type() == Type::Array; }

    bool read(bool& out) const noexcept { return valid() && backend_->readBool(value_, out); }
    bool read(std::int64_t& out) const noexcept { return valid() && backend_->readInt64(value_, out); }
    bool read(std::uint64_t& out) const noexcept { return valid() && backend_->readUint64(value_, out); }
    bool read(double& out) const noexcept { return valid() && backend_->readDouble(value_, out); }
    bool read(std::string_view& out) const noexcept { return valid() && backend_->readString(value_, out); }

    [[nodiscard]] Reader member(std::string_view key) const noexcept { return valid() ? backend_->member(value_, key) : Reader{}; }
    [[nodiscard]] Reader element(std::size_t index) const noexcept { return valid() ? backend_->element(value_, index) : Reader{}; }
    [[nodiscard]] std::size_t size() const noexcept { return valid() ? backend_->size(value_) : 0; }
    bool forEachMember(MemberVisitor visitor, void* context) const noexcept { return valid() && backend_->forEachMember(value_, visitor, context); }

private:
    const ReaderBackend* backend_ = nullptr;
    const void* value_ = nullptr;
};

/// Backend operations for a JSON output sink. These are for adapter authors.
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
    bool writeObject(Fn&& fn) {
        if (!beginObject()) return false;
        ObjectWriter object(*this);
        const bool result = std::forward<Fn>(fn)(object);
        return endObject() && result;
    }

    template <typename Fn>
    bool writeArray(Fn&& fn) {
        if (!beginArray()) return false;
        ArrayWriter array(*this);
        const bool result = std::forward<Fn>(fn)(array);
        return endArray() && result;
    }

private:
    friend class ObjectWriter;
    friend class ArrayWriter;
    bool beginObject() noexcept { return valid() && backend_->beginObject(sink_); }
    bool key(std::string_view value) noexcept { return valid() && backend_->writeKey(sink_, value); }
    bool endObject() noexcept { return valid() && backend_->endObject(sink_); }
    bool beginArray() noexcept { return valid() && backend_->beginArray(sink_); }
    bool endArray() noexcept { return valid() && backend_->endArray(sink_); }

    const WriterBackend* backend_ = nullptr;
    void* sink_ = nullptr;
};

class ObjectWriter {
public:
    explicit ObjectWriter(Writer& writer) noexcept : writer_(writer) {}
    Writer& writer() noexcept { return writer_; }
    bool write(std::string_view key, bool value) noexcept { return writer_.key(key) && writer_.write(value); }
    bool write(std::string_view key, int value) noexcept { return writer_.key(key) && writer_.write(value); }
    bool write(std::string_view key, std::int64_t value) noexcept { return writer_.key(key) && writer_.write(value); }
    bool write(std::string_view key, std::uint64_t value) noexcept { return writer_.key(key) && writer_.write(value); }
    bool write(std::string_view key, float value) noexcept { return writer_.key(key) && writer_.write(value); }
    bool write(std::string_view key, double value) noexcept { return writer_.key(key) && writer_.write(value); }
    bool write(std::string_view key, std::string_view value) noexcept { return writer_.key(key) && writer_.write(value); }
    bool write(std::string_view key, const std::string& value) noexcept { return write(key, std::string_view(value)); }
    bool write(std::string_view key, const char* value) noexcept { return writer_.key(key) && writer_.write(value); }
    bool write(std::string_view key, Reader value) noexcept { return writer_.key(key) && writer_.write(value); }

    template <typename Fn>
    bool writeValue(std::string_view key, Fn&& fn) { return writer_.key(key) && std::forward<Fn>(fn)(writer_); }

    template <typename Fn>
    bool writeObject(std::string_view key, Fn&& fn) { return writer_.key(key) && writer_.writeObject(std::forward<Fn>(fn)); }
    template <typename Fn>
    bool writeArray(std::string_view key, Fn&& fn) { return writer_.key(key) && writer_.writeArray(std::forward<Fn>(fn)); }

private:
    Writer& writer_;
};

class ArrayWriter {
public:
    explicit ArrayWriter(Writer& writer) noexcept : writer_(writer) {}
    Writer& writer() noexcept { return writer_; }
    bool write(bool value) noexcept { return writer_.write(value); }
    bool write(int value) noexcept { return writer_.write(value); }
    bool write(std::int64_t value) noexcept { return writer_.write(value); }
    bool write(std::uint64_t value) noexcept { return writer_.write(value); }
    bool write(float value) noexcept { return writer_.write(value); }
    bool write(double value) noexcept { return writer_.write(value); }
    bool write(std::string_view value) noexcept { return writer_.write(value); }
    bool write(const std::string& value) noexcept { return writer_.write(value); }
    bool write(const char* value) noexcept { return writer_.write(value); }
    bool write(Reader value) noexcept { return writer_.write(value); }
    template <typename Fn>
    bool writeObject(Fn&& fn) { return writer_.writeObject(std::forward<Fn>(fn)); }
    template <typename Fn>
    bool writeArray(Fn&& fn) { return writer_.writeArray(std::forward<Fn>(fn)); }
private:
    Writer& writer_;
};

/// Opaque, move-only owned JSON document supplied by the selected backend.
struct DocumentBackend {
    void (*destroy)(void*) noexcept;
    Reader (*root)(const void*) noexcept;
    Writer (*writer)(void*) noexcept;
    std::string (*toString)(const void*);
};

class Document {
public:
    Document() = default;
    Document(const Document&) = delete;
    Document& operator=(const Document&) = delete;
    Document(Document&& other) noexcept : backend_(other.backend_), storage_(std::exchange(other.storage_, nullptr)) {}
    Document& operator=(Document&& other) noexcept {
        if (this != &other) { reset(); backend_ = other.backend_; storage_ = std::exchange(other.storage_, nullptr); }
        return *this;
    }
    ~Document() { reset(); }

    [[nodiscard]] bool valid() const noexcept { return backend_ && storage_; }
    [[nodiscard]] Reader root() const noexcept { return valid() ? backend_->root(storage_) : Reader{}; }
    /// Returns a fresh writer for this document, discarding its previous root value.
    /// The returned writer must be used to write exactly one root JSON value.
    [[nodiscard]] Writer writer() noexcept { return valid() ? backend_->writer(storage_) : Writer{}; }
    [[nodiscard]] std::string toString() const { return valid() ? backend_->toString(storage_) : std::string{}; }

    static Document adopt(const DocumentBackend& backend, void* storage) noexcept { return Document(&backend, storage); }

private:
    Document(const DocumentBackend* backend, void* storage) noexcept : backend_(backend), storage_(storage) {}
    void reset() noexcept { if (storage_) backend_->destroy(storage_); storage_ = nullptr; backend_ = nullptr; }
    const DocumentBackend* backend_ = nullptr;
    void* storage_ = nullptr;
};

/// The currently linked backend's document factories (RapidJSON in this build).
Document parse(std::string_view text);
Document object();
Document array();

} // namespace mg::json
