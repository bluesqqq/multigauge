#pragma once

#include <multigauge/editor/Editable.h>
#include <yoga/Yoga.h>

struct LayoutSize {
    enum class Unit { Px, Percent, Auto } unit = Unit::Px;
	float value = 0.0f;

    void setMargin(YGNodeRef node, YGEdge edge) {
        if (!node) return;

        switch (unit) {
            case Unit::Px:
                YGNodeStyleSetMargin(node, edge, value);
            case Unit::Percent:
                YGNodeStyleSetMarginPercent(node, edge, value);
            case Unit::Auto:
                YGNodeStyleSetMarginAuto(node, edge);
        }
    }

    void setPadding(YGNodeRef node, YGEdge edge) {
        if (!node) return;

        switch (unit) {
            case Unit::Px:
                YGNodeStyleSetPadding(node, edge, value);
            case Unit::Percent:
                YGNodeStyleSetPaddingPercent(node, edge, value);
            case Unit::Auto:
                YGNodeStyleSetPadding(node, edge, 0.0f); // TODO: maybe not handle it this way...
        }
    }
};

struct Flex : public Editable{
    YGNodeRef node = nullptr;

    YGFlexDirection direction;
    YGJustify justify;
    YGAlign align;
    YGWrap wrap;

    float grow = 1;
    float shrink = 1;

    void updateDirection() { if (node) YGNodeStyleSetFlexDirection(node, direction); }
    void updateJustify()   { if (node) YGNodeStyleSetJustifyContent(node, justify); }
    void updateAlign()     { if (node) YGNodeStyleSetAlignItems(node, align); }
    void updateWrap()      { if (node) YGNodeStyleSetFlexWrap(node, wrap); }

    void updateGrow()   { if (node) YGNodeStyleSetFlexGrow(node, grow); }
    void updateShrink() { if (node) YGNodeStyleSetFlexShrink(node, shrink); }

    MG_EDITOR_BEGIN()
        MG_EDITOR_PROP_CALLBACK(direction, "direction", updateDirection, "Direction", "Direction of flex content.")
        MG_EDITOR_PROP_CALLBACK(justify,   "justify",   updateJustify,   "Justify",   "Main-axis alignment of children.")
        MG_EDITOR_PROP_CALLBACK(align,     "align",     updateAlign,     "Align",     "Cross-axis alignment of children.")
        MG_EDITOR_PROP_CALLBACK(wrap,      "wrap",      updateWrap,      "Wrap",      "Wrap setting for children.")

        MG_EDITOR_PROP_CALLBACK(grow,   "grow",   updateGrow,   "Grow",   "Rate element expands to fill avaiable space.")
        MG_EDITOR_PROP_CALLBACK(shrink, "shrink", updateShrink, "Shrink", "Rate element contracts to avoid overflow.")
    MG_EDITOR_END()
};

struct PositionEdge {
    
    enum class Unit { Px, Percent } unit = Unit::Px;
	float value = 0.0f;

    void setEdge(YGNodeRef node, YGEdge edge) {
        if (!node) return;
        if (unit == Unit::Px) YGNodeStyleSetPosition(node, edge, value);
        else if (unit == Unit::Percent) YGNodeStyleSetPositionPercent(node, edge, value);
    }
};

CODEC_BEGIN(PositionEdge)
    DECODE() {
        if (v.IsNumber()) {
            out.value = v.GetFloat();
            out.unit = PositionEdge::Unit::Px;
            return true;
        }

        if (v.IsString()) {
            // TODO: finish
            return true;
        }

        return false;
    }

    ENCODE() {
        if (v.unit == PositionEdge::Unit::Px) {
            out.SetFloat(v.value);
            return true;
        } else if (v.unit == PositionEdge::Unit::Percent) {
            out.SetString("%"); // TODO: finish
            return true;
        }

        return false;
    }
CODEC_END()

struct Position : public Editable {
    YGNodeRef node = nullptr;
    
    YGPositionType type;

    PositionEdge left;
    PositionEdge right;
    PositionEdge top;
    PositionEdge bottom;

    void updateType() { if (node) YGNodeStyleSetPositionType(node, type); }

    void updateLeft()   { left.setEdge(node, YGEdgeLeft); }
    void updateRight()  { right.setEdge(node, YGEdgeRight); }
    void updateTop()    { top.setEdge(node, YGEdgeTop); }
    void updateBottom() { bottom.setEdge(node, YGEdgeBottom); }

    MG_EDITOR_BEGIN()
        MG_EDITOR_PROP_CALLBACK(type, "type", updateType, "Type", "Positioning type.")

        MG_EDITOR_PROP_CALLBACK(left,   "left",   updateLeft,   "Left",   "Left edge position.")
        MG_EDITOR_PROP_CALLBACK(right,  "right",  updateRight,  "Right",  "Right edge position.")
        MG_EDITOR_PROP_CALLBACK(top,    "top",    updateTop,    "Top",    "Top edge position.")
        MG_EDITOR_PROP_CALLBACK(bottom, "bottom", updateBottom, "Bottom", "Bottom edge position.")
    MG_EDITOR_END()
};

struct Margin : public Editable {
    YGNodeRef node = nullptr;

    LayoutSize left;
    LayoutSize right;
    LayoutSize top;
    LayoutSize bottom;

    void updateLeft()   { left.setMargin(node, YGEdgeLeft); }
    void updateRight()  { right.setMargin(node, YGEdgeRight); }
    void updateTop()    { top.setMargin(node, YGEdgeTop); }
    void updateBottom() { bottom.setMargin(node, YGEdgeBottom); }

    MG_EDITOR_BEGIN()

        MG_EDITOR_PROP_CALLBACK(left,   "left",   updateLeft,   "Left",   "Left edge position.")
        MG_EDITOR_PROP_CALLBACK(right,  "right",  updateRight,  "Right",  "Right edge position.")
        MG_EDITOR_PROP_CALLBACK(top,    "top",    updateTop,    "Top",    "Top edge position.")
        MG_EDITOR_PROP_CALLBACK(bottom, "bottom", updateBottom, "Bottom", "Bottom edge position.")
    MG_EDITOR_END()
};

struct Padding : public Editable {
    YGNodeRef node = nullptr;

    LayoutSize left;
    LayoutSize right;
    LayoutSize top;
    LayoutSize bottom;

    void updateLeft()   { left.setPadding(node, YGEdgeLeft); }
    void updateRight()  { right.setPadding(node, YGEdgeRight); }
    void updateTop()    { top.setPadding(node, YGEdgeTop); }
    void updateBottom() { bottom.setPadding(node, YGEdgeBottom); }

    MG_EDITOR_BEGIN()

        MG_EDITOR_PROP_CALLBACK(left,   "left",   updateLeft,   "Left",   "Left edge position.")
        MG_EDITOR_PROP_CALLBACK(right,  "right",  updateRight,  "Right",  "Right edge position.")
        MG_EDITOR_PROP_CALLBACK(top,    "top",    updateTop,    "Top",    "Top edge position.")
        MG_EDITOR_PROP_CALLBACK(bottom, "bottom", updateBottom, "Bottom", "Bottom edge position.")
    MG_EDITOR_END()
};

struct Layout : public Editable {
    YGNodeRef node = nullptr;

    Flex flex;
    Position position;
    Margin margin;
    Padding padding;

    YGDisplay display;
    YGOverflow overflow;

    float aspectRatio = 1;

    void updateOverflow()    { if (node) YGNodeStyleSetOverflow(node, overflow); }
    void updateDisplay()     { if (node) YGNodeStyleSetDisplay(node, display); }
    void updateAspectRatio() { if (node) YGNodeStyleSetAspectRatio(node, aspectRatio); }

    MG_EDITOR_BEGIN()
        MG_EDITOR_PROP(flex, "flex", "Flex", "Flex options.")
        MG_EDITOR_PROP(position, "position", "Position", "Position options.")

        MG_EDITOR_PROP(margin, "margin", "Margin", "Margin options.")
        MG_EDITOR_PROP(padding, "padding", "Padding", "Padding options.")

        MG_EDITOR_PROP_CALLBACK(overflow, "overflow", updateOverflow, "Overflow", "Overflow setting for content.")
        MG_EDITOR_PROP_CALLBACK(display, "display", updateDisplay, "Display", "Display setting.")

        MG_EDITOR_PROP_CALLBACK(aspectRatio, "aspect-ratio", updateAspectRatio, "Aspect Ratio", "Aspect ratio of the element.")
    MG_EDITOR_END()
};

