#pragma once

#include "graphics/colors/ColorTimeline.h"
#include "graphics/TextStyle.h"

#include "utils.h"

enum TickStyle { LINE, TRIANGLE, CIRCLE };

struct TickValueStyle {
    ColorTimeline color;

    TextStyle textStyle;

    int spacing = 1;

    bool flipValuesPosition = false;

    TickValueStyle(const rapidjson::Value::ConstObject& json) {
        setValue(json, "color", color);
        setObj(json, "textStyle", textStyle);
        setInt(json, "spacing", spacing);
        setBool(json, "flipValuesPosition", flipValuesPosition);
    }
};