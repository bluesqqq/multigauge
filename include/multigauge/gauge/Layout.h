#pragma once

#include <multigauge/properties/PropertyObject.h>
#include <yoga/Yoga.h>

struct LayoutSize {
    enum class Unit { Px, Percent, Auto } unit = Unit::Auto;
	float value = 0.0f;

    void setPosition(YGNodeRef node, YGEdge edge) const {
        switch (unit) {
            case Unit::Px: YGNodeStyleSetPosition(node, edge, value); break;
            case Unit::Percent: YGNodeStyleSetPositionPercent(node, edge, value); break;
            case Unit::Auto: YGNodeStyleSetPosition(node, edge, YGUndefined); break;
        }
    }

    void setMargin(YGNodeRef node, YGEdge edge) const {
        switch (unit) {
            case Unit::Px: YGNodeStyleSetMargin(node, edge, value); break;
            case Unit::Percent: YGNodeStyleSetMarginPercent(node, edge, value); break;
            case Unit::Auto: YGNodeStyleSetMarginAuto(node, edge); break;
        }
    }

    void setPadding(YGNodeRef node, YGEdge edge) const {
        switch (unit) {
            case Unit::Px: YGNodeStyleSetPadding(node, edge, value); break;
            case Unit::Percent: YGNodeStyleSetPaddingPercent(node, edge, value); break;
            case Unit::Auto: YGNodeStyleSetPadding(node, edge, 0.0f); break;
        }
    }

    void setWidth(YGNodeRef node) const {
        switch (unit) {
            case Unit::Px: YGNodeStyleSetWidth(node, value); break;
            case Unit::Percent: YGNodeStyleSetWidthPercent(node, value); break;
            case Unit::Auto: YGNodeStyleSetWidthAuto(node); break;
        }
    }

    void setHeight(YGNodeRef node) const {
        switch (unit) {
            case Unit::Px: YGNodeStyleSetHeight(node, value); break;
            case Unit::Percent: YGNodeStyleSetHeightPercent(node, value); break;
            case Unit::Auto: YGNodeStyleSetHeightAuto(node); break;
        }
    }

    void setMinWidth(YGNodeRef node) const {
        switch (unit) {
            case Unit::Px: YGNodeStyleSetMinWidth(node, value); break;
            case Unit::Percent: YGNodeStyleSetMinWidthPercent(node, value); break;
            case Unit::Auto: YGNodeStyleSetMinWidth(node, YGUndefined); break;
        }
    }

    void setMinHeight(YGNodeRef node) const {
        switch (unit) {
            case Unit::Px: YGNodeStyleSetMinHeight(node, value); break;
            case Unit::Percent: YGNodeStyleSetMinHeightPercent(node, value); break;
            case Unit::Auto: YGNodeStyleSetMinHeight(node, YGUndefined); break;
        }
    }

    void setMaxWidth(YGNodeRef node) const {
        switch (unit) {
            case Unit::Px: YGNodeStyleSetMaxWidth(node, value); break;
            case Unit::Percent: YGNodeStyleSetMaxWidthPercent(node, value); break;
            case Unit::Auto: YGNodeStyleSetMaxWidth(node, YGUndefined); break;
        }
    }

    void setMaxHeight(YGNodeRef node) const {
        switch (unit) {
            case Unit::Px: YGNodeStyleSetMaxHeight(node, value); break;
            case Unit::Percent: YGNodeStyleSetMaxHeightPercent(node, value); break;
            case Unit::Auto: YGNodeStyleSetMaxHeight(node, YGUndefined); break;
        }
    }

    void setBasis(YGNodeRef node) const {
        switch (unit) {
            case Unit::Px: YGNodeStyleSetFlexBasis(node, value); break;
            case Unit::Percent: YGNodeStyleSetFlexBasisPercent(node, value); break;
            case Unit::Auto: YGNodeStyleSetFlexBasisAuto(node); break;
        }
    }
};

MG_EDITOR_WIDGET(LayoutSize, "layout-size")

struct Margin;
struct Padding;

MG_EDITOR_WIDGET(Margin, "box")
MG_EDITOR_WIDGET(Padding, "box")

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

        return false;
    }

    ENCODE() {
        switch (v.unit) {
            case LayoutSize::Unit::Auto:
                out.SetString("auto", a);
                break;

            case LayoutSize::Unit::Percent: {
                char buf[32];
                std::snprintf(buf, sizeof(buf), "%g%%", v.value);
                out.SetString(buf, a);
                break;
            }

            case LayoutSize::Unit::Px: default: out.SetFloat(v.value); break;
        }

        return true;
    }
CODEC_END()