CODEC_BEGIN(YGFlexDirection)
    DECODE() {
        if (!v.IsString()) return false;

        const auto s = v.GetString();

        if (!s) return false;

        if (strcmp(s, "row") == 0) return YGFlexDirectionRow;
        if (strcmp(s, "column") == 0) return YGFlexDirectionColumn;
        if (strcmp(s, "row-reverse") == 0) return YGFlexDirectionRowReverse;
        if (strcmp(s, "column-reverse") == 0) return YGFlexDirectionColumnReverse;

        return false;
    }

    ENCODE() {
        out.SetString(YGFlexDirectionToString(v), a);
    }
CODEC_END()


CODEC_BEGIN(YGJustify)
    DECODE() {
        if (!v.IsString()) return false;

        const auto s = v.GetString();

        if (!s) return false;

        if (strcmp(s, "flex-start") == 0) return YGJustifyFlexStart;
        if (strcmp(s, "center") == 0) return YGJustifyCenter;
        if (strcmp(s, "flex-end") == 0) return YGJustifyFlexEnd;
        if (strcmp(s, "space-between") == 0) return YGJustifySpaceBetween;
        if (strcmp(s, "space-around") == 0) return YGJustifySpaceAround;
        if (strcmp(s, "space-evenly") == 0) return YGJustifySpaceEvenly;

        return false;
    }

    ENCODE() {
        out.SetString(YGJustifyToString(v), a);
    }
CODEC_END()

CODEC_BEGIN(YGAlign)
    DECODE() {
        if (!v.IsString()) return false;

        const auto s = v.GetString();

        if (!s) return false;

        if (strcmp(s, "auto") == 0) return YGAlignAuto;
        if (strcmp(s, "flex-start") == 0) return YGAlignFlexStart;
        if (strcmp(s, "center") == 0) return YGAlignCenter;
        if (strcmp(s, "flex-end") == 0) return YGAlignFlexEnd;
        if (strcmp(s, "stretch") == 0) return YGAlignStretch;
        if (strcmp(s, "baseline") == 0) return YGAlignBaseline;
        if (strcmp(s, "space-between") == 0) return YGAlignSpaceBetween;
        if (strcmp(s, "space-around") == 0) return YGAlignSpaceAround;

        return false;
    }

    ENCODE() {
        out.SetString(YGAlignToString(v), a);
    }
CODEC_END()


CODEC_BEGIN(YGWrap)
    DECODE() {
        if (!v.IsString()) return false;

        const auto s = v.GetString();

        if (!s) return false;

        if (strcmp(s, "no-wrap") == 0) return YGWrapNoWrap;
        if (strcmp(s, "wrap") == 0) return YGWrapWrap;
        if (strcmp(s, "wrap-reverse") == 0) return YGWrapWrapReverse;

        return false;
    }

    ENCODE() {
        out.SetString(YGWrapToString(v), a);
    }
CODEC_END()

CODEC_BEGIN(YGPositionType)
    DECODE() {
        if (!v.IsString()) return false;

        const auto s = v.GetString();

        if (!s) return false;

        if (strcmp(s, "static") == 0) return YGPositionTypeStatic;
        if (strcmp(s, "relative") == 0) return YGPositionTypeRelative;
        if (strcmp(s, "absolute") == 0) return YGPositionTypeAbsolute;
        
        return false;
    }

    ENCODE() {
        out.SetString(YGPositionTypeToString(v), a);
    }
CODEC_END()

CODEC_BEGIN(YGDisplay)
    DECODE() {
        if (!v.IsString()) return false;

        const auto s = v.GetString();

        if (!s) return false;

        if (strcmp(s, "none") == 0) return YGDisplayNone;
        if (strcmp(s, "flex") == 0) return YGDisplayFlex;
        
        return false;
    }

    ENCODE() {
        out.SetString(YGDisplayToString(v), a);
    }
CODEC_END()

CODEC_BEGIN(YGOverflow)
    DECODE() {
        if (!v.IsString()) return false;

        const auto s = v.GetString();

        if (!s) return false;

        if (strcmp(s, "visible") == 0) return YGOverflowVisible;
        if (strcmp(s, "hidden") == 0) return YGOverflowHidden;
        if (strcmp(s, "scroll") == 0) return YGOverflowScroll;

        return false;
    }

    ENCODE() {
        out.SetString(YGOverflowToString(v), a);
    }
CODEC_END()
