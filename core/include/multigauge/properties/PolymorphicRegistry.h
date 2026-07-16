#pragma once

#include <cstddef>
#include <cstring>

#include <rapidjson/document.h>

namespace mg {

template <typename Owned, typename... Args>
struct MgPolymorphicTypeDescriptor {
    const char* id;
    const char* name;
    Owned (*create)(Args...);
};

template <typename Derived, typename Owned, typename... Args>
constexpr MgPolymorphicTypeDescriptor<Owned, Args...> makePolymorphicTypeDescriptor(Owned (*create)(Args...)) {
    return {Derived::staticTypeId(), Derived::staticTypeName(), create};
}

template <typename Owned, typename... Args>
class MgPolymorphicRegistry {
public:
    using Descriptor = MgPolymorphicTypeDescriptor<Owned, Args...>;
    using Creator = Owned (*)(Args...);

    constexpr MgPolymorphicRegistry(const Descriptor* descriptors, std::size_t count, Creator fallback)
        : descriptors_(descriptors), count_(count), fallback_(fallback) {}

    const Descriptor* find(const char* type) const {
        if (!type) return nullptr;

        for (std::size_t i = 0; i < count_; ++i) {
            const Descriptor& descriptor = descriptors_[i];
            if (descriptor.id && std::strcmp(descriptor.id, type) == 0) {
                return &descriptor;
            }
        }

        return nullptr;
    }

    Owned create(const char* type, Args... args) const {
        if (const Descriptor* descriptor = find(type)) {
            if (descriptor->create) return descriptor->create(args...);
        }

        return fallback_ ? fallback_(args...) : Owned{};
    }

    const Descriptor* begin() const { return descriptors_; }
    const Descriptor* end() const { return descriptors_ + count_; }
    std::size_t size() const { return count_; }

    rapidjson::Value getTypesMeta(rapidjson::Document::AllocatorType& a) const {
        rapidjson::Value types(rapidjson::kArrayType);

        for (std::size_t i = 0; i < count_; ++i) {
            const Descriptor& descriptor = descriptors_[i];
            rapidjson::Value entry(rapidjson::kObjectType);
            entry.AddMember("id", rapidjson::Value(descriptor.id ? descriptor.id : "", a), a);
            entry.AddMember("name", rapidjson::Value(descriptor.name ? descriptor.name : "", a), a);
            types.PushBack(std::move(entry), a);
        }

        return types;
    }

private:
    const Descriptor* descriptors_ = nullptr;
    std::size_t count_ = 0;
    Creator fallback_ = nullptr;
};

template <typename T, typename = void>
struct MgPolymorphicRegistryTraits {
    static constexpr bool supported = false;

    static rapidjson::Value getTypesMeta(rapidjson::Document::AllocatorType&) {
        return rapidjson::Value(rapidjson::kArrayType);
    }
};

#define MG_POLYMORPHIC_REGISTRY(owned_type) \
    using Registry = MgPolymorphicRegistry<owned_type>; \
    static const Registry& registry();

#define MG_POLYMORPHIC_REGISTRY_WITH_ARGS(owned_type, ...) \
    using Registry = MgPolymorphicRegistry<owned_type, __VA_ARGS__>; \
    static const Registry& registry();

} // namespace mg
