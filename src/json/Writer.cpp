#include <multigauge/json/Writer.h>
#include <multigauge/json/ArrayWriter.h>
#include <multigauge/json/ObjectWriter.h>

namespace mg::json {

namespace {

bool copyMember(void* context, std::string_view key, Reader value) noexcept {
    return static_cast<ObjectWriter*>(context)->write(key, value);
}

} // namespace

bool Writer::beginObject() noexcept { return valid() && backend_->beginObject(sink_); }
bool Writer::key(std::string_view value) noexcept { return valid() && backend_->writeKey(sink_, value); }
bool Writer::endObject() noexcept { return valid() && backend_->endObject(sink_); }
bool Writer::beginArray() noexcept { return valid() && backend_->beginArray(sink_); }
bool Writer::endArray() noexcept { return valid() && backend_->endArray(sink_); }

bool Writer::write(Reader value) noexcept {
    switch (value.type()) {
    case Type::Null:
        return null();
    case Type::Bool: {
        bool out;
        return value.read(out) && write(out);
    }
    case Type::Int: {
        std::int64_t out;
        return value.read(out) && write(out);
    }
    case Type::Uint: {
        std::uint64_t out;
        return value.read(out) && write(out);
    }
    case Type::Number: {
        double out;
        return value.read(out) && write(out);
    }
    case Type::String: {
        std::string_view out;
        return value.read(out) && write(out);
    }
    case Type::Array:
        return writeArray([&](ArrayWriter& array) {
            for (std::size_t i = 0; i < value.size(); ++i) {
                if (!array.write(value.element(i))) return false;
            }
            return true;
        });
    case Type::Object:
        return writeObject([&](ObjectWriter& object) {
            return value.forEachMember(&copyMember, &object);
        });
    default:
        return false;
    }
}

} // namespace mg::json
