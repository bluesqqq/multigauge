#pragma once

#include <multigauge/json/Json.h>

#include <concepts>
#include <optional>
#include <type_traits>

namespace mg {

/// @brief Describes a serializable enum option.
/// @tparam T Enum type represented by this option.
template <typename T>
struct EnumOption {
    T value;
    const char* key;
    const char* label;
};

/// @brief Provides metadata describing the serializable values of an enum.
/// @tparam T Enum type being described.
template <typename T>
struct EnumTraits;

/// @brief Determines whether an enum type provides EnumTraits metadata.
template <typename T>
concept EnumDescribed = requires {
    EnumTraits<T>::options;
};

/// @brief Resolves the underlying type used for enum metadata lookup.
template <typename T>
struct EnumTraitsType {
    using type = std::remove_cv_t<std::remove_reference_t<T>>;
};

/// @brief Resolves optional enum types to their contained enum type.
template <typename T>
struct EnumTraitsType<std::optional<T>> {
    using type = typename EnumTraitsType<T>::type;
};

/// @brief Convenience alias for the normalized enum matadata type. 
template <typename T>
using EnumTraitsTypeT = typename EnumTraitsType<T>::type;


/// @brief Decodes an enum from its serialized key.
/// @tparam E Described enum type.
/// @param value JSON value containing the enum key.
/// @param out Destination receiving the decoded enum value.
/// @return true if the key matches a declared enum option; otherwise false.
template <typename E>
bool decodeEnum(json::Reader value, E& out) {
    std::string_view key;
    if (!value.read(key)) return false;

    for (const auto& option : EnumTraits<E>::options) {
        if (key == option.key) {
            out = option.value;
            return true;
        }
    }

    return false;
}

/// @brief Encodes an enum using its serialized key.
/// @tparam E Described enum type.
/// @param writer JSON writer receiving the encoded key.
/// @param value Enum value to encode.
/// @return true if the value matches a declared enum option and is written successfully; otherwise false.
template <typename E>
bool encodeEnum(json::Writer& writer, E value) {
    for (const auto& option : EnumTraits<E>::options) {
        if (option.value == value) {
            return writer.write(option.key);
        }
    }

    return false;
}

/// @brief Writes editor metadata for all declared values of an enum.
/// @tparam E Described enum type.
/// @param writer JSON writer receiving the option metadata.
/// @return true if all option metadata is written successfully; otherwise false.
template <typename E>
bool enumOptionsMeta(json::Writer& writer) {
    return writer.writeArray([&](json::ArrayWriter& options) {
        for (const auto& option : EnumTraits<E>::options) {
            if (!options.writeObject([&](json::ObjectWriter& entry) {
                return entry.write("value", option.key)
                    && entry.write("label", option.label ? option.label : option.key);
            })) {
                return false;
            }
        }

        return true;
    });
}

} // namespace mg
