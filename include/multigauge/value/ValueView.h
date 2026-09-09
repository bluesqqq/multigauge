#pragma once

#include <multigauge/properties/PropertyObject.h>
#include <multigauge/properties/meta/Inspector.h>
#include <multigauge/value/ValueRef.h>

#include <optional>
#include <string>

namespace mg {

/// @brief A class that wraps around a `Value` object, allowing the use of custom minimum and maximum limits & units
/// @note This class does not have setter functions for setting the value of the `Value` reference, as it is meant to supply context to the `Value` object.
class ValueView : public ::mg::PropertyObject {
    CODEC_FRIEND(ValueView)
    MG_EDITOR_NAME("Value")

/*
    This class allows for `GaugeElement` objects that require `Value` references to not be limited to displaying the
    minimum and maximum values defined in the `Value` object. A unit index can be defined as well to specify which
    index to represent the value in this context.
*/

public:
    /* ----- CONSTRUCTORS ----- */

    ValueView() noexcept;

    explicit ValueView(std::string_view id);

    /* ----- VALUE ----- */

    /// @brief Retrieves the raw value stored in the 'Value' reference without any conversion, clamped to the `GaugeValue`s custom limits if defined.
    /// @return The raw value from `value.getValueRaw()`, with additional custom limits if defined.
    Measurement valueBase() const noexcept;

    /// @brief Retrieves the value stored in the 'Value' reference using `unitIndex` as the index, clamped to the `GaugeValue`s custom limits if defined.
    /// @return The converted value from `value.getValueRaw()`, with additional custom limits if defined
    Measurement value() const noexcept;

    /* ----- RANGE ----- */

    Measurement minimumBase() const noexcept;

    Measurement maximumBase() const noexcept;

    Measurement minimum() const noexcept;

    Measurement maximum() const noexcept;

    Measurement interpolationValue() const noexcept;

    /* ----- METADATA ----- */

    std::string_view name() const noexcept;

    const ::mg::Unit* unit() const noexcept;

    /* ----- FORMATTING ----- */

    std::string valueString(bool includeAbbreviation = false) const;

private:
    ::mg::ValueRef value_; ///< The base `Value` object being wrapped
    std::optional<Measurement> minimumBase_ = std::nullopt; ///< Optional custom minimum value in base units.
    std::optional<Measurement> maximumBase_ = std::nullopt; ///< Optional custom maximum value in base units.
    std::optional<UnitIndex> unitIndex_ = std::nullopt; ///< Optional custom unit index.

    UnitIndex getUnitIndex() const noexcept;

    MG_PROPS_BEGIN()
        MG_PROP(value_, "id")
        MG_PROP(minimumBase_, "min")
        MG_PROP(maximumBase_, "max")
    MG_PROP(unitIndex_, "unitIndex")
    MG_PROPS_END()

#if MG_BUILD_EDITOR
    MG_INSPECTOR_BEGIN()
    MG_SECTION("Value", {
        MG_PROPERTY("id", "ID", "text");
        MG_ROW({
            MG_PROPERTY("min", "Minimum", "number");
            MG_PROPERTY("max", "Maximum", "number");
        });
        MG_PROPERTY("unitIndex", "Unit Index", "number");
    });
    MG_INSPECTOR_END()
#endif
};

CODEC_BEGIN(ValueView)
    DECODE() {
        std::string_view id;

        if (v.read(id)) {
            out = ValueView(id);
            return static_cast<bool>(out.value_);
        }

        return false;
    }

    ENCODE() {
        // we only encode as a plain id string when no optional fields are set
        if (v.minimumBase_.has_value() || v.maximumBase_.has_value() || v.unitIndex_.has_value()) return false;

        return !v.value_.id().empty() ? out.write(v.value_.id()) : out.null();
    }
CODEC_END()

} // namespace mg