template<>
struct EnumTraits<YGFlexDirection> {
    static constexpr EnumOption<YGFlexDirection> options[] = {
        { YGFlexDirectionRow,           "row",            "Row" },
        { YGFlexDirectionColumn,        "column",         "Column" },
        { YGFlexDirectionRowReverse,    "row-reverse",    "Row Reverse" },
        { YGFlexDirectionColumnReverse, "column-reverse", "Column Reverse" },
    };
};

template<>
struct EnumTraits<YGJustify> {
    static constexpr EnumOption<YGJustify> options[] = {
        { YGJustifyFlexStart,    "flex-start",    "Flex Start" },
        { YGJustifyCenter,       "center",        "Center" },
        { YGJustifyFlexEnd,      "flex-end",      "Flex End" },
        { YGJustifySpaceBetween, "space-between", "Space Between" },
        { YGJustifySpaceAround,  "space-around",  "Space Around" },
        { YGJustifySpaceEvenly,  "space-evenly",  "Space Evenly" },
    };
};

template<>
struct EnumTraits<YGAlign> {
    static constexpr EnumOption<YGAlign> options[] = {
        { YGAlignFlexStart,    "flex-start",    "Flex Start" },
        { YGAlignCenter,       "center",        "Center" },
        { YGAlignFlexEnd,      "flex-end",      "Flex End" },
    };
};

MG_EDITOR_WIDGET(YGFlexDirection, "segmented-select")
MG_EDITOR_WIDGET(YGAlign, "segmented-select")
MG_EDITOR_WIDGET(YGJustify, "justify")

template<>
struct EnumTraits<YGWrap> {
    static constexpr EnumOption<YGWrap> options[] = {
        { YGWrapNoWrap,      "no-wrap",      "No Wrap" },
        { YGWrapWrap,        "wrap",         "Wrap" },
        { YGWrapWrapReverse, "wrap-reverse", "Wrap Reverse" },
    };
};

template<>
struct EnumTraits<YGPositionType> {
    static constexpr EnumOption<YGPositionType> options[] = {
        { YGPositionTypeStatic,   "static",   "Static" },
        { YGPositionTypeRelative, "relative", "Relative" },
        { YGPositionTypeAbsolute, "absolute", "Absolute" },
    };
};

template<>
struct EnumTraits<YGDisplay> {
    static constexpr EnumOption<YGDisplay> options[] = {
        { YGDisplayNone, "off", "Off" },
        { YGDisplayFlex, "on", "On" },
    };
};

template<>
struct EnumTraits<YGOverflow> {
    static constexpr EnumOption<YGOverflow> options[] = {
        { YGOverflowVisible, "visible", "Visible" },
        { YGOverflowHidden,  "hidden",  "Hidden" },
    };
};

struct Gap : public PropertyObject {
    float row = 0;
    float column = 0;

    void apply(YGNodeRef node) const {
        YGNodeStyleSetGap(node, YGGutterRow, row);
        YGNodeStyleSetGap(node, YGGutterColumn, column);
    }

    static rapidjson::Value rowInteractableWhen(rapidjson::Document::AllocatorType& a) {
        return MG_UI_RULES(
            MG_UI_RULE_ANY(
                MG_UI_RULE("../wrap", "!=", "no-wrap"),
                MG_UI_RULE_IN("../direction", "column", "column-reverse")
            )
        );
    }

    static rapidjson::Value columnInteractableWhen(rapidjson::Document::AllocatorType& a) {
        return MG_UI_RULES(
            MG_UI_RULE_ANY(
                MG_UI_RULE("../wrap", "!=", "no-wrap"),
                MG_UI_RULE_IN("../direction", "row", "row-reverse")
            )
        );
    }

    MG_PROPS_BEGIN()
    MG_PROP_UI(row,     "row",     "Row",     "Row gap.",    nullptr, &Gap::rowInteractableWhen)
    MG_PROP_UI(column,  "column",  "Column",  "Column gap.", nullptr, &Gap::columnInteractableWhen)
    MG_PROPS_END()
};

struct FlexContainer : public PropertyObject {
    YGFlexDirection direction = YGFlexDirectionRow;
    YGJustify justify = YGJustifyFlexStart;
    YGAlign alignItems = YGAlignStretch;
    YGAlign alignContent = YGAlignStretch;
    YGWrap wrap = YGWrapNoWrap;
    Gap gap;

    void apply(YGNodeRef node) const {
        YGNodeStyleSetFlexDirection(node, direction);
        YGNodeStyleSetJustifyContent(node, justify);
        YGNodeStyleSetAlignItems(node, alignItems);
        YGNodeStyleSetAlignContent(node, alignContent);
        YGNodeStyleSetFlexWrap(node, wrap);
        gap.apply(node);
    }

