#include "rapidjson/document.h"
#include <yoga/Yoga.h>
#include <cctype>

static YGFlexDirection parseFlexDirection(const char* s) {
	if (!s) return YGFlexDirectionColumn;
	if (strcmp(s, "row") == 0) return YGFlexDirectionRow;
	if (strcmp(s, "row-reverse") == 0) return YGFlexDirectionRowReverse;
	if (strcmp(s, "column-reverse") == 0) return YGFlexDirectionColumnReverse;
	return YGFlexDirectionColumn;
}

static YGJustify parseJustify(const char* s) {
	if (!s) return YGJustifyFlexStart;
	if (strcmp(s, "center") == 0) return YGJustifyCenter;
	if (strcmp(s, "flex-end") == 0) return YGJustifyFlexEnd;
	if (strcmp(s, "space-between") == 0) return YGJustifySpaceBetween;
	if (strcmp(s, "space-around") == 0) return YGJustifySpaceAround;
	if (strcmp(s, "space-evenly") == 0) return YGJustifySpaceEvenly;
	return YGJustifyFlexStart;
}

static YGAlign parseAlign(const char* s) {
	if (!s) return YGAlignStretch;
	if (strcmp(s, "flex-start") == 0) return YGAlignFlexStart;
	if (strcmp(s, "center") == 0) return YGAlignCenter;
	if (strcmp(s, "flex-end") == 0) return YGAlignFlexEnd;
	if (strcmp(s, "baseline") == 0) return YGAlignBaseline;
	if (strcmp(s, "stretch") == 0) return YGAlignStretch;
	return YGAlignStretch;
}

static YGWrap parseWrap(const char* s) {
	if (!s) return YGWrapNoWrap;
	if (strcmp(s, "wrap") == 0) return YGWrapWrap;
	if (strcmp(s, "wrap-reverse") == 0) return YGWrapWrapReverse;
	return YGWrapNoWrap;
}

static YGPositionType parsePositionType(const char* s) {
	if (!s) return YGPositionTypeRelative;
	if (strcmp(s, "absolute") == 0) return YGPositionTypeAbsolute;
	return YGPositionTypeRelative;
}

static YGDisplay parseDisplay(const char* s) {
	if (!s) return YGDisplayFlex;
	if (strcmp(s, "none") == 0) return YGDisplayNone;
	return YGDisplayFlex;
}

static YGOverflow parseOverflow(const char* s) {
	if (!s) return YGOverflowVisible;
	if (strcmp(s, "hidden") == 0) return YGOverflowHidden;
	if (strcmp(s, "scroll") == 0) return YGOverflowScroll;
	return YGOverflowVisible;
}

struct JsonSize {
	enum class Unit { Px, Percent, Auto, Undefined } unit = Unit::Undefined;
	float value = 0.0f;
};

static const char* skipWs(const char* s) {
    while (s && *s && std::isspace(static_cast<unsigned char>(*s))) ++s;
    return s;
}

static JsonSize parseSize(const rapidjson::Value& v) {
    JsonSize out;

    if (v.IsNull()) return out;
	
    if (v.IsNumber()) {
        out.unit = JsonSize::Unit::Px;
        out.value = v.GetFloat();
        return out;
    }

    if (v.IsString()) {
        const char* raw = v.GetString();
        const char* s = skipWs(raw);

        if (!s || *s == '\0') return out;

        if (strcmp(s, "auto") == 0) {
            out.unit = JsonSize::Unit::Auto;
            return out;
        }
        if (strcmp(s, "undefined") == 0) {
            out.unit = JsonSize::Unit::Undefined;
            return out;
        }

        char* end = nullptr;
        float num = std::strtof(s, &end);
        if (end == s) return out;

        end = const_cast<char*>(skipWs(end));

        if (*end == '\0') {
            out.unit = JsonSize::Unit::Px;
            out.value = num;
            return out;
        }

        if (*end == '%') {
            out.unit = JsonSize::Unit::Percent;
            out.value = num;
            return out;
        }

        if ((end[0] == 'p' || end[0] == 'P') && (end[1] == 'x' || end[1] == 'X')) {
            out.unit = JsonSize::Unit::Px;
            out.value = num;
            return out;
        }

        return out;
    }

    return out;
}


static void applyWidth(YGNodeRef node, const rapidjson::Value& v) {
	JsonSize s = parseSize(v);
	switch (s.unit) {
		case JsonSize::Unit::Px:      YGNodeStyleSetWidth(node, s.value); break;
		case JsonSize::Unit::Percent: YGNodeStyleSetWidthPercent(node, s.value); break;
		case JsonSize::Unit::Auto:    YGNodeStyleSetWidthAuto(node); break;
		case JsonSize::Unit::Undefined: default: break;
	}
}

static void applyHeight(YGNodeRef node, const rapidjson::Value& v) {
	JsonSize s = parseSize(v);
	switch (s.unit) {
		case JsonSize::Unit::Px:      YGNodeStyleSetHeight(node, s.value); break;
		case JsonSize::Unit::Percent: YGNodeStyleSetHeightPercent(node, s.value); break;
		case JsonSize::Unit::Auto:    YGNodeStyleSetHeightAuto(node); break;
		case JsonSize::Unit::Undefined: default: break;
	}
}

static void applyMinWidth(YGNodeRef node, const rapidjson::Value& v) {
	JsonSize s = parseSize(v);
	if (s.unit == JsonSize::Unit::Px) YGNodeStyleSetMinWidth(node, s.value);
	else if (s.unit == JsonSize::Unit::Percent) YGNodeStyleSetMinWidthPercent(node, s.value);
}

static void applyMinHeight(YGNodeRef node, const rapidjson::Value& v) {
	JsonSize s = parseSize(v);
	if (s.unit == JsonSize::Unit::Px) YGNodeStyleSetMinHeight(node, s.value);
	else if (s.unit == JsonSize::Unit::Percent) YGNodeStyleSetMinHeightPercent(node, s.value);
}

static void applyMaxWidth(YGNodeRef node, const rapidjson::Value& v) {
	JsonSize s = parseSize(v);
	if (s.unit == JsonSize::Unit::Px) YGNodeStyleSetMaxWidth(node, s.value);
	else if (s.unit == JsonSize::Unit::Percent) YGNodeStyleSetMaxWidthPercent(node, s.value);
}

static void applyMaxHeight(YGNodeRef node, const rapidjson::Value& v) {
	JsonSize s = parseSize(v);
	if (s.unit == JsonSize::Unit::Px) YGNodeStyleSetMaxHeight(node, s.value);
	else if (s.unit == JsonSize::Unit::Percent) YGNodeStyleSetMaxHeightPercent(node, s.value);
}

static void applyFlexBasis(YGNodeRef node, const rapidjson::Value& v) {
	JsonSize s = parseSize(v);
	switch (s.unit) {
		case JsonSize::Unit::Px:      YGNodeStyleSetFlexBasis(node, s.value); break;
		case JsonSize::Unit::Percent: YGNodeStyleSetFlexBasisPercent(node, s.value); break;
		case JsonSize::Unit::Auto:    YGNodeStyleSetFlexBasisAuto(node); break;
		default: break;
	}
}

static void applyEdgesMargin(YGNodeRef node, const rapidjson::Value::ConstObject edges) {
	if (edges.HasMember("all") && edges["all"].IsNumber()) YGNodeStyleSetMargin(node, YGEdgeAll, edges["all"].GetFloat());
	if (edges.HasMember("horizontal") && edges["horizontal"].IsNumber()) YGNodeStyleSetMargin(node, YGEdgeHorizontal, edges["horizontal"].GetFloat());
	if (edges.HasMember("vertical") && edges["vertical"].IsNumber()) YGNodeStyleSetMargin(node, YGEdgeVertical, edges["vertical"].GetFloat());
	if (edges.HasMember("left") && edges["left"].IsNumber()) YGNodeStyleSetMargin(node, YGEdgeLeft, edges["left"].GetFloat());
	if (edges.HasMember("top") && edges["top"].IsNumber()) YGNodeStyleSetMargin(node, YGEdgeTop, edges["top"].GetFloat());
	if (edges.HasMember("right") && edges["right"].IsNumber()) YGNodeStyleSetMargin(node, YGEdgeRight, edges["right"].GetFloat());
	if (edges.HasMember("bottom") && edges["bottom"].IsNumber()) YGNodeStyleSetMargin(node, YGEdgeBottom, edges["bottom"].GetFloat());
}

static void applyEdgesPadding(YGNodeRef node, const rapidjson::Value::ConstObject edges) {
	if (edges.HasMember("all") && edges["all"].IsNumber())  YGNodeStyleSetPadding(node, YGEdgeAll, edges["all"].GetFloat());
	if (edges.HasMember("horizontal") && edges["horizontal"].IsNumber()) YGNodeStyleSetPadding(node, YGEdgeHorizontal, edges["horizontal"].GetFloat());
	if (edges.HasMember("vertical") && edges["vertical"].IsNumber()) YGNodeStyleSetPadding(node, YGEdgeVertical, edges["vertical"].GetFloat());
	if (edges.HasMember("left") && edges["left"].IsNumber()) YGNodeStyleSetPadding(node, YGEdgeLeft, edges["left"].GetFloat());
	if (edges.HasMember("top") && edges["top"].IsNumber()) YGNodeStyleSetPadding(node, YGEdgeTop, edges["top"].GetFloat());
	if (edges.HasMember("right") && edges["right"].IsNumber()) YGNodeStyleSetPadding(node, YGEdgeRight, edges["right"].GetFloat());
	if (edges.HasMember("bottom") && edges["bottom"].IsNumber()) YGNodeStyleSetPadding(node, YGEdgeBottom, edges["bottom"].GetFloat());
}

static void applyEdgesBorder(YGNodeRef node, const rapidjson::Value::ConstObject edges) {
	if (edges.HasMember("all") && edges["all"].IsNumber()) YGNodeStyleSetBorder(node, YGEdgeAll, edges["all"].GetFloat());
	if (edges.HasMember("horizontal") && edges["horizontal"].IsNumber()) YGNodeStyleSetBorder(node, YGEdgeHorizontal, edges["horizontal"].GetFloat());
	if (edges.HasMember("vertical") && edges["vertical"].IsNumber()) YGNodeStyleSetBorder(node, YGEdgeVertical, edges["vertical"].GetFloat());
	if (edges.HasMember("left") && edges["left"].IsNumber()) YGNodeStyleSetBorder(node, YGEdgeLeft, edges["left"].GetFloat());
	if (edges.HasMember("top") && edges["top"].IsNumber()) YGNodeStyleSetBorder(node, YGEdgeTop, edges["top"].GetFloat());
	if (edges.HasMember("right") && edges["right"].IsNumber()) YGNodeStyleSetBorder(node, YGEdgeRight, edges["right"].GetFloat());
	if (edges.HasMember("bottom") && edges["bottom"].IsNumber()) YGNodeStyleSetBorder(node, YGEdgeBottom, edges["bottom"].GetFloat());
}

static void applyPositionEdges(YGNodeRef node, const rapidjson::Value::ConstObject pos) {
	if (pos.HasMember("left")) {
		const rapidjson::Value& v = pos["left"];
		JsonSize s = parseSize(v);
		if (s.unit == JsonSize::Unit::Px)           YGNodeStyleSetPosition(node, YGEdgeLeft, s.value);
		else if (s.unit == JsonSize::Unit::Percent) YGNodeStyleSetPositionPercent(node, YGEdgeLeft, s.value);
	}

	if (pos.HasMember("top")) {
		const rapidjson::Value& v = pos["top"];
		JsonSize s = parseSize(v);
		if (s.unit == JsonSize::Unit::Px)           YGNodeStyleSetPosition(node, YGEdgeTop, s.value);
		else if (s.unit == JsonSize::Unit::Percent) YGNodeStyleSetPositionPercent(node, YGEdgeTop, s.value);
	}

	if (pos.HasMember("right")) {
		const rapidjson::Value& v = pos["right"];
		JsonSize s = parseSize(v);
		if (s.unit == JsonSize::Unit::Px)           YGNodeStyleSetPosition(node, YGEdgeRight, s.value);
		else if (s.unit == JsonSize::Unit::Percent) YGNodeStyleSetPositionPercent(node, YGEdgeRight, s.value);
	}

	if (pos.HasMember("bottom")) {
		const rapidjson::Value& v = pos["bottom"];
		JsonSize s = parseSize(v);
		if (s.unit == JsonSize::Unit::Px)           YGNodeStyleSetPosition(node, YGEdgeBottom, s.value);
		else if (s.unit == JsonSize::Unit::Percent) YGNodeStyleSetPositionPercent(node, YGEdgeBottom, s.value);
	}
}

void loadYogaLayout(YGNodeRef node, const rapidjson::Value::ConstObject json);