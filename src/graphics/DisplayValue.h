#pragma once

#include "values/value.h"
#include "utils.h"

/// @brief A class that wraps around a `Value` object, allowing the use of custom minimum and maximum limits & units
/// @note This class does not have setter functions for setting the value of the `Value` reference, as it is meant to supply context to the `Value` object.
class DisplayValue {
    /*
        This class allows for `GaugeElement` objects that require `Value` references to not be limited to displaying the
        minimum and maximum values defined in the `Value` object. A unit index can be defined as well to specify which
        index to represent the value in this context.
    */

    private:
        /// @brief The base `Value` object being wrapped
        Value* value = nullptr;

        /// @brief Optional custom minimum value pointer. 
        std::optional<float> minimum = std::nullopt;

        /// @brief Optional custom maximum value pointer. 
        std::optional<float> maximum = std::nullopt;
        
        /// @brief Optional Index of the unit used for conversion or display.
        std::optional<int> unitIndex = std::nullopt;

        int getUnitIndex() const;

    public: 
        DisplayValue();
        
        DisplayValue(Value* value, std::optional<int> unitIndex = std::nullopt, std::optional<float> minimum = std::nullopt, std::optional<float> maximum = std::nullopt);

        DisplayValue(const rapidjson::Value::ConstObject json);

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

        const Unit* getUnit() const;

        std::string getValueString(bool abbreviation = false) const;

        const char* getName() const;
};