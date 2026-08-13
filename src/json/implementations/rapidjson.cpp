#include "rapidjson.h"

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace mg::json::implementations::rapidjson {

Type readerType(const void* raw) noexcept {
    const auto& value = *static_cast<const RapidValue*>(raw);
    if (value.IsNull()) return Type::Null;
    if (value.IsBool()) return Type::Bool;
    if (value.IsInt64()) return Type::Int;
    if (value.IsUint64()) return Type::Uint;
    if (value.IsNumber()) return Type::Number;
    if (value.IsString()) return Type::String;
    if (value.IsArray()) return Type::Array;
    if (value.IsObject()) return Type::Object;
    return Type::Invalid;
}

bool readBool(const void* raw, bool& out) noexcept {
    const auto& value = *static_cast<const RapidValue*>(raw);
    if (!value.IsBool()) return false;
    out = value.GetBool();
    return true;
}

bool readInt64(const void* raw, std::int64_t& out) noexcept {
    const auto& value = *static_cast<const RapidValue*>(raw);
    if (!value.IsInt64()) return false;
    out = value.GetInt64();
    return true;
}

bool readUint64(const void* raw, std::uint64_t& out) noexcept {
    const auto& value = *static_cast<const RapidValue*>(raw);
    if (!value.IsUint64()) return false;
    out = value.GetUint64();
    return true;
}

bool readDouble(const void* raw, double& out) noexcept {
    const auto& value = *static_cast<const RapidValue*>(raw);
    if (!value.IsNumber()) return false;
    out = value.GetDouble();
    return true;
}

bool readString(const void* raw, std::string_view& out) noexcept {
    const auto& value = *static_cast<const RapidValue*>(raw);
    if (!value.IsString()) return false;
    out = std::string_view(value.GetString(), value.GetStringLength());
    return true;
}

Reader member(const void* raw, std::string_view key) noexcept {
    const auto& value = *static_cast<const RapidValue*>(raw);
    if (!value.IsObject()) return {};
    const auto it = value.FindMember(::rapidjson::StringRef(key.data(), static_cast<::rapidjson::SizeType>(key.size())));
    return it == value.MemberEnd() ? Reader{} : Reader(readerBackend, &it->value);
}

Reader element(const void* raw, std::size_t index) noexcept {
    const auto& value = *static_cast<const RapidValue*>(raw);
    return !value.IsArray() || index >= value.Size() ? Reader{} : Reader(readerBackend, &value[static_cast<::rapidjson::SizeType>(index)]);
}

std::size_t size(const void* raw) noexcept {
    const auto& value = *static_cast<const RapidValue*>(raw);
    return value.IsArray() || value.IsObject() ? value.Size() : 0;
}

bool forEachMember(const void* raw, Reader::MemberVisitor visitor, void* context) noexcept {
    const auto& value = *static_cast<const RapidValue*>(raw);
    if (!value.IsObject() || !visitor) return false;
    for (auto it = value.MemberBegin(); it != value.MemberEnd(); ++it) {
        if (!visitor(context, std::string_view(it->name.GetString(), it->name.GetStringLength()), Reader(readerBackend, &it->value))) {
            return false;
        }
    }
    return true;
}

bool DomWriter::append(RapidValue value, RapidValue** inserted) noexcept {
    auto& allocator = document.GetAllocator();
    if (frames.empty()) {
        if (hasRoot) return false;
        document.CopyFrom(value, allocator);
        hasRoot = true;
        if (inserted) *inserted = &document;
        return true;
    }
    Frame& frame = frames.back();
    if (frame.object) {
        if (frame.key.empty()) return false;
        frame.value->AddMember(RapidValue(frame.key.data(), static_cast<::rapidjson::SizeType>(frame.key.size()), allocator), std::move(value), allocator);
        frame.key.clear();
        if (inserted) *inserted = &((frame.value->MemberEnd() - 1)->value);
    } else {
        frame.value->PushBack(std::move(value), allocator);
        if (inserted) *inserted = &(*(frame.value->End() - 1));
    }
    return true;
}

bool writeNull(void* raw) noexcept { return static_cast<DomWriter*>(raw)->append(RapidValue(::rapidjson::kNullType)); }
bool writeBool(void* raw, bool value) noexcept { return static_cast<DomWriter*>(raw)->append(RapidValue(value)); }
bool writeInt64(void* raw, std::int64_t value) noexcept { return static_cast<DomWriter*>(raw)->append(RapidValue(value)); }
bool writeUint64(void* raw, std::uint64_t value) noexcept { return static_cast<DomWriter*>(raw)->append(RapidValue(value)); }
bool writeDouble(void* raw, double value) noexcept { return static_cast<DomWriter*>(raw)->append(RapidValue(value)); }
bool writeString(void* raw, std::string_view value) noexcept {
    auto* writer = static_cast<DomWriter*>(raw);
    return writer->append(RapidValue(value.data(), static_cast<::rapidjson::SizeType>(value.size()), writer->document.GetAllocator()));
}
bool beginObject(void* raw) noexcept {
    auto* writer = static_cast<DomWriter*>(raw);
    RapidValue* value = nullptr;
    if (!writer->append(RapidValue(::rapidjson::kObjectType), &value)) return false;
    writer->frames.push_back({value, true, {}});
    return true;
}
bool writeKey(void* raw, std::string_view key) noexcept {
    auto* writer = static_cast<DomWriter*>(raw);
    if (writer->frames.empty() || !writer->frames.back().object || !writer->frames.back().key.empty()) return false;
    writer->frames.back().key.assign(key);
    return true;
}
bool endObject(void* raw) noexcept {
    auto* writer = static_cast<DomWriter*>(raw);
    if (writer->frames.empty() || !writer->frames.back().object || !writer->frames.back().key.empty()) return false;
    writer->frames.pop_back();
    return true;
}
bool beginArray(void* raw) noexcept {
    auto* writer = static_cast<DomWriter*>(raw);
    RapidValue* value = nullptr;
    if (!writer->append(RapidValue(::rapidjson::kArrayType), &value)) return false;
    writer->frames.push_back({value, false, {}});
    return true;
}
bool endArray(void* raw) noexcept {
    auto* writer = static_cast<DomWriter*>(raw);
    if (writer->frames.empty() || writer->frames.back().object) return false;
    writer->frames.pop_back();
    return true;
}

const ::mg::json::ReaderBackend readerBackend{
    readerType,
    readBool,
    readInt64,
    readUint64,
    readDouble,
    readString,
    member,
    element,
    size,
    forEachMember,
};

const ::mg::json::WriterBackend writerBackend{
    writeNull,
    writeBool,
    writeInt64,
    writeUint64,
    writeDouble,
    writeString,
    beginObject,
    writeKey,
    endObject,
    beginArray,
    endArray,
};

void destroy(void* raw) noexcept {
    delete static_cast<Storage*>(raw);
}

Reader root(const void* raw) noexcept {
    return Reader(readerBackend, &static_cast<const Storage*>(raw)->document);
}

Writer writer(void* raw) noexcept {
    auto& state = static_cast<Storage*>(raw)->writer;
    state.frames.clear();
    state.hasRoot = false;
    state.document.SetNull();
    return Writer(writerBackend, &state);
}

std::string toString(const void* raw) {
    ::rapidjson::StringBuffer buffer;
    ::rapidjson::Writer<::rapidjson::StringBuffer> writer(buffer);
    static_cast<const Storage*>(raw)->document.Accept(writer);
    return buffer.GetString();
}

const ::mg::json::DocumentBackend documentBackend{
    destroy,
    root,
    writer,
    toString,
};

} // namespace mg::json::implementations::rapidjson
