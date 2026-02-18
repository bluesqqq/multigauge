#include "TimeColor.h"

float TimeColor::getTime() const {
    float duration = timeline.getEndPosition();

    if (timeline.size() == 0 || duration <= 0.0f) return 0.0f;

    float t = 0; //static_cast<float>(millis()) / 1000.0f; // TODO: PUT TIME HERE!!!

    switch (loopType) {
        case LoopType::Forward:
            t = std::fmod(t, duration);
            break;
        
        case LoopType::Reverse:
            t = duration - std::fmod(t, duration);
            break;

        case LoopType::PingPong:
            t = std::fmod(t, 2.0f * duration);
            if (t > duration) t = 2.0f * duration - t;
            break;
    }

    return t;
}

const ColorTimeline *TimeColor::getTimeline() const { return &timeline; }

TimeColor::TimeColor() : timeline(ColorTimeline()), loopType(LoopType::Forward) { }

TimeColor::TimeColor(ColorTimeline timeline, LoopType loopType) : timeline(std::move(timeline)), loopType(loopType) {}

TimeColor::TimeColor(const rapidjson::Value::ConstObject json) {
    setValue(json, "timeline", timeline);
    if (json.HasMember("loopType") && json["loopType"].IsString()) {
        const char* loopStr = json["loopType"].GetString();
        if (strcmp(loopStr, "forward") == 0) loopType = LoopType::Forward;
        else if (strcmp(loopStr, "reverse") == 0) loopType = LoopType::Reverse;
        else if (strcmp(loopStr, "pingpong") == 0) loopType = LoopType::PingPong;
        else loopType = LoopType::Forward;
    } else {
        loopType = LoopType::Forward;
    }
}

TimeColor::TimeColor(const TimeColor &other) : timeline(other.timeline), loopType(other.loopType) {}

TimeColor &TimeColor::operator=(const TimeColor &other) {
    if (this != &other) {
        this->timeline = other.timeline;
        this->loopType = other.loopType;
    }
    return *this;
}

std::unique_ptr<Color> TimeColor::blended(rgba color, float alpha) const { return std::make_unique<TimeColor>(timeline.blended(color, alpha)); }

std::unique_ptr<Color> TimeColor::blended(const Color &other, float alpha) const {
    switch(other.getType()) {
        case (Type::Static):
            return blended(other.getColor(), alpha);

        case (Type::Time):
        case (Type::Value): {
            const ColorTimeline* timeline = other.getTimeline();
            if (timeline != nullptr) return std::make_unique<TimeColor>(this->timeline.blended(*timeline, alpha), this->loopType);
        }
    }

    //return std::make_unique<TimeColor>(this->timeline, this->loopType);
    return std::make_unique<TimeColor>();
}

std::unique_ptr<Color> TimeColor::clone() const { return std::make_unique<TimeColor>(*this); }

rgba TimeColor::getColor() const { return timeline.getColor(getTime()); }

Color::Type TimeColor::getType() const { return Type::Time; }
