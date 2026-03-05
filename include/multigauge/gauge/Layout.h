#pragma once

#include <multigauge/editor/Editable.h>
#include <yoga/Yoga.h>

struct LayoutSize {
    enum class Unit { Px, Percent, Auto } unit = Unit::Auto;
	float value = 0.0f;

    void setPosition(YGNodeRef node, YGEdge edge) {
        switch (unit) {
            case Unit::Px: YGNodeStyleSetPosition(node, edge, value); break;
            case Unit::Percent: YGNodeStyleSetPositionPercent(node, edge, value); break;
            case Unit::Auto: YGNodeStyleSetPosition(node, edge, YGUndefined); break;
        }
    }

    void setMargin(YGNodeRef node, YGEdge edge) {
        switch (unit) {
            case Unit::Px: YGNodeStyleSetMargin(node, edge, value); break;
            case Unit::Percent: YGNodeStyleSetMarginPercent(node, edge, value); break;
            case Unit::Auto: YGNodeStyleSetMarginAuto(node, edge); break;
        }
    }

    void setPadding(YGNodeRef node, YGEdge edge) {
        switch (unit) {
            case Unit::Px: YGNodeStyleSetPadding(node, edge, value); break;
            case Unit::Percent: YGNodeStyleSetPaddingPercent(node, edge, value); break;
            case Unit::Auto: YGNodeStyleSetPadding(node, edge, 0.0f); break;
        }
    }

    void setWidth(YGNodeRef node) {
        switch (unit) {
            case Unit::Px: YGNodeStyleSetWidth(node, value); break;
            case Unit::Percent: YGNodeStyleSetWidthPercent(node, value); break;
            case Unit::Auto: YGNodeStyleSetWidthAuto(node); break;
        }
    }

    void setHeight(YGNodeRef node) {
        switch (unit) {
            case Unit::Px: YGNodeStyleSetHeight(node, value); break;
            case Unit::Percent: YGNodeStyleSetHeightPercent(node, value); break;
            case Unit::Auto: YGNodeStyleSetHeightAuto(node); break;
        }
    }

    void setMinWidth(YGNodeRef node) {
        switch (unit) {
            case Unit::Px: YGNodeStyleSetMinWidth(node, value); break;
            case Unit::Percent: YGNodeStyleSetMinWidthPercent(node, value); break;
            case Unit::Auto: YGNodeStyleSetMinWidth(node, YGUndefined); break;
        }
    }

    void setMinHeight(YGNodeRef node) {
        switch (unit) {
            case Unit::Px: YGNodeStyleSetMinHeight(node, value); break;
            case Unit::Percent: YGNodeStyleSetMinHeightPercent(node, value); break;
            case Unit::Auto: YGNodeStyleSetMinHeight(node, YGUndefined); break;
        }
    }

    void setMaxWidth(YGNodeRef node) {
        switch (unit) {
            case Unit::Px: YGNodeStyleSetMaxWidth(node, value); break;
            case Unit::Percent: YGNodeStyleSetMaxWidthPercent(node, value); break;
            case Unit::Auto: YGNodeStyleSetMaxWidth(node, YGUndefined); break;
        }
    }

    void setMaxHeight(YGNodeRef node) {
        switch (unit) {
            case Unit::Px: YGNodeStyleSetMaxHeight(node, value); break;
            case Unit::Percent: YGNodeStyleSetMaxHeightPercent(node, value); break;
            case Unit::Auto: YGNodeStyleSetMaxHeight(node, YGUndefined); break;
        }
    }

    void setBasis(YGNodeRef node) {
        switch (unit) {
            case Unit::Px: YGNodeStyleSetFlexBasis(node, value); break;
            case Unit::Percent: YGNodeStyleSetFlexBasisPercent(node, value); break;
            case Unit::Auto: YGNodeStyleSetFlexBasisAuto(node); break;
        }
    }
};

