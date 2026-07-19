#pragma once

#include <multigauge/properties/PropertyObject.h>
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
    ValueView();

    ValueView(const char* id);

    /// @brief Retrieves the raw value stored in the 'Value' reference without any conversion, clamped to the `GaugeValue`s custom limits if defined.
    /// @return The raw value from `value.getValueRaw()`, with additional custom limits if defined.
    Measurement valueBase() const;

    /// @brief Retrieves the value stored in the 'Value' reference using `unitIndex` as the index, clamped to the `GaugeValue`s custom limits if defined.
    /// @return The converted value from `value.getValueRaw()`, with additional custom limits if defined
    Measurement value() const;

    Measurement interpolatedValue() const;

    Measurement minimumBase() const;
    Measurement maximumBase() const;
    Measurement minimum() const;
    Measurement maximum() const;

    const ::mg::Unit* unit() const;

    std::string formatString(bool includeAbbreviation = false) const;

    const char* name() const;

private:
    ::mg::ValueRef value_; ///< The base `Value` object being wrapped
    std::optional<Measurement> minimum_ = std::nullopt; ///< Optional custom minimum value.
    std::optional<Measurement> maximum_ = std::nullopt; ///< Optional custom maximum value.
    std::optional<UnitIndex> unitIndex_ = std::nullopt; ///< Optional custom unit index.

    UnitIndex getUnitIndex() const;

    MG_PROPS_BEGIN()
        MG_PROP(value_, "id", "ID", "Value ID.")
        MG_PROP(minimum_, "min", "Minimum", "Minimum value. Make null to use default minimum.")
        MG_PROP(maximum_, "max", "Maximum", "Maximum value. Make null to use default maximum.")
        MG_PROP(unitIndex_, "unitIndex", "Unit Index", "Unit Index to display. Make null to use default index.")
    MG_PROPS_END()
};

} // namespace mg

namespace mg {

CODEC_BEGIN(ValueView)
    DECODE() {
        if (v.IsString()) {
            out = CodecType(v.GetString());
            return true;
        }
        return false;
    }

    ENCODE() {
        // we only encode as a plain id string when no optional fields are set
        if (v.minimum_.has_value() || v.maximum_.has_value() || v.unitIndex_.has_value()) return false;

        if (v.value_) {
            out.SetString(
                v.value_.getId().c_str(),
                static_cast<rapidjson::SizeType>(v.value_.getId().size()),
                a
            );
            return true;
        } else {
            out.SetNull();
            return true;
        }
    }
CODEC_END()

} // namespace mg
