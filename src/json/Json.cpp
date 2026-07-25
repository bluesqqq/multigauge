#include <multigauge/json/Json.h>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <vector>

namespace mg::json {
namespace {

using RapidValue = rapidjson::Value;

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
bool readBool(const void* raw, bool& out) noexcept { const auto& v = *static_cast<const RapidValue*>(raw); if (!v.IsBool()) return false; out = v.GetBool(); return true; }
bool readInt64(const void* raw, std::int64_t& out) noexcept { const auto& v = *static_cast<const RapidValue*>(raw); if (!v.IsInt64()) return false; out = v.GetInt64(); return true; }
bool readUint64(const void* raw, std::uint64_t& out) noexcept { const auto& v = *static_cast<const RapidValue*>(raw); if (!v.IsUint64()) return false; out = v.GetUint64(); return true; }
bool readDouble(const void* raw, double& out) noexcept { const auto& v = *static_cast<const RapidValue*>(raw); if (!v.IsNumber()) return false; out = v.GetDouble(); return true; }
bool readString(const void* raw, std::string_view& out) noexcept { const auto& v = *static_cast<const RapidValue*>(raw); if (!v.IsString()) return false; out = std::string_view(v.GetString(), v.GetStringLength()); return true; }
Reader member(const void* raw, std::string_view key) noexcept;
Reader element(const void* raw, std::size_t index) noexcept;
std::size_t size(const void* raw) noexcept { const auto& v = *static_cast<const RapidValue*>(raw); return v.IsArray() || v.IsObject() ? v.Size() : 0; }
bool forEachMember(const void* raw, Reader::MemberVisitor visitor, void* context) noexcept;

const ReaderBackend readerBackend{readerType, readBool, readInt64, readUint64, readDouble, readString, member, element, size, forEachMember};
Reader member(const void* raw, std::string_view key) noexcept {
    const auto& value = *static_cast<const RapidValue*>(raw);
    if (!value.IsObject()) return {};
    const auto it = value.FindMember(rapidjson::StringRef(key.data(), static_cast<rapidjson::SizeType>(key.size())));
    return it == value.MemberEnd() ? Reader{} : Reader(readerBackend, &it->value);
}
Reader element(const void* raw, std::size_t index) noexcept {
    const auto& value = *static_cast<const RapidValue*>(raw);
    return !value.IsArray() || index >= value.Size() ? Reader{} : Reader(readerBackend, &value[static_cast<rapidjson::SizeType>(index)]);
}
bool forEachMember(const void* raw, Reader::MemberVisitor visitor, void* context) noexcept {
    const auto& value = *static_cast<const RapidValue*>(raw);
    if (!value.IsObject() || !visitor) return false;
    for (auto it = value.MemberBegin(); it != value.MemberEnd(); ++it) {
        if (!visitor(context, std::string_view(it->name.GetString(), it->name.GetStringLength()), Reader(readerBackend, &it->value))) return false;
    }
    return true;
}

struct DomWriter {
    struct Frame { RapidValue* value; bool object; std::string key; };
    rapidjson::Document& document;
    std::vector<Frame> frames;
    bool hasRoot = false;

