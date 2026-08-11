#pragma once

#include <string>

#include <multigauge/gauge/Element.h>

namespace mg::gauge {

/// @brief Represents an element with a runtime-owned custom type identifier.
class CustomElement final : public Element {
public:
    /// @brief Creates a custom element preserving its serialized type identifier.
    explicit CustomElement(std::string typeId) noexcept;

    /// @brief Returns the custom serialized type identifier.
    [[nodiscard]] const char* typeId() const override;

private:
    std::string typeId_;
};

} // namespace mg::gauge