CODEC_BEGIN(LayoutSize)
    DECODE() {
        if (v.IsNumber()) { out = {LayoutSize::Unit::Px, v.GetFloat()}; return true; }

        if (!v.IsString()) return false;

        const auto s = v.GetString();
        if (!s) return false;

        if (strcmp(s, "auto") == 0) { out = {LayoutSize::Unit::Auto, 0}; return true; }

        char* end = nullptr;
        float num = std::strtof(s, &end);
        if (end == s) return false;

        if (*end == '\0') { out = {LayoutSize::Unit::Px, num}; return true; } 

        if (*end == '%') { out = {LayoutSize::Unit::Percent, num}; return true; }

        if ((end[0] == 'p' || end[0] == 'P') && (end[1] == 'x' || end[1] == 'X')) {
            end = const_cast<char*>(end + 2);
            if (*end != '\0') return false;

            out = {LayoutSize::Unit::Px, num};
            return true;
        }
    }

    ENCODE() {
        switch (v.unit) {
            case LayoutSize::Unit::Auto:
                out.SetString("auto", a);
                break;

            case LayoutSize::Unit::Percent: {
                char buf[32];
                std::snprintf(buf, sizeof(buf), "%%%g", v.value);
                out.SetString(buf, a);
                break;
            }

            case LayoutSize::Unit::Px: default: out.SetFloat(v.value); break;
        }

        return true;
    }
CODEC_END()

struct FlexContainer : public Editable {
    YGNodeRef node = nullptr;

    YGFlexDirection direction = YGFlexDirectionRow;
    YGJustify justify = YGJustifyFlexStart;
    YGAlign alignItems = YGAlignStretch;
    YGAlign alignContent = YGAlignStretch;
    YGWrap wrap = YGWrapNoWrap;

    void update() {
        if (!node) return;

        YGNodeStyleSetFlexDirection(node, direction);
        YGNodeStyleSetJustifyContent(node, justify);
        YGNodeStyleSetAlignItems(node, alignItems);
        YGNodeStyleSetAlignContent(node, alignContent);
        YGNodeStyleSetFlexWrap(node, wrap);
    }

    MG_EDITOR_BEGIN()
        MG_EDITOR_PROP_CALLBACK(direction,    "direction",     &FlexContainer::update, "Direction",     "Direction of flex content.")
        MG_EDITOR_PROP_CALLBACK(justify,      "justify",       &FlexContainer::update, "Justify",       "Main-axis alignment of children.")
        MG_EDITOR_PROP_CALLBACK(alignItems,   "align-items",   &FlexContainer::update, "Align Items",   "Cross-axis alignment of children.")
        MG_EDITOR_PROP_CALLBACK(alignContent, "align-content", &FlexContainer::update, "Align Content", "Cross-axis alignment of children.")
        MG_EDITOR_PROP_CALLBACK(wrap,         "wrap",          &FlexContainer::update, "Wrap",          "Wrap setting for children.")
    MG_EDITOR_END()
};

struct FlexItem : public Editable {
    YGNodeRef node = nullptr; 

    float grow = 0;
    float shrink = 1;
    LayoutSize basis;
    YGAlign alignSelf = YGAlignAuto;

    void update() {
        if (!node) return;

        YGNodeStyleSetFlexGrow(node, grow);
        YGNodeStyleSetFlexShrink(node, shrink);
        basis.setBasis(node);
        YGNodeStyleSetAlignSelf(node, alignSelf);
    }

    MG_EDITOR_BEGIN()
        MG_EDITOR_PROP_CALLBACK(grow,      "grow",       &FlexItem::update, "Grow",       "Rate element expands to fill avaiable space.")
        MG_EDITOR_PROP_CALLBACK(shrink,    "shrink",     &FlexItem::update, "Shrink",     "Rate element contracts to avoid overflow.")
        MG_EDITOR_PROP_CALLBACK(basis,     "basis",      &FlexItem::update, "Basis",      "Flex basis.")
        MG_EDITOR_PROP_CALLBACK(alignSelf, "align-self", &FlexItem::update, "Align Self", "Align self.")
    MG_EDITOR_END()
};

struct Position : public Editable {
    YGNodeRef node = nullptr;
    
    YGPositionType type = YGPositionTypeStatic;

