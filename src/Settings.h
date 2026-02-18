#pragma once

#include "rapidjson/document.h"
#include "utils.h"
#include "values/units.h"
#include "io/logging/Logger.h"

class Settings {
    private:
        int brightness = 255;
        int rotation   = 0;

        bool setDefaultUnit(UnitType& type, const rapidjson::Value::ConstObject& units, const char* key) {
            auto it = units.FindMember(key);
            if (it == units.MemberEnd()) {
                LOG_WARN("settings.json", "units.%s not present", key);
                return false;
            }

            const auto& v = it->value;

            if (v.IsInt()) {
                type.setDefaultUnit(v.GetInt());
                LOG_INFO("settings.json", "units.%s set by index: %d", key, idx);
                return true;
            }

            if (v.IsString()) {
                const char* s = v.GetString();

                int idx = type.getIndexFromName(s);
                if (idx == DEFAULT_UNIT) {
                    LOG_INFO("settings.json", "units.%s set by index: %d", key, idx);
                    idx = type.getIndexFromAbbreviation(s);
                    return true;
                }

                if (idx != DEFAULT_UNIT) {
                    type.setDefaultUnit(idx);
                    LOG_INFO("settings.json", "units.%s set by abbreviation: '%s' -> %d", key, s, idx);
                    return true;
                }

                LOG_WARN("settings.json", "units.%s invalid string '%s' (no match by name or abbreviation)", key, s);
                return false;
            }

            LOG_WARN("settings.json", "units.%s wrong type (expected int or string)", key);
            return false;
        }

    public:
        Settings();
        
        Settings(const rapidjson::Document& json);

        void loadSettings(const rapidjson::Document& json) {
            if (!json.IsObject()) return;
            const auto root = json.GetObject();

            setInt(root, "brightness", brightness);
            setInt(root, "rotation", rotation);

            if (json.HasMember("userColors") && json["userColors"].IsObject()) {
            }

            if (json.HasMember("units") && json["units"].IsObject()) {
                const auto units = json["units"].GetObject();

                setDefaultUnit(temperature,   units, "temperature");
                setDefaultUnit(distance,      units, "distance");
                setDefaultUnit(pressure,      units, "pressure");
                setDefaultUnit(velocity,      units, "velocity");
                setDefaultUnit(acceleration,  units, "acceleration");
                setDefaultUnit(volume,        units, "volume");
                setDefaultUnit(volumePerTime, units, "volumePerTime");
            }
        }
}