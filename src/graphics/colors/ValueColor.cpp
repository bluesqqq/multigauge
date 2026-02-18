#include "ValueColor.h"

const ColorTimeline *ValueColor::getTimeline() const { return &timeline; }

ValueColor::ValueColor(Value *value) : timeline(ColorTimeline()), value(value) {}

ValueColor::ValueColor(Value *value, ColorTimeline timeline) : timeline(std::move(timeline)), value(value) { }

ValueColor::ValueColor(const rapidjson::Value::ConstObject json) {
    setValue(json, "timeline", timeline);
    if (json.HasMember("id") && json["id"].IsString()) value = Value::find(json["id"].GetString());
}

ValueColor::ValueColor(const ValueColor &other) : timeline(other.timeline), value(other.value) {}

ValueColor &ValueColor::operator=(const ValueColor &other) {
    if (this != &other) {
        this->timeline = other.timeline;
        this->value = other.value;
    }
    return *this;
}

std::unique_ptr<Color> ValueColor::blended(rgba color, float alpha) const { return std::make_unique<ValueColor>(this->value, timeline.blended(color, alpha)); }

std::unique_ptr<Color> ValueColor::blended(const Color &other, float alpha) const{
    switch(other.getType()) {
        case (Type::Static):
            return blended(other.getColor(), alpha);

        case (Type::Time):
        case (Type::Value): {
            const ColorTimeline* timeline = other.getTimeline();
            if (timeline != nullptr) return std::make_unique<ValueColor>(this->value, this->timeline.blended(*timeline, alpha));
        }
    }

    //return std::make_unique<ValueColor>(this->value, this->timeline);
    return std::make_unique<ValueColor>(this->value);
}

std::unique_ptr<Color> ValueColor::clone() const { return std::make_unique<ValueColor>(*this); }

rgba ValueColor::getColor() const { return timeline.getColor((value != nullptr) ? value->getValueBase() : 0.0f); }

Color::Type ValueColor::getType() const { return Type::Value; }
