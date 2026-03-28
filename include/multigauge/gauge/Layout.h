#pragma once

#include <multigauge/editor/PropertyObject.h>
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
    YGNodeRef node = nullptr;

    float row = 0;
    float column = 0;

    void update() {
        if (!node) return;

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
    MG_PROP_CALLBACK_UI(row,     "row",     "Row",     "Row gap.",    &Gap::update, nullptr, &Gap::rowInteractableWhen)
    MG_PROP_CALLBACK_UI(column,  "column",  "Column",  "Column gap.", &Gap::update, nullptr, &Gap::columnInteractableWhen)
    MG_PROPS_END()
};

struct FlexContainer : public PropertyObject {
    YGNodeRef node = nullptr;

    YGFlexDirection direction = YGFlexDirectionRow;
    YGJustify justify = YGJustifyFlexStart;
    YGAlign alignItems = YGAlignStretch;
    YGAlign alignContent = YGAlignStretch;
    YGWrap wrap = YGWrapNoWrap;
    Gap gap;

    void update() {
        if (!node) return;

        YGNodeStyleSetFlexDirection(node, direction);
        YGNodeStyleSetJustifyContent(node, justify);
        YGNodeStyleSetAlignItems(node, alignItems);
        YGNodeStyleSetAlignContent(node, alignContent);
        YGNodeStyleSetFlexWrap(node, wrap);
        gap.update();
    }

    static rapidjson::Value alignContentVisibleWhen(rapidjson::Document::AllocatorType& a) {
        return MG_UI_RULES(
            MG_UI_RULE("wrap", "!=", "no-wrap")
        );
    }

    MG_PROPS_BEGIN()
    MG_PROP_CALLBACK(direction,    "direction",     "Direction",     "Direction of flex content.",        &FlexContainer::update)
    MG_PROP_CALLBACK(justify,      "justify",       "Justify",       "Main-axis alignment of children.",  &FlexContainer::update)
    MG_PROP_CALLBACK(alignItems,   "align-items",   "Align Items",   "Cross-axis alignment of children.", &FlexContainer::update)
    MG_PROP_CALLBACK_UI(alignContent, "align-content", "Align Content", "Cross-axis alignment of children.", &FlexContainer::update, &FlexContainer::alignContentVisibleWhen, nullptr)
    MG_PROP_CALLBACK(wrap,         "wrap",          "Wrap",          "Wrap setting for children.",        &FlexContainer::update)
    MG_PROP(gap,                   "gap",           "Gap",           "Gap options.")
    MG_PROPS_END()
};

struct FlexItem : public PropertyObject {
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

    MG_PROPS_BEGIN()
    MG_PROP_CALLBACK(grow,      "grow",       "Grow",       "Rate element expands to fill avaiable space.", &FlexItem::update)
    MG_PROP_CALLBACK(shrink,    "shrink",     "Shrink",     "Rate element contracts to avoid overflow.",    &FlexItem::update)
    MG_PROP_CALLBACK(basis,     "basis",      "Basis",      "Flex basis.",                                   &FlexItem::update)
    MG_PROP_CALLBACK(alignSelf, "align-self", "Align Self", "Align self.",                                   &FlexItem::update)
    MG_PROPS_END()
};

struct Position : public PropertyObject {
    YGNodeRef node = nullptr;
    
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

    void update() {
        if (!node) return;
        
        YGNodeStyleSetPositionType(node, type);

        if (type == YGPositionTypeStatic) {
            // Yoga should ignore inset edges for static positioning, but this
            // vendored version doesn't (assuming its a bug) so I'm just handling it
            // here instead of modifying yoga. 
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
    MG_PROP_CALLBACK(type, "type", "Type", "Positioning type.", &Position::update)

    MG_PROP_CALLBACK_UI(left,   "left",   "Left",   "Left edge position.",   &Position::update, &Position::insetVisibleWhen, nullptr)
    MG_PROP_CALLBACK_UI(right,  "right",  "Right",  "Right edge position.",  &Position::update, &Position::insetVisibleWhen, nullptr)
    MG_PROP_CALLBACK_UI(top,    "top",    "Top",    "Top edge position.",    &Position::update, &Position::insetVisibleWhen, nullptr)
    MG_PROP_CALLBACK_UI(bottom, "bottom", "Bottom", "Bottom edge position.", &Position::update, &Position::insetVisibleWhen, nullptr)
    MG_PROPS_END()
};

struct LayoutBox : public PropertyObject {
    LayoutSize left{LayoutSize::Unit::Px, 0.0f};
    LayoutSize right{LayoutSize::Unit::Px, 0.0f};
    LayoutSize top{LayoutSize::Unit::Px, 0.0f};
    LayoutSize bottom{LayoutSize::Unit::Px, 0.0f};

    virtual void update() {}

    MG_PROPS_BEGIN()
    MG_PROP_CALLBACK(left,   "left",   "Left",   "Left edge position.",   &LayoutBox::update)
    MG_PROP_CALLBACK(right,  "right",  "Right",  "Right edge position.",  &LayoutBox::update)
    MG_PROP_CALLBACK(top,    "top",    "Top",    "Top edge position.",    &LayoutBox::update)
    MG_PROP_CALLBACK(bottom, "bottom", "Bottom", "Bottom edge position.", &LayoutBox::update)
    MG_PROPS_END()
};

struct Margin : public LayoutBox {
    MG_PROPS_PARENT(LayoutBox)

    YGNodeRef node = nullptr;

    void update() {
        if (!node) return;

        left.setMargin(node, YGEdgeLeft);
        right.setMargin(node, YGEdgeRight);
        top.setMargin(node, YGEdgeTop);
        bottom.setMargin(node, YGEdgeBottom);
    }
};

struct Padding : public LayoutBox {
    MG_PROPS_PARENT(LayoutBox)

    YGNodeRef node = nullptr;

    void update() {
        if (!node) return;

        left.setPadding(node, YGEdgeLeft);
        right.setPadding(node, YGEdgeRight);
        top.setPadding(node, YGEdgeTop);
        bottom.setPadding(node, YGEdgeBottom);
    }
};

struct Layout : public PropertyObject {
    YGNodeRef node = nullptr;

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

    Layout(YGNodeRef node = nullptr) { if (node) setNode(node); }

    void setNode(YGNodeRef n) {
        node = n;

        flexContainer.node = n;
        flexContainer.gap.node = n;
        flexItem.node = n;
        position.node = n;
        margin.node = n;
        padding.node = n;
    }

    void update() {
        if (!node) return;

        flexContainer.update();
        flexItem.update();
        position.update();
        margin.update();
        padding.update();

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

    MG_PROP_CALLBACK(overflow,    "overflow",     "Overflow",     "Overflow setting for content.", &Layout::update)
    MG_PROP_CALLBACK_HIDDEN(display, "display",   "Display",      "Whether this element participates in layout.", &Layout::update)
    MG_PROP_CALLBACK(width,       "width",        "Width",        "Width of the element.",         &Layout::update)
    MG_PROP_CALLBACK(height,      "height",       "Height",       "Height of the element.",        &Layout::update)
    MG_PROP_CALLBACK(minWidth,    "min-width",    "Min Width",    "Width of the element.",         &Layout::update)
    MG_PROP_CALLBACK(minHeight,   "min-height",   "Min Height",   "Height of the element.",        &Layout::update)
    MG_PROP_CALLBACK(maxWidth,    "max-width",    "Max Width",    "Width of the element.",         &Layout::update)
    MG_PROP_CALLBACK(maxHeight,   "max-height",   "Max Height",   "Height of the element.",        &Layout::update)
    MG_PROP_CALLBACK(aspectRatio, "aspect-ratio", "Aspect Ratio", "Aspect ratio of the element.",  &Layout::update)
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
