#include <multigauge/value/ValueRef.h>

namespace mg {

ValueRef::ValueRef() noexcept = default;
ValueRef::ValueRef(ValueHandle handle) noexcept {
    setHandle(handle);
}

ValueRef::ValueRef(BuiltInValue value) noexcept : ValueRef(ValueRegistry::handle(value)) {
}

ValueRef::ValueRef(std::string_view id) noexcept {
    static_cast<void>(resolve(id));
}

bool ValueRef::resolve(std::string_view id) noexcept {
    handle_ = ValueRegistry::resolve(id);
    return handle_.valid();
}

void ValueRef::clear() noexcept {
    handle_.clear();
}

ValueHandle ValueRef::handle() const noexcept {
    return handle_;
}

void ValueRef::setHandle(ValueHandle handle) noexcept {
    handle_ = ValueRegistry::exists(handle) ? handle : ValueHandle::invalid();
}

std::string_view ValueRef::id() const noexcept {
    return ValueRegistry::id(handle_);
}

std::string_view ValueRef::name() const noexcept {
    return ValueRegistry::name(handle_);
}

const UnitType* ValueRef::unit() const noexcept {
    return ValueRegistry::unit(handle_);
}

Measurement ValueRef::minimum() const noexcept {
    return ValueRegistry::minimum(handle_);
}

Measurement ValueRef::maximum() const noexcept {
    return ValueRegistry::maximum(handle_);
}

Measurement ValueRef::value() const noexcept {
    return ValueRegistry::value(handle_);
}

bool ValueRef::setValue(Measurement value) noexcept {
    return ValueRegistry::set(handle_, value);
}

bool ValueRef::available() const noexcept {
    return ValueRegistry::available(handle_);
}

bool ValueRef::invalidate() noexcept {
    return ValueRegistry::invalidate(handle_);
}

ValueRef::operator bool() const noexcept {
    return ValueRegistry::exists(handle_);
}

bool operator==(const ValueRef& lhs, const ValueRef& rhs) noexcept {
    return lhs.handle_ == rhs.handle_;
}

} // namespace mg
