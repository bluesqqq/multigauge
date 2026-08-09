#pragma once

#include <multigauge/json/Json.h>

#include <cstddef>
#include <span>
#include <string_view>

namespace mg {

/// @brief Describes a concrete type available through a polymorphic registry.
/// @tparam Owned Owning type returned when creating an instance.
/// @tparam Args Arguments forwarded to the creation function.
template <typename Owned, typename... Args>
struct MgPolymorphicTypeDescriptor {
    const char* id;             ///< Stable type identifier used for serialization and lookup.
    const char* name;           ///< Human-readable type name shown in the editor.
    Owned (*create)(Args...);   ///< Factory function used to create an instance.
};

/// @brief Creates a polymorphic type descriptor for a derived type.
/// @tparam Derived Concrete type being registered.
/// @tparam Owned Owning type returned by the factory.
/// @tparam Args Arguments accepted by the factory.
/// @param create Factory function used to create the derived type.
/// @return Descriptor containing the derived type metadata and factory.
template <typename Derived, typename Owned, typename... Args>
constexpr MgPolymorphicTypeDescriptor<Owned, Args...>
makePolymorphicTypeDescriptor(Owned (*create)(Args...)) {
    return {
        Derived::staticTypeId(),
        Derived::staticTypeName(),
        create
    };
}

/// @brief Registry of concrete types available for a polymorphic base type.
/// @tparam Owned Owning type returned when creating registered instances.
/// @tparam Args Arguments forwarded to registered factory functions.
template <typename Owned, typename... Args>
class MgPolymorphicRegistry {
public:
    using Descriptor = MgPolymorphicTypeDescriptor<Owned, Args...>;
    using Creator = Owned (*)(Args...);

    /// @brief Constructs a registry over an existing descriptor array.
    /// @param descriptors Registered type descriptors.
    /// @param fallback Factory used when no matching registered type is found.
    constexpr MgPolymorphicRegistry(std::span<const Descriptor> descriptors, Creator fallback)
        : descriptors_(descriptors),
          fallback_(fallback) {}

    /// @brief Finds a registered type by identifier.
    /// @param type Type identifier to find.
    /// @return Matching descriptor, or nullptr if no type matches.
    [[nodiscard]] constexpr const Descriptor* find(std::string_view type) const noexcept {
        if (type.empty()) return nullptr;

        for (const Descriptor& descriptor : descriptors_) {
            if (descriptor.id && type == descriptor.id) {
                return &descriptor;
            }
        }

        return nullptr;
    }

    /// @brief Creates an instance of a registered type.
    /// @param type Type identifier to create.
    /// @param args Arguments forwarded to the selected factory.
    /// @return Created instance, the fallback instance if configured, or an empty owner.
    [[nodiscard]] Owned create(std::string_view type, Args... args) const {
        if (const Descriptor* descriptor = find(type);
            descriptor && descriptor->create) {
            return descriptor->create(args...);
        }

        return fallback_ ? fallback_(args...) : Owned{};
    }

    [[nodiscard]] constexpr const Descriptor* begin() const noexcept { return descriptors_.data(); }
    [[nodiscard]] constexpr const Descriptor* end() const noexcept { return descriptors_.data() + descriptors_.size(); }
    [[nodiscard]] constexpr std::size_t size() const noexcept { return descriptors_.size(); }

    /// @brief Writes editor metadata for all registered polymorphic types.
    /// @param writer JSON writer receiving the type metadata.
    /// @return true if all metadata is written successfully; otherwise false.
    [[nodiscard]] bool writeTypesMeta(json::Writer& writer) const {
        return writer.writeArray([&](json::ArrayWriter& types) {
            for (const Descriptor& descriptor : descriptors_) {

                if (!types.writeObject([&](json::ObjectWriter& entry) {
                    return entry.write("id", descriptor.id ? descriptor.id : "")
                        && entry.write("name", descriptor.name ? descriptor.name : "");
                })) {
                    return false;
                }
            }

            return true;
        });
    }

private:
    std::span<const Descriptor> descriptors_;
    Creator fallback_ = nullptr;
};

/// @brief Provides polymorphic registry metadata for a type.
/// @tparam T Type whose registry support is being queried.
///
/// Specialize this trait for types that expose polymorphic type metadata.
template <typename T, typename = void>
struct MgPolymorphicRegistryTraits {
    static constexpr bool supported = false;

    static bool getTypesMeta(json::Writer& writer) {
        return writer.writeArray([](json::ArrayWriter&) {
            return true;
        });
    }
};

/// @brief Declares a polymorphic registry with no factory arguments.
#define MG_POLYMORPHIC_REGISTRY(owned_type) \
    using Registry = MgPolymorphicRegistry<owned_type>; \
    static const Registry& registry();

/// @brief Declares a polymorphic registry whose factories accept additional arguments.
#define MG_POLYMORPHIC_REGISTRY_WITH_ARGS(owned_type, ...) \
    using Registry = MgPolymorphicRegistry<owned_type, __VA_ARGS__>; \
    static const Registry& registry();

} // namespace mg
