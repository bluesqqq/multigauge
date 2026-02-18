#include "color.h"

#include "StaticColor.h"
#include "ValueColor.h"
#include "TimeColor.h"

const ColorTimeline* Color::getTimeline() const { return nullptr; }

std::unique_ptr<Color> Color::fromJson(const rapidjson::Value& json) {
    // Parse string as rgba
    if (json.IsString()) { return std::make_unique<StaticColor>(rgba(json.GetString())); }

    if (!json.IsObject()) return std::make_unique<StaticColor>();

    const auto obj = json.GetObject();

    if (!obj.HasMember("type") || !obj["type"].IsString()) return std::make_unique<StaticColor>();

    const char* type = obj["type"].GetString();

    if (strcmp(type, "value")  == 0) return std::make_unique<ValueColor>(obj);
    if (strcmp(type, "time")   == 0) return std::make_unique<TimeColor>(obj);
    if (strcmp(type, "user")   == 0) return std::make_unique<StaticColor>();

    return std::make_unique<StaticColor>();
}

//----------[ FILL STROKE ]----------//

FillStroke::FillStroke() : fill(nullptr), stroke(nullptr) {}

FillStroke::FillStroke(std::unique_ptr<Color> fill, std::unique_ptr<Color> stroke, float thickness) : fill(std::move(fill)), stroke(std::move(stroke)), thickness(thickness) {}

FillStroke::FillStroke(const rapidjson::Value::ConstObject json)
    : fill((json.HasMember("fill") ? Color::fromJson(json["fill"]) : nullptr)),
      stroke((json.HasMember("stroke") ? Color::fromJson(json["stroke"]) : nullptr)),
      thickness((json.HasMember("thickness") && json["thickness"].IsNumber()) ? json["thickness"].GetFloat() :  1.0f) {}

FillStroke FillStroke::blended(rgba c, float alpha) const { return FillStroke((fill) ? fill->blended(c, alpha) : nullptr, (stroke) ? stroke->blended(c, alpha) : nullptr, thickness); }

FillStroke FillStroke::blended(const Color &c, float alpha) const { return FillStroke((fill) ? fill->blended(c, alpha) : nullptr, (stroke) ? stroke->blended(c, alpha) : nullptr, thickness); }

FillStroke FillStroke::blended(const FillStroke &other, float alpha) const {
    std::unique_ptr<Color> outFill;
    std::unique_ptr<Color> outStroke;

    if (fill && other.fill) outFill = fill->blended(*other.fill, alpha);
    else if (fill) outFill = fill->clone();
    else if (other.fill) outFill = other.fill->clone();

    if (stroke && other.stroke) outStroke = stroke->blended(*other.stroke, alpha);
    else if (stroke) outStroke = stroke->clone();
    else if (other.stroke) outStroke = other.stroke->clone();

    return FillStroke(std::move(outFill), std::move(outStroke), thickness);
}
