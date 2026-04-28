#pragma once

#include <multigauge/properties/PropertyObject.h>
#include <multigauge/values/ValueRef.h>

namespace mg::graphics {

/// @brief A class that wraps around a `Value` object, allowing the use of custom minimum and maximum limits & units
/// @note This class does not have setter functions for setting the value of the `Value` reference, as it is meant to supply context to the `Value` object.
class DisplayValue : public ::mg::PropertyObject {
    CODEC_FRIEND(DisplayValue)
    MG_EDITOR_NAME("Value")

    /*
        This class allows for `GaugeElement` objects that require `Value` references to not be limited to displaying the
        minimum and maximum values defined in the `Value` object. A unit index can be defined as well to specify which
        index to represent the value in this context.
    */

    private:
        /// @brief The base `Value` object being wrapped
        ::mg::ValueRef value;

        /// @brief Optional custom minimum value pointer. 
        std::optional<float> minimum = std::nullopt;

        /// @brief Optional custom maximum value pointer. 
        std::optional<float> maximum = std::nullopt;
        
        /// @brief Optional Index of the unit used for conversion or display.
        std::optional<int> unitIndex = std::nullopt;

        int getUnitIndex() const;

    public: 
        DisplayValue();
        
        DisplayValue(const char* id);

        /// @brief Retrieves the raw value stored in the 'Value' reference without any conversion, clamped to the `GaugeValue`s custom limits if defined.
        /// @return The raw value from `value.getValueRaw()`, with additional custom limits if defined.
        float getValueBase() const;

        /// @brief Retrieves the value stored in the 'Value' reference using `unitIndex` as the index, clamped to the `GaugeValue`s custom limits if defined.
        /// @return The converted value from `value.getValueRaw()`, with additional custom limits if defined
        float getValue() const;

        float getInterpolationValue() const;

        float getMinimumBase() const;
        float getMaximumBase() const;
        float getMinimum() const;
        float getMaximum() const;

        const ::mg::Unit* getUnit() const;

        std::string getValueString(bool abbreviation = false) const;

        const char* getName() const;

        MG_PROPS_BEGIN()
    MG_PROP(value, "id", "ID", "Value ID.")
    MG_PROP(minimum, "min", "Minimum", "Minimum value. Make null to use default minimum.")
    MG_PROP(maximum, "max", "Maximum", "Maximum value. Make null to use default maximum.")
    MG_PROP(unitIndex, "unitIndex", "Unit Index", "Unit Index to display. Make null to use default index.")
        MG_PROPS_END()
};

} // namespace mg::graphics

namespace mg {

CODEC_BEGIN(graphics::DisplayValue)
    DECODE() {
        if (v.IsString()) {
            out = CodecType(v.GetString());
            return true;
        }
        return false;
    }

    ENCODE() {
        // we only encode as a plain id string when no optional fields are set
        if (v.minimum.has_value() || v.maximum.has_value() || v.unitIndex.has_value()) return false;
        
        if (v.value) {
            out.SetString(v.value.getId().c_str(), static_cast<rapidjson::SizeType>(v.value.getId().size()), a);
            return true;
        } else {
            out.SetNull();
            return true;
        }
    }
CODEC_END()

} // namespace mg