    static rapidjson::Value alignContentVisibleWhen(rapidjson::Document::AllocatorType& a) {
        return MG_UI_RULES(
            MG_UI_RULE("wrap", "!=", "no-wrap")
        );
    }

    MG_PROPS_BEGIN()
    MG_PROP(direction,    "direction",     "Direction",     "Direction of flex content.")
    MG_PROP(justify,      "justify",       "Justify",       "Main-axis alignment of children.")
    MG_PROP(alignItems,   "align-items",   "Align Items",   "Cross-axis alignment of children.")
    MG_PROP_UI(alignContent, "align-content", "Align Content", "Cross-axis alignment of children.", &FlexContainer::alignContentVisibleWhen, nullptr)
    MG_PROP(wrap,         "wrap",          "Wrap",          "Wrap setting for children.")
    MG_PROP(gap,                   "gap",           "Gap",           "Gap options.")
    MG_PROPS_END()
};

struct FlexItem : public PropertyObject {
    float grow = 0;
    float shrink = 1;
    LayoutSize basis;
    YGAlign alignSelf = YGAlignAuto;

    void apply(YGNodeRef node) const {
        YGNodeStyleSetFlexGrow(node, grow);
        YGNodeStyleSetFlexShrink(node, shrink);
        basis.setBasis(node);
        YGNodeStyleSetAlignSelf(node, alignSelf);
    }

    MG_PROPS_BEGIN()
    MG_PROP(grow,      "grow",       "Grow",       "Rate element expands to fill avaiable space.")
    MG_PROP(shrink,    "shrink",     "Shrink",     "Rate element contracts to avoid overflow.")
    MG_PROP(basis,     "basis",      "Basis",      "Flex basis.")
    MG_PROP(alignSelf, "align-self", "Align Self", "Align self.")
    MG_PROPS_END()
};

struct Position : public PropertyObject {
    YGPositionType type = YGPositionTypeStatic;

    LayoutSize left;
    LayoutSize right;
    LayoutSize top;
    LayoutSize bottom;

    static rapidjson::Value insetVisibleWhen(rapidjson::Document::AllocatorType& a) {
        return MG_UI_RULES(
            MG_UI_RULE("type", "!=", "static")
        );
    }

    void apply(YGNodeRef node) const {
        YGNodeStyleSetPositionType(node, type);

        if (type == YGPositionTypeStatic) {
            YGNodeStyleSetPosition(node, YGEdgeLeft, YGUndefined);
            YGNodeStyleSetPosition(node, YGEdgeRight, YGUndefined);
            YGNodeStyleSetPosition(node, YGEdgeTop, YGUndefined);
            YGNodeStyleSetPosition(node, YGEdgeBottom, YGUndefined);
            return;
        }

        left.setPosition(node, YGEdgeLeft);
        right.setPosition(node, YGEdgeRight);
        top.setPosition(node, YGEdgeTop);
        bottom.setPosition(node, YGEdgeBottom);
    }

    MG_PROPS_BEGIN()
        MG_PROP(type, "type", "Type", "Positioning type.")

        MG_PROP_UI(left,   "left",   "Left",   "Left edge position.",   &Position::insetVisibleWhen, nullptr)
        MG_PROP_UI(right,  "right",  "Right",  "Right edge position.",  &Position::insetVisibleWhen, nullptr)
        MG_PROP_UI(top,    "top",    "Top",    "Top edge position.",    &Position::insetVisibleWhen, nullptr)
        MG_PROP_UI(bottom, "bottom", "Bottom", "Bottom edge position.", &Position::insetVisibleWhen, nullptr)
    MG_PROPS_END()
};

struct LayoutBox : public PropertyObject {
    LayoutSize left{LayoutSize::Unit::Px, 0.0f};
    LayoutSize right{LayoutSize::Unit::Px, 0.0f};
    LayoutSize top{LayoutSize::Unit::Px, 0.0f};
    LayoutSize bottom{LayoutSize::Unit::Px, 0.0f};

    virtual void apply(YGNodeRef node) const = 0;

    MG_PROPS_BEGIN()
        MG_PROP(left,   "left",   "Left",   "Left edge position.")
        MG_PROP(right,  "right",  "Right",  "Right edge position.")
        MG_PROP(top,    "top",    "Top",    "Top edge position.")
        MG_PROP(bottom, "bottom", "Bottom", "Bottom edge position.")
    MG_PROPS_END()
};

struct Margin : public LayoutBox {
    MG_PROPS_PARENT(LayoutBox)

    void apply(YGNodeRef node) const {
        left.setMargin(node, YGEdgeLeft);
        right.setMargin(node, YGEdgeRight);
        top.setMargin(node, YGEdgeTop);
        bottom.setMargin(node, YGEdgeBottom);
    }
};

struct Padding : public LayoutBox {
    MG_PROPS_PARENT(LayoutBox)

