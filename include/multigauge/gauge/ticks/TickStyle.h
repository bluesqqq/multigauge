#pragma once

#include <multigauge/graphics/colors/ColorTimeline.h>
#include <multigauge/graphics/TextPaint.h>

#include <multigauge/utils.h>

enum TickStyle { LINE, TRIANGLE, CIRCLE };

struct TickValueStyle {
    ColorTimeline color;
    TextPaint textPaint;
    int spacing = 1;
    bool flipValuesPosition = false;
};