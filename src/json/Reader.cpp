#include <multigauge/json/Reader.h>

namespace mg::json {

bool Reader::valid() const noexcept { return backend_ && value_; }
Type Reader::type() const noexcept { return valid() ? backend_->type(value_) : Type::Invalid; }
bool Reader::isNull() const noexcept { return type() == Type::Null; }
bool Reader::isObject() const noexcept { return type() == Type::Object; }
bool Reader::isArray() const noexcept { return type() == Type::Array; }

bool Reader::read(bool& out) const noexcept { return valid() && backend_->readBool(value_, out); }
bool Reader::read(std::int64_t& out) const noexcept { return valid() && backend_->readInt64(value_, out); }
bool Reader::read(std::uint64_t& out) const noexcept { return valid() && backend_->readUint64(value_, out); }
bool Reader::read(double& out) const noexcept { return valid() && backend_->readDouble(value_, out); }
bool Reader::read(std::string_view& out) const noexcept { return valid() && backend_->readString(value_, out); }

Reader Reader::member(std::string_view key) const noexcept { return valid() ? backend_->member(value_, key) : Reader{}; }
Reader Reader::element(std::size_t index) const noexcept { return valid() ? backend_->element(value_, index) : Reader{}; }
std::size_t Reader::size() const noexcept { return valid() ? backend_->size(value_) : 0; }
bool Reader::forEachMember(MemberVisitor visitor, void* context) const noexcept { return valid() && backend_->forEachMember(value_, visitor, context); }

} // namespace mg::json
