#pragma once

#include <string>
#include <functional>
#include "units.h"
#include <unordered_map>

class Value {
    private:
        static std::unordered_map<std::string, Value*> registry;

        const char* id;
        const char* name;

        float value;
        const float minimumValue;
        const float maximumValue;

        const UnitType& unitType;

    public:
        Value(const char* id, const char* name, const UnitType& unitType, float minimumValue, float maximumValue);
        
        static Value* find(const std::string& id);

        std::function<void(float)> onChange;

        operator float() const;
        Value& operator=(float newValue);

        float getValueBase() const;
        void  setValueBase(float newValue);
        float getMinimumBase() const;
        float getMaximumBase() const;

        /// @brief Gets the value in the specified unit.
        /// @param index The index of the unit in the associated UnitType.
        /// @return The value converted to the specified unit.
        float getValue(int index = DEFAULT_UNIT) const;

        /// @brief Sets the value in the specified unit.
        /// @param newValue The value (in the source unit) to set.
        /// @param index The index of the source unit.
        void setValue(float newValue, int index = DEFAULT_UNIT);

        /// @brief Gets the minimum value in the specified unit
        /// @param index The index of the specified unit.
        /// @return The minimum value converted to the specified unit.
        float getMinimum(int index = DEFAULT_UNIT) const;

        /// @brief Gets the maximum value in the specified unit
        /// @param index The index of the specified unit.
        /// @return The maximum value converted to the specified unit.
        float getMaximum(int index = DEFAULT_UNIT) const;

        const char* getId() const;
        const char* getName() const;

        const UnitType& getUnitType() const;

        /// @brief Returns the normalized position of the current value between its minimum and maximum.
        /// @return A float between 0.0 and 1.0 representing the value's relative position: 0.0 corresponds to the minimum, 1.0 corresponds to the maximum.
        float getInterpolationValue() const;

        /// @brief Returns a string representation of the value for a specific unit.
        /// @param index The index of the unit.
        /// @param abbreviation If true, appends the unit abbreviation to the string.
        /// @return The value, in the specified unit, as a string.
        std::string getValueString(int index = DEFAULT_UNIT, bool abbreviation = true) const;
        
        /// @brief Returns the longest string representation of the value's range (minimum or maximum) for a specific unit
        /// @param index The index of the unit.
        /// @param abbreviation If true, appends the unit abbreviation to the string.
        /// @return The longer of the minimum or maximum value, in the specified unit, as a string.
        std::string getLongestValueString(int index = DEFAULT_UNIT, bool abbreviation = true);
};

extern Value dummy;
extern Value engineRPM;
extern Value engineCoolantTemp;
extern Value calculatedEngineLoad;
extern Value throttlePosition;
extern Value engineRunTimeSinceStart;
extern Value engineFuelRate;
extern Value engineOilTemp;
extern Value transmissionTemp;
extern Value engineOilPressure;
extern Value transmissionFluidPressure;
extern Value fuelPressure;
extern Value boostPressure;
extern Value fuelLevel;
extern Value distanceDriven;
extern Value speed;
extern Value verticalAcceleration;
extern Value longitudinalAcceleration;
extern Value lateralAcceleration;