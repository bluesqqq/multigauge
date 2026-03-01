#include <multigauge/graphics/colors/ValueColor.h>

const ColorTimeline *ValueColor::getTimeline() const { return &timeline; }

ValueColor::ValueColor(Value *value, ColorTimeline timeline) : timeline(std::move(timeline)), value(value) { }

ValueColor::ValueColor(const ValueColor &other) : timeline(other.timeline), value(other.value) {}

ValueColor &ValueColor::operator=(const ValueColor &other) {
    if (this != &other) {
        this->timeline = other.timeline;
        this->value = other.value;
        this->id = other.id;
    }
    return *this;
}

OwnedColor ValueColor::blended(rgba color, float alpha) const { return std::make_unique<ValueColor>(this->value, timeline.blended(color, alpha)); }

OwnedColor ValueColor::blended(const Color &other, float alpha) const{
    switch(other.getType()) {
        case (Type::Static):
            return blended(other.getColor(), alpha);

        case (Type::Time):
        case (Type::Value): {
            const ColorTimeline* timeline = other.getTimeline();
            if (timeline != nullptr) return std::make_unique<ValueColor>(this->value, this->timeline.blended(*timeline, alpha));
        }
    }

    return std::make_unique<ValueColor>(this->value, this->timeline);
}

OwnedColor ValueColor::clone() const { return std::make_unique<ValueColor>(*this); }

rgba ValueColor::getColor() const { return value ? timeline.getColor((value != nullptr) ? value->getValueBase() : 0.0f) : rgba(); }

Color::Type ValueColor::getType() const { return Type::Value; }

// ----------[ JSON helpers ]----------