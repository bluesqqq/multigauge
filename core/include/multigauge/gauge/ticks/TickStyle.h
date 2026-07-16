#pragma once

#include <multigauge/graphics/colors/ColorTimeline.h>
#include <multigauge/graphics/TextPaint.h>

namespace mg::gauge {

using ::mg::graphics::ColorTimeline;
using ::mg::graphics::TextPaint;

enum TickStyle { LINE, TRIANGLE, CIRCLE };

struct TickValueStyle {
    ColorTimeline color;
    TextPaint textPaint;
    int spacing = 1;
    bool flipValuesPosition = false;
};

} // namespace mg::gauge
