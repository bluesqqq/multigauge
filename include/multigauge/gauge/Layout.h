#pragma once

#include <multigauge/gauge/layout/ChildAlignment.h>
#include <multigauge/gauge/layout/Direction.h>
#include <multigauge/gauge/layout/Floating.h>
#include <multigauge/gauge/layout/Padding.h>
#include <multigauge/gauge/layout/Size.h>
#include <multigauge/Config.h>
#include <multigauge/properties/PropertyObject.h>
#if MG_BUILD_EDITOR
#include <multigauge/properties/meta/Inspector.h>
#endif

namespace mg::gauge::layout {

/// @brief Stores Clay-native layout state for an element or face.
struct Layout : ::mg::PropertyObject {
    Size width;
    Size height;
    Direction direction = Direction::Vertical;
    Padding padding;
    int childGap = 0;
    ChildAlignment childAlignment;
    Floating floating;
    float aspectRatio = 0.0F;

    MG_PROPS_BEGIN()
    MG_PROP(width, "width", "Width", "Clay width sizing.")
    MG_PROP(height, "height", "Height", "Clay height sizing.")
    MG_PROP(direction, "direction", "Direction", "Child layout direction.")
    MG_PROP(padding, "padding", "Padding", "Padding around children.")
    MG_PROP(childGap, "childGap", "Child Gap", "Space between children.")
    MG_PROP(childAlignment, "childAlignment", "Child Alignment", "Cross-axis alignment for children.")
    MG_PROP(floating, "floating", "Floating", "Floating-layer placement.")
    MG_PROP(aspectRatio, "aspectRatio", "Aspect Ratio", "Target width divided by height; zero disables it.")
    MG_PROPS_END()

#if MG_BUILD_EDITOR
    MG_INSPECTOR_BEGIN()
    MG_SECTION("Size", {
        MG_ROW({
            MG_PROPERTY("width");
            MG_PROPERTY("height");
        });
        MG_PROPERTY("aspectRatio");
    });
    MG_SECTION("Layout", {
        MG_CONTROL("direction-toggle", {MG_BIND("value", "direction")});
        MG_ROW({
            MG_CONTROL("alignment-grid", {MG_BIND("value", "childAlignment")});
            MG_PROPERTY("childGap");
        });
        MG_CONTROL("insets", {MG_BIND("value", "padding")});
    });
    MG_SECTION("Position", {
        MG_PROPERTY("floating.mode");
        MG_CONTROL_IF("anchor-pair", MG_IN("floating.mode", "relative", "absolute"),
                      {MG_BIND("target", "floating.parentAnchor"),
                       MG_BIND("element", "floating.elementAnchor")});
        MG_CONTROL_IF("axis-pair", MG_IN("floating.mode", "relative", "absolute"),
                      {MG_BIND("value", "floating.offset")});
        MG_PROPERTY_IF("floating.zIndex", MG_IN("floating.mode", "relative", "absolute"));
        MG_PROPERTY_IF("floating.expand", MG_IN("floating.mode", "relative", "absolute"));
    });
    MG_INSPECTOR_END()
#endif
};

} // namespace mg::gauge::layout