    bool append(RapidValue value, RapidValue** inserted = nullptr) noexcept {
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
            frame.value->AddMember(RapidValue(frame.key.data(), static_cast<rapidjson::SizeType>(frame.key.size()), allocator), std::move(value), allocator);
            frame.key.clear();
            if (inserted) *inserted = &((frame.value->MemberEnd() - 1)->value);
        } else {
            frame.value->PushBack(std::move(value), allocator);
            if (inserted) *inserted = &(*(frame.value->End() - 1));
        }
        return true;
    }
};

bool writeNull(void* raw) noexcept { return static_cast<DomWriter*>(raw)->append(RapidValue(rapidjson::kNullType)); }
bool writeBool(void* raw, bool v) noexcept { return static_cast<DomWriter*>(raw)->append(RapidValue(v)); }
bool writeInt64(void* raw, std::int64_t v) noexcept { return static_cast<DomWriter*>(raw)->append(RapidValue(v)); }
bool writeUint64(void* raw, std::uint64_t v) noexcept { return static_cast<DomWriter*>(raw)->append(RapidValue(v)); }
bool writeDouble(void* raw, double v) noexcept { return static_cast<DomWriter*>(raw)->append(RapidValue(v)); }
bool writeString(void* raw, std::string_view v) noexcept { auto* writer = static_cast<DomWriter*>(raw); return writer->append(RapidValue(v.data(), static_cast<rapidjson::SizeType>(v.size()), writer->document.GetAllocator())); }
bool beginObject(void* raw) noexcept { auto* writer = static_cast<DomWriter*>(raw); RapidValue* value = nullptr; if (!writer->append(RapidValue(rapidjson::kObjectType), &value)) return false; writer->frames.push_back({value, true, {}}); return true; }
bool writeKey(void* raw, std::string_view key) noexcept { auto* writer = static_cast<DomWriter*>(raw); if (writer->frames.empty() || !writer->frames.back().object || !writer->frames.back().key.empty()) return false; writer->frames.back().key.assign(key); return true; }
bool endObject(void* raw) noexcept { auto* writer = static_cast<DomWriter*>(raw); if (writer->frames.empty() || !writer->frames.back().object || !writer->frames.back().key.empty()) return false; writer->frames.pop_back(); return true; }
bool beginArray(void* raw) noexcept { auto* writer = static_cast<DomWriter*>(raw); RapidValue* value = nullptr; if (!writer->append(RapidValue(rapidjson::kArrayType), &value)) return false; writer->frames.push_back({value, false, {}}); return true; }
bool endArray(void* raw) noexcept { auto* writer = static_cast<DomWriter*>(raw); if (writer->frames.empty() || writer->frames.back().object) return false; writer->frames.pop_back(); return true; }
const WriterBackend writerBackend{writeNull, writeBool, writeInt64, writeUint64, writeDouble, writeString, beginObject, writeKey, endObject, beginArray, endArray};

struct Storage { rapidjson::Document document; DomWriter writer{document}; };
void destroy(void* raw) noexcept { delete static_cast<Storage*>(raw); }
Reader root(const void* raw) noexcept { return Reader(readerBackend, &static_cast<const Storage*>(raw)->document); }
Writer writer(void* raw) noexcept { auto& state = static_cast<Storage*>(raw)->writer; state.frames.clear(); state.hasRoot = false; state.document.SetNull(); return Writer(writerBackend, &state); }
std::string toString(const void* raw) { rapidjson::StringBuffer buffer; rapidjson::Writer<rapidjson::StringBuffer> writer(buffer); static_cast<const Storage*>(raw)->document.Accept(writer); return buffer.GetString(); }
const DocumentBackend documentBackend{destroy, root, writer, toString};

bool writeMemberCopy(void* context, std::string_view key, Reader value) noexcept { return static_cast<ObjectWriter*>(context)->write(key, value); }
}

bool Writer::write(Reader value) noexcept {
    switch (value.type()) {
    case Type::Null: return null();
    case Type::Bool: { bool out; return value.read(out) && write(out); }
    case Type::Int: { std::int64_t out; return value.read(out) && write(out); }
    case Type::Uint: { std::uint64_t out; return value.read(out) && write(out); }
    case Type::Number: { double out; return value.read(out) && write(out); }
    case Type::String: { std::string_view out; return value.read(out) && write(out); }
    case Type::Array: return writeArray([&](ArrayWriter& array) { for (std::size_t i = 0; i < value.size(); ++i) if (!array.write(value.element(i))) return false; return true; });
    case Type::Object: return writeObject([&](ObjectWriter& object) { return value.forEachMember(&writeMemberCopy, &object); });
    default: return false;
    }
}

Document parse(std::string_view text) { auto* storage = new Storage; storage->document.Parse(text.data(), text.size()); if (storage->document.HasParseError()) { delete storage; return {}; } return Document::adopt(documentBackend, storage); }
Document object() { auto* storage = new Storage; storage->document.SetObject(); storage->writer.hasRoot = true; return Document::adopt(documentBackend, storage); }
Document array() { auto* storage = new Storage; storage->document.SetArray(); storage->writer.hasRoot = true; return Document::adopt(documentBackend, storage); }

} // namespace mg::json