    void apply(YGNodeRef node) const {
        left.setPadding(node, YGEdgeLeft);
        right.setPadding(node, YGEdgeRight);
        top.setPadding(node, YGEdgeTop);
        bottom.setPadding(node, YGEdgeBottom);
    }
};

struct Layout : public PropertyObject {
    FlexContainer flexContainer;
    FlexItem flexItem;
    Position position;
    Margin margin;
    Padding padding;

    YGDisplay display = YGDisplayFlex;
    YGOverflow overflow = YGOverflowVisible;

    LayoutSize width;
    LayoutSize height;
    LayoutSize minWidth;
    LayoutSize minHeight;
    LayoutSize maxWidth;
    LayoutSize maxHeight;
    
    float aspectRatio = 0.0f;

    Layout() = default;

    void apply(YGNodeRef node) const {
        flexContainer.apply(node);
        flexItem.apply(node);
        position.apply(node);
        margin.apply(node);
        padding.apply(node);

        YGNodeStyleSetDisplay(node, display);
        YGNodeStyleSetOverflow(node, overflow);

        width.setWidth(node);
        height.setHeight(node);
        minWidth.setMinWidth(node);
        minHeight.setMinHeight(node);
        maxWidth.setMaxWidth(node);
        maxHeight.setMaxHeight(node);

        if (aspectRatio > 0.0f) YGNodeStyleSetAspectRatio(node, aspectRatio);
        else YGNodeStyleSetAspectRatio(node, YGUndefined);
    }

    MG_PROPS_BEGIN()
        MG_PROP(flexContainer, "flex-container", "Flex Container", "Flex container options.")
        MG_PROP(flexItem,      "flex-item",      "Flex Item",      "Flex item options.")
        MG_PROP(position,      "position",       "Position",       "Position options.")
        MG_PROP(margin,        "margin",         "Margin",         "Margin options.")
        MG_PROP(padding,       "padding",        "Padding",        "Padding options.")

        MG_PROP(overflow,    "overflow",     "Overflow",     "Overflow setting for content.")
        MG_PROP_HIDDEN(display, "display",   "Display",      "Whether this element participates in layout.")
        MG_PROP(width,       "width",        "Width",        "Width of the element.")
        MG_PROP(height,      "height",       "Height",       "Height of the element.")
        MG_PROP(minWidth,    "min-width",    "Min Width",    "Width of the element.")
        MG_PROP(minHeight,   "min-height",   "Min Height",   "Height of the element.")
        MG_PROP(maxWidth,    "max-width",    "Max Width",    "Width of the element.")
        MG_PROP(maxHeight,   "max-height",   "Max Height",   "Height of the element.")
        MG_PROP(aspectRatio, "aspect-ratio", "Aspect Ratio", "Aspect ratio of the element.")
    MG_PROPS_END()
};

struct RootLayout : public PropertyObject {
    FlexContainer flexContainer;
    Padding padding;

    RootLayout() = default;

    void apply(YGNodeRef node) const {
        flexContainer.apply(node);
        padding.apply(node);
    }

    MG_PROPS_BEGIN()
        MG_PROP(flexContainer, "flex-container", "Flex Container", "Flex container options.")
        MG_PROP(padding,       "padding",        "Padding",        "Padding inside the root.")
    MG_PROPS_END()
};

CODEC_BEGIN(YGFlexDirection)
    DECODE() {
        return decodeEnum(v, out);
    }

    ENCODE() {
        return encodeEnum(out, a, v);
    }
CODEC_END()


CODEC_BEGIN(YGJustify)
    DECODE() {
        return decodeEnum(v, out);
    }

    ENCODE() {
        return encodeEnum(out, a, v);
    }
CODEC_END()

CODEC_BEGIN(YGAlign)
    DECODE() {
        return decodeEnum(v, out);
    }

    ENCODE() {
        return encodeEnum(out, a, v);
    }
CODEC_END()


CODEC_BEGIN(YGWrap)
    DECODE() {
        return decodeEnum(v, out);
    }

    ENCODE() {
        return encodeEnum(out, a, v);
    }
CODEC_END()


CODEC_BEGIN(YGPositionType)
    DECODE() {
        return decodeEnum(v, out);
    }

    ENCODE() {
        return encodeEnum(out, a, v);
    }
CODEC_END()


CODEC_BEGIN(YGDisplay)
    DECODE() {
        return decodeEnum(v, out);
    }

    ENCODE() {
        return encodeEnum(out, a, v);
    }
CODEC_END()


CODEC_BEGIN(YGOverflow)
    DECODE() {
        return decodeEnum(v, out);
    }

    ENCODE() {
        return encodeEnum(out, a, v);
    }
CODEC_END()
