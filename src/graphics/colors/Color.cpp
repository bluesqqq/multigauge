#include <multigauge/graphics/colors/Color.h>

#include <multigauge/graphics/colors/StaticColor.h>
#include <multigauge/graphics/colors/ValueColor.h>
#include <multigauge/graphics/colors/TimeColor.h>
#include <multigauge/graphics/colors/UserColor.h>

const ColorTimeline* Color::getTimeline() const { return nullptr; }

DECODE_IMPL(OwnedColor) {
    if (v.IsNull()) {
        out = nullptr;
        return true;
    }

    if (!v.IsObject()) return false;

    const auto obj = v.GetObject();

    const char* type = nullptr;
    if (auto it = obj.FindMember("type"); it != obj.MemberEnd() && it->value.IsString()) type = it->value.GetString();

    if (!type) out = std::make_unique<StaticColor>();
    else if (std::strcmp(type, "value") == 0) out = std::make_unique<ValueColor>();
    else if (std::strcmp(type, "time") == 0) out = std::make_unique<TimeColor>();
    else if (std::strcmp(type, "user") == 0) out = std::make_unique<UserColor>();
    else out = std::make_unique<StaticColor>();

    out->loadProperties(obj);

    return true;
}

ENCODE_IMPL(OwnedColor) {
    if (!v) { 
        out.SetNull(); 
        return true; 
    }

    v->saveProperties(out, a);

    return true;
}

//----------[ FILL STROKE ]----------//

Paint::Paint() : fill(nullptr), stroke(nullptr) {}

Paint::Paint(OwnedColor fill, OwnedColor stroke, float thickness) : fill(std::move(fill)), stroke(std::move(stroke)), thickness(thickness) {}

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
