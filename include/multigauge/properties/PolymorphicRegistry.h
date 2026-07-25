#pragma once

#include <multigauge/json/Json.h>

#include <cstddef>
#include <string_view>

namespace mg {
template <typename Owned, typename... Args> struct MgPolymorphicTypeDescriptor { const char* id; const char* name; Owned (*create)(Args...); };
template <typename Derived, typename Owned, typename... Args> constexpr MgPolymorphicTypeDescriptor<Owned, Args...> makePolymorphicTypeDescriptor(Owned (*create)(Args...)) { return {Derived::staticTypeId(), Derived::staticTypeName(), create}; }
template <typename Owned, typename... Args> class MgPolymorphicRegistry {
public:
    using Descriptor = MgPolymorphicTypeDescriptor<Owned, Args...>; using Creator = Owned (*)(Args...);
    constexpr MgPolymorphicRegistry(const Descriptor* descriptors, std::size_t count, Creator fallback) : descriptors_(descriptors), count_(count), fallback_(fallback) {}
    const Descriptor* find(std::string_view type) const { if (type.empty()) return nullptr; for (std::size_t i = 0; i < count_; ++i) if (descriptors_[i].id && type == descriptors_[i].id) return &descriptors_[i]; return nullptr; }
    Owned create(const char* type, Args... args) const { if (const Descriptor* descriptor = find(type ? std::string_view(type) : std::string_view{}); descriptor && descriptor->create) return descriptor->create(args...); return fallback_ ? fallback_(args...) : Owned{}; }
    const Descriptor* begin() const { return descriptors_; } const Descriptor* end() const { return descriptors_ + count_; } std::size_t size() const { return count_; }
    bool writeTypesMeta(json::Writer& writer) const { return writer.writeArray([&](json::ArrayWriter& types) { for (std::size_t i = 0; i < count_; ++i) { const auto& d = descriptors_[i]; if (!types.writeObject([&](json::ObjectWriter& entry) { return entry.write("id", d.id ? d.id : "") && entry.write("name", d.name ? d.name : ""); })) return false; } return true; }); }
private: const Descriptor* descriptors_ = nullptr; std::size_t count_ = 0; Creator fallback_ = nullptr;
};
template <typename T, typename = void> struct MgPolymorphicRegistryTraits { static constexpr bool supported = false; static bool getTypesMeta(json::Writer& writer) { return writer.writeArray([](json::ArrayWriter&) { return true; }); } };
#define MG_POLYMORPHIC_REGISTRY(owned_type) using Registry = MgPolymorphicRegistry<owned_type>; static const Registry& registry();
#define MG_POLYMORPHIC_REGISTRY_WITH_ARGS(owned_type, ...) using Registry = MgPolymorphicRegistry<owned_type, __VA_ARGS__>; static const Registry& registry();
} // namespace mg