    LayoutSize left;
    LayoutSize right;
    LayoutSize top;
    LayoutSize bottom;

    void update() {
        if (!node) return;
        
        YGNodeStyleSetPositionType(node, type);
        
        left.setPosition(node, YGEdgeLeft);
        right.setPosition(node, YGEdgeRight);
        top.setPosition(node, YGEdgeTop);
        bottom.setPosition(node, YGEdgeBottom);
    }

    MG_EDITOR_BEGIN()
        MG_EDITOR_PROP_CALLBACK(type, "type", &Position::update, "Type", "Positioning type.")

        MG_EDITOR_PROP_CALLBACK(left,   "left",   &Position::update,   "Left",   "Left edge position.")
        MG_EDITOR_PROP_CALLBACK(right,  "right",  &Position::update,  "Right",  "Right edge position.")
        MG_EDITOR_PROP_CALLBACK(top,    "top",    &Position::update,    "Top",    "Top edge position.")
        MG_EDITOR_PROP_CALLBACK(bottom, "bottom", &Position::update, "Bottom", "Bottom edge position.")
    MG_EDITOR_END()
};

struct Margin : public Editable {
    YGNodeRef node = nullptr;

    LayoutSize left{LayoutSize::Unit::Px, 0.0f};
    LayoutSize right{LayoutSize::Unit::Px, 0.0f};
    LayoutSize top{LayoutSize::Unit::Px, 0.0f};
    LayoutSize bottom{LayoutSize::Unit::Px, 0.0f};

    void update() {
        if (!node) return;

        left.setMargin(node, YGEdgeLeft);
        right.setMargin(node, YGEdgeRight);
        top.setMargin(node, YGEdgeTop);
        bottom.setMargin(node, YGEdgeBottom);
    }

    MG_EDITOR_BEGIN()
        MG_EDITOR_PROP_CALLBACK(left,   "left",   &Margin::update,   "Left",   "Left edge position.")
        MG_EDITOR_PROP_CALLBACK(right,  "right",  &Margin::update,  "Right",  "Right edge position.")
        MG_EDITOR_PROP_CALLBACK(top,    "top",    &Margin::update,    "Top",    "Top edge position.")
        MG_EDITOR_PROP_CALLBACK(bottom, "bottom", &Margin::update, "Bottom", "Bottom edge position.")
    MG_EDITOR_END()
};

struct Padding : public Editable {
    YGNodeRef node = nullptr;

    LayoutSize left;
    LayoutSize right;
    LayoutSize top;
    LayoutSize bottom;

    void update() {
        if (!node) return;

        left.setPadding(node, YGEdgeLeft);
        right.setPadding(node, YGEdgeRight);
        top.setPadding(node, YGEdgeTop);
        bottom.setPadding(node, YGEdgeBottom);
    }

    MG_EDITOR_BEGIN()
        MG_EDITOR_PROP_CALLBACK(left,   "left",   &Padding::update,   "Left",   "Left edge position.")
        MG_EDITOR_PROP_CALLBACK(right,  "right",  &Padding::update,  "Right",  "Right edge position.")
        MG_EDITOR_PROP_CALLBACK(top,    "top",    &Padding::update,    "Top",    "Top edge position.")
        MG_EDITOR_PROP_CALLBACK(bottom, "bottom", &Padding::update, "Bottom", "Bottom edge position.")
    MG_EDITOR_END()
};

struct Gap : public Editable {
    YGNodeRef node = nullptr;

    float row = 0;
    float column = 0;

    void update() {
        if (!node) return;

        YGNodeStyleSetGap(node, YGGutterRow, row);
        YGNodeStyleSetGap(node, YGGutterColumn, column);
    }

    MG_EDITOR_BEGIN()
        MG_EDITOR_PROP_CALLBACK(row,     "row",     &Gap::update,  "Row",     "Row gap.")
        MG_EDITOR_PROP_CALLBACK(column,  "column",  &Gap::update,  "Column",  "Column gap.")
    MG_EDITOR_END()
};

struct Layout : public Editable {
    YGNodeRef node = nullptr;

    FlexContainer flexContainer;
    FlexItem flexItem;
    Position position;
    Margin margin;
    Padding padding;
    Gap gap;

    YGDisplay display = YGDisplayFlex;
    YGOverflow overflow = YGOverflowVisible;

    LayoutSize width;
    LayoutSize height;
    LayoutSize minWidth;
    LayoutSize minHeight;
    LayoutSize maxWidth;
    LayoutSize maxHeight;
    
    float aspectRatio = YGUndefined;

    Layout(YGNodeRef node = nullptr) { if (node) setNode(node); }

    void setNode(YGNodeRef n) {
        node = n;

        flexContainer.node = n;
        flexItem.node = n;
        position.node = n;
        margin.node = n;
        padding.node = n;
        gap.node = n;
    }

    void update() {
        if (!node) return;

        YGNodeStyleSetDisplay(node, display);
        YGNodeStyleSetOverflow(node, overflow);

        width.setWidth(node);
        height.setHeight(node);
        minWidth.setMinWidth(node);
        minHeight.setMinHeight(node);
        maxWidth.setMaxWidth(node);
        maxHeight.setMaxHeight(node);

        YGNodeStyleSetAspectRatio(node, aspectRatio);
    }

    MG_EDITOR_BEGIN()
        MG_EDITOR_PROP(flexContainer, "flex-container", "Flex Container", "Flex container options.")
        MG_EDITOR_PROP(flexItem,      "flex-item",      "Flex Item",      "Flex item options.")
        MG_EDITOR_PROP(position,      "position",       "Position",       "Position options.")
        MG_EDITOR_PROP(margin,        "margin",         "Margin",         "Margin options.")
        MG_EDITOR_PROP(padding,       "padding",        "Padding",        "Padding options.")
        MG_EDITOR_PROP(gap,           "gap",            "Gap",            "Gap options.")
        
        MG_EDITOR_PROP_CALLBACK(overflow,    "overflow",     &Layout::update, "Overflow",     "Overflow setting for content.")
        MG_EDITOR_PROP_CALLBACK(display,     "display",      &Layout::update, "Display",      "Display setting.")
        MG_EDITOR_PROP_CALLBACK(width,       "width",        &Layout::update, "Width",        "Width of the element.")
        MG_EDITOR_PROP_CALLBACK(height,      "height",       &Layout::update, "Height",       "Height of the element.")
        MG_EDITOR_PROP_CALLBACK(minWidth,    "min-width",    &Layout::update, "Min Width",    "Width of the element.")
        MG_EDITOR_PROP_CALLBACK(minHeight,   "min-height",   &Layout::update, "Min Height",   "Height of the element.")
        MG_EDITOR_PROP_CALLBACK(maxWidth,    "max-width",    &Layout::update, "Max Width",    "Width of the element.")
        MG_EDITOR_PROP_CALLBACK(maxHeight,   "max-height",   &Layout::update, "Max Height",   "Height of the element.")
        MG_EDITOR_PROP_CALLBACK(aspectRatio, "aspect-ratio", &Layout::update, "Aspect Ratio", "Aspect ratio of the element.")
    MG_EDITOR_END()
};

CODEC_BEGIN(YGFlexDirection)
    DECODE() {
        if (!v.IsString()) return false;

        const auto s = v.GetString();

        if (!s) return false;

        if (strcmp(s, "row") == 0) { out = YGFlexDirectionRow; return true; }
        if (strcmp(s, "column") == 0) { out = YGFlexDirectionColumn; return true; }
        if (strcmp(s, "row-reverse") == 0) { out = YGFlexDirectionRowReverse; return true; }
        if (strcmp(s, "column-reverse") == 0) { out = YGFlexDirectionColumnReverse; return true; }

        return false;
    }

    ENCODE() {
        out.SetString(YGFlexDirectionToString(v), a);
        return true;
    }
CODEC_END()


CODEC_BEGIN(YGJustify)
    DECODE() {
        if (!v.IsString()) return false;

        const auto s = v.GetString();
        if (!s) return false;

        if (strcmp(s, "flex-start") == 0) { out = YGJustifyFlexStart; return true; }
        if (strcmp(s, "center") == 0)     { out = YGJustifyCenter; return true; }
        if (strcmp(s, "flex-end") == 0)   { out = YGJustifyFlexEnd; return true; }
        if (strcmp(s, "space-between") == 0) { out = YGJustifySpaceBetween; return true; }
        if (strcmp(s, "space-around") == 0)  { out = YGJustifySpaceAround; return true; }
        if (strcmp(s, "space-evenly") == 0)  { out = YGJustifySpaceEvenly; return true; }

        return false;
    }

    ENCODE() {
        out.SetString(YGJustifyToString(v), a);
        return true;
    }
CODEC_END()

CODEC_BEGIN(YGAlign)
    DECODE() {
        if (!v.IsString()) return false;

        const auto s = v.GetString();
        if (!s) return false;

        if (strcmp(s, "auto") == 0)        { out = YGAlignAuto; return true; }
        if (strcmp(s, "flex-start") == 0)  { out = YGAlignFlexStart; return true; }
        if (strcmp(s, "center") == 0)      { out = YGAlignCenter; return true; }
        if (strcmp(s, "flex-end") == 0)    { out = YGAlignFlexEnd; return true; }
        if (strcmp(s, "stretch") == 0)     { out = YGAlignStretch; return true; }
        if (strcmp(s, "baseline") == 0)    { out = YGAlignBaseline; return true; }
        if (strcmp(s, "space-between") == 0) { out = YGAlignSpaceBetween; return true; }
        if (strcmp(s, "space-around") == 0)  { out = YGAlignSpaceAround; return true; }

        return false;
    }

    ENCODE() {
        out.SetString(YGAlignToString(v), a);
        return true;
    }
CODEC_END()


CODEC_BEGIN(YGWrap)
    DECODE() {
        if (!v.IsString()) return false;

        const auto s = v.GetString();
        if (!s) return false;

        if (strcmp(s, "no-wrap") == 0)      { out = YGWrapNoWrap; return true; }
        if (strcmp(s, "wrap") == 0)         { out = YGWrapWrap; return true; }
        if (strcmp(s, "wrap-reverse") == 0) { out = YGWrapWrapReverse; return true; }

        return false;
    }

    ENCODE() {
        out.SetString(YGWrapToString(v), a);
        return true;
    }
CODEC_END()


CODEC_BEGIN(YGPositionType)
    DECODE() {
        if (!v.IsString()) return false;

        const auto s = v.GetString();
        if (!s) return false;

        if (strcmp(s, "static") == 0)   { out = YGPositionTypeStatic; return true; }
        if (strcmp(s, "relative") == 0) { out = YGPositionTypeRelative; return true; }
        if (strcmp(s, "absolute") == 0) { out = YGPositionTypeAbsolute; return true; }

        return false;
    }

    ENCODE() {
        out.SetString(YGPositionTypeToString(v), a);
        return true;
    }
CODEC_END()


CODEC_BEGIN(YGDisplay)
    DECODE() {
        if (!v.IsString()) return false;

        const auto s = v.GetString();
        if (!s) return false;

        if (strcmp(s, "none") == 0) { out = YGDisplayNone; return true; }
        if (strcmp(s, "flex") == 0) { out = YGDisplayFlex; return true; }

        return false;
    }

    ENCODE() {
        out.SetString(YGDisplayToString(v), a);
        return true;
    }
CODEC_END()


CODEC_BEGIN(YGOverflow)
    DECODE() {
        if (!v.IsString()) return false;

        const auto s = v.GetString();
        if (!s) return false;

        if (strcmp(s, "visible") == 0) { out = YGOverflowVisible; return true; }
        if (strcmp(s, "hidden") == 0)  { out = YGOverflowHidden; return true; }
        if (strcmp(s, "scroll") == 0)  { out = YGOverflowScroll; return true; }

        return false;
    }

    ENCODE() {
        out.SetString(YGOverflowToString(v), a);
        return true;
    }
CODEC_END()