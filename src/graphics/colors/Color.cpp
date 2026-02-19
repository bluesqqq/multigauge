#include <multigauge/graphics/colors/Color.h>

#include <multigauge/graphics/colors/StaticColor.h>
#include <multigauge/graphics/colors/ValueColor.h>
#include <multigauge/graphics/colors/TimeColor.h>

const ColorTimeline* Color::getTimeline() const { return nullptr; }

OwnedColor Color::fromJson(const rapidjson::Value& json) {
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

Paint::Paint() : fill(nullptr), stroke(nullptr) {}

Paint::Paint(OwnedColor fill, OwnedColor stroke, float thickness) : fill(std::move(fill)), stroke(std::move(stroke)), thickness(thickness) {}

Paint::Paint(const rapidjson::Value::ConstObject json)
    : fill((json.HasMember("fill") ? Color::fromJson(json["fill"]) : nullptr)),
      stroke((json.HasMember("stroke") ? Color::fromJson(json["stroke"]) : nullptr)),
      thickness((json.HasMember("thickness") && json["thickness"].IsNumber()) ? json["thickness"].GetFloat() :  1.0f) {}

Paint Paint::blended(rgba c, float alpha) const { return Paint((fill) ? fill->blended(c, alpha) : nullptr, (stroke) ? stroke->blended(c, alpha) : nullptr, thickness); }

Paint Paint::blended(const Color &c, float alpha) const { return Paint((fill) ? fill->blended(c, alpha) : nullptr, (stroke) ? stroke->blended(c, alpha) : nullptr, thickness); }

Paint Paint::blended(const Paint &other, float alpha) const {
    OwnedColor outFill;
    OwnedColor outStroke;

    if (fill && other.fill) outFill = fill->blended(*other.fill, alpha);
    else if (fill) outFill = fill->clone();
    else if (other.fill) outFill = other.fill->clone();

    if (stroke && other.stroke) outStroke = stroke->blended(*other.stroke, alpha);
    else if (stroke) outStroke = stroke->clone();
    else if (other.stroke) outStroke = other.stroke->clone();

    return Paint(std::move(outFill), std::move(outStroke), thickness);
}
