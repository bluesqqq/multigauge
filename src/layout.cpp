#include "layout.h"

// Helper: safely get object member without double lookup
static inline const rapidjson::Value* getMember(const rapidjson::Value::ConstObject& o, const char* key) {
    auto it = o.FindMember(key);
    return (it != o.MemberEnd()) ? &it->value : nullptr;
}

void loadLayout(YGNodeRef node, const rapidjson::Value::ConstObject json) {
    if (!node) return;

    const rapidjson::Value* styleV = getMember(json, "style");
    if (!styleV || !styleV->IsObject()) return;
    const rapidjson::Value::ConstObject style = styleV->GetObject();

    // ---- size ----
    if (const rapidjson::Value* v = getMember(style, "width"))     applyWidth(node, *v);
    if (const rapidjson::Value* v = getMember(style, "height"))    applyHeight(node, *v);
    if (const rapidjson::Value* v = getMember(style, "minWidth"))  applyMinWidth(node, *v);
    if (const rapidjson::Value* v = getMember(style, "minHeight")) applyMinHeight(node, *v);
    if (const rapidjson::Value* v = getMember(style, "maxWidth"))  applyMaxWidth(node, *v);
    if (const rapidjson::Value* v = getMember(style, "maxHeight")) applyMaxHeight(node, *v);

    if (const rapidjson::Value* v = getMember(style, "aspectRatio"); v && v->IsNumber())
        YGNodeStyleSetAspectRatio(node, v->GetFloat());

    // ---- display / overflow / direction ----
    if (const rapidjson::Value* v = getMember(style, "display"); v && v->IsString())
        YGNodeStyleSetDisplay(node, parseDisplay(v->GetString()));

    if (const rapidjson::Value* v = getMember(style, "overflow"); v && v->IsString())
        YGNodeStyleSetOverflow(node, parseOverflow(v->GetString()));

    if (const rapidjson::Value* v = getMember(style, "direction"); v && v->IsString()) {
        const char* d = v->GetString();
        if (strcmp(d, "rtl") == 0)      YGNodeStyleSetDirection(node, YGDirectionRTL);
        else if (strcmp(d, "ltr") == 0) YGNodeStyleSetDirection(node, YGDirectionLTR);
        else                            YGNodeStyleSetDirection(node, YGDirectionInherit);
    }

    // ---- flex block
    if (const rapidjson::Value* flexV = getMember(style, "flex"); flexV && flexV->IsObject()) {
        const rapidjson::Value::ConstObject flex = flexV->GetObject();

        if (const rapidjson::Value* v = getMember(flex, "direction"); v && v->IsString())
            YGNodeStyleSetFlexDirection(node, parseFlexDirection(v->GetString()));

        if (const rapidjson::Value* v = getMember(flex, "justify"); v && v->IsString())
            YGNodeStyleSetJustifyContent(node, parseJustify(v->GetString()));

        if (const rapidjson::Value* v = getMember(flex, "align"); v && v->IsString())
            YGNodeStyleSetAlignItems(node, parseAlign(v->GetString()));

        if (const rapidjson::Value* v = getMember(flex, "alignSelf"); v && v->IsString())
            YGNodeStyleSetAlignSelf(node, parseAlign(v->GetString()));

        if (const rapidjson::Value* v = getMember(flex, "alignContent"); v && v->IsString())
            YGNodeStyleSetAlignContent(node, parseAlign(v->GetString()));

        if (const rapidjson::Value* v = getMember(flex, "wrap"); v && v->IsString())
            YGNodeStyleSetFlexWrap(node, parseWrap(v->GetString()));

        if (const rapidjson::Value* v = getMember(flex, "grow"); v && v->IsNumber())
            YGNodeStyleSetFlexGrow(node, v->GetFloat());

        if (const rapidjson::Value* v = getMember(flex, "shrink"); v && v->IsNumber())
            YGNodeStyleSetFlexShrink(node, v->GetFloat());

        if (const rapidjson::Value* v = getMember(flex, "basis"))
            applyFlexBasis(node, *v);
    }

    // ---- spacing
    if (const rapidjson::Value* v = getMember(style, "margin"); v && v->IsObject())
        applyEdgesMargin(node, v->GetObject());

    if (const rapidjson::Value* v = getMember(style, "padding"); v && v->IsObject())
        applyEdgesPadding(node, v->GetObject());

    if (const rapidjson::Value* v = getMember(style, "border"); v && v->IsObject())
        applyEdgesBorder(node, v->GetObject());

    // ---- gap
    if (const rapidjson::Value* gapV = getMember(style, "gap"); gapV && gapV->IsObject()) {
        const rapidjson::Value::ConstObject gap = gapV->GetObject();

        if (const rapidjson::Value* v = getMember(gap, "row"); v && v->IsNumber())
            YGNodeStyleSetGap(node, YGGutterRow, v->GetFloat());

        if (const rapidjson::Value* v = getMember(gap, "column"); v && v->IsNumber())
            YGNodeStyleSetGap(node, YGGutterColumn, v->GetFloat());
    }

    // ---- position
    if (const rapidjson::Value* posV = getMember(style, "position"); posV && posV->IsObject()) {
        const rapidjson::Value::ConstObject pos = posV->GetObject();

        if (const rapidjson::Value* v = getMember(pos, "type"); v && v->IsString())
            YGNodeStyleSetPositionType(node, parsePositionType(v->GetString()));

        applyPositionEdges(node, pos);
    }
}
