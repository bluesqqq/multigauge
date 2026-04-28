#pragma once

#include <multigauge/graphics/colors/ColorTimeline.h>
#include <multigauge/graphics/TextPaint.h>

#include <multigauge/utils.h>

namespace mg::gauge {

enum TickStyle { LINE, TRIANGLE, CIRCLE };

struct TickValueStyle {
    ColorTimeline color;
    TextPaint textPaint;
    int spacing = 1;
    bool flipValuesPosition = false;
};

} // namespace mg::gauge

using TickStyle = mg::gauge::TickStyle;
using TickValueStyle = mg::gauge::TickValueStyle;
