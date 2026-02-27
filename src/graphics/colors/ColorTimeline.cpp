#include <multigauge/graphics/colors/ColorTimeline.h>
#include <multigauge/graphics/colors/StaticColor.h>
#include <multigauge/graphics/colors/TimeColor.h>
#include <multigauge/graphics/colors/ValueColor.h>
#include <multigauge/utils.h>
#include <set>
#include <algorithm>

ColorKeyframe::ColorKeyframe() { }

ColorKeyframe::ColorKeyframe(OwnedColor color, float position) : color(std::move(color)), position(position) { }

ColorKeyframe::ColorKeyframe(const rapidjson::Value::ConstObject json)
    : position(json.HasMember("position") && json["position"].IsNumber() ? json["position"].GetFloat() : 0.0f),
      color((json.HasMember("color")) ? Color::fromJson(json["color"]) : std::make_unique<StaticColor>()) {}

ColorKeyframe::ColorKeyframe(const ColorKeyframe &other) : position(other.position), color(other.color ? other.color->clone() : nullptr) {}

ColorKeyframe &ColorKeyframe::operator=(const ColorKeyframe &other) {
    if (this != &other) {
        this->position = other.position;
        this->color = other.color ? other.color->clone() : nullptr;
    }
    return *this;
}

size_t ColorTimeline::getKeyframeIndexAtPosition(float position) const {
    const size_t n = keyframes.size();
    if (n <= 1) return 0;
    if (position < keyframes[1].position) return 0;
    if (position >= keyframes.back().position) return n - 1;

    size_t left = 1;
    size_t right = n - 1;
    while (left < right) {
        size_t middle = (left + right) / 2;
        if (keyframes[middle].position < position) 
            left = middle + 1;
        else right = middle;
    }

    return left - 1;
}

ColorTimeline::ColorTimeline() {}

ColorTimeline::ColorTimeline(rgba color) { addKeyframe(color, 0.0f); }

ColorTimeline::ColorTimeline(const rapidjson::Value &json) {
    if (json.IsString() || json.IsObject()) {
        addKeyframe(Color::fromJson(json), 0.0f);
        return;
    }

    if (json.IsArray()) {
        for (const auto& keyframe : json.GetArray()) {
            if (keyframe.IsObject()) keyframes.emplace_back(ColorKeyframe(keyframe.GetObject()));
            else LOG_WARN("ColorTimeline", "Bad entry type: %s", rjTypeName(keyframe));
        }
        return;
    }

    LOG_WARN("ColorTimeline", "Bad timeline type: %s", rjTypeName(json));
}

ColorTimeline::ColorTimeline(const ColorTimeline &other) {
    keyframes.reserve(other.keyframes.size());
    for (const auto& keyframe : other.keyframes)
        this->keyframes.emplace_back(keyframe);
}

ColorTimeline &ColorTimeline::operator=(const ColorTimeline &other) {
    if (this == &other) return *this;

    std::vector<ColorKeyframe> tmp;
    tmp.reserve(other.keyframes.size());
    for (const auto& keyframe : other.keyframes) tmp.emplace_back(keyframe);
    keyframes = std::move(tmp);
    return *this;
}

ColorTimeline ColorTimeline::blended(rgba color, float alpha) const {
    ColorTimeline newTimeline;

    // TODO: addKeyframe is inefficient in this case. emplace_back directly into the keyframes would be better
    for (const auto& keyframe : keyframes)
        newTimeline.addKeyframe(ColorKeyframe(keyframe.color->blended(color, alpha), keyframe.position));

    return newTimeline;
}

ColorTimeline ColorTimeline::blended(const Color &other, float alpha) const {
    const ColorTimeline* tl = other.getTimeline();

    if (const ColorTimeline* tl = other.getTimeline()) return blended(*tl, alpha);

    return blended(other.getColor(), alpha);
}

ColorTimeline ColorTimeline::blended(const ColorTimeline &other, float alpha) const {
    ColorTimeline newTimeline;

    std::set<float> positions;
    for (const auto& keyframe : this->keyframes) positions.insert(keyframe.position);
    for (const auto& keyframe : other.keyframes) positions.insert(keyframe.position);

    // TODO: this only creates static colors, figure out a way to make it work for all types
    for (float position : positions) {
        rgba colorA = this->getColor(position);
        rgba colorB = other.getColor(position);
        rgba blended = colorA.blended(colorB, alpha);

        newTimeline.addKeyframe(blended, position);
    }

    return newTimeline;
}

void ColorTimeline::addKeyframe(rgba color, float position) { addKeyframe(ColorKeyframe(std::make_unique<StaticColor>(color), position)); }

void ColorTimeline::addKeyframe(OwnedColor color, float position) { addKeyframe(ColorKeyframe(std::move(color), position)); }

void ColorTimeline::addKeyframe(ColorKeyframe keyframe) {
    auto it = keyframes.begin();
    while(it != keyframes.end() && it->position < keyframe.position) ++it;
    keyframes.insert(it, std::move(keyframe));
}

void ColorTimeline::clear() { keyframes.clear(); }

size_t ColorTimeline::size() const { return keyframes.size(); }

bool ColorTimeline::empty() const { return keyframes.empty(); }

rgba ColorTimeline::getColor(float position) const {
    if (keyframes.empty()) return DEFAULT_COLOR;

    size_t index = getKeyframeIndexAtPosition(position);

    if (index >= keyframes.size() - 1) return keyframes.back().color->getColor();

    float normalized = (position - keyframes[index].position) / (keyframes[index + 1].position - keyframes[index].position);

    return keyframes[index].color->getColor().blended(keyframes[index + 1].color->getColor(), normalized);
}

float ColorTimeline::getStartPosition() const { return !keyframes.empty() ? keyframes.front().position : 0.0f; }

float ColorTimeline::getEndPosition() const { return !keyframes.empty() ? keyframes.back().position : 0.0f; }

std::vector<float> ColorTimeline::getPositions() const {
    std::vector<float> positions;

    for (const auto& keyframe : keyframes) positions.push_back(keyframe.position);

    return positions;
}

std::vector<float> ColorTimeline::getPositionsMapped(float start, float end) const {
    std::vector<float> positions;

    float startPosition = getStartPosition();
    float endPosition = getEndPosition();

    for (const auto& keyframe : keyframes) positions.push_back(mapf(keyframe.position, startPosition, endPosition, start, end));

    return positions;
}

std::vector<rgba> ColorTimeline::sample(float startPosition, float endPosition, size_t numSamples) {
    std::vector<rgba> result;
    if (keyframes.empty() || numSamples == 0) return result;
    result.reserve(numSamples);

    float step = (endPosition - startPosition) / (numSamples - 1);
    size_t startIndex = 0;
    size_t endIndex = (keyframes.size() > 1) ? 1 : 0;

    rgba startColor = keyframes[startIndex].color->getColor();
    rgba endColor   = keyframes[endIndex].color->getColor();

    for (size_t sample = 0; sample < numSamples; ++sample) {
        float position = startPosition + step * sample;

        while (endIndex < keyframes.size() && position > keyframes[endIndex].position) {
            startIndex = endIndex;
            startColor = endColor;
            endIndex++;
            endColor = keyframes[std::min(endIndex, keyframes.size() - 1)].color->getColor();
        }

        endIndex = std::min(endIndex, keyframes.size() - 1);

        float t = 0;
        if (startIndex != endIndex) {
            float delta = keyframes[endIndex].position - keyframes[startIndex].position;
            t = (delta != 0) ? (position - keyframes[startIndex].position) / delta : 0;
        }

        result.push_back(startColor.blended(endColor, t));
    }

    return result;
}

PaintTimeline::PaintTimeline(ColorTimeline f, ColorTimeline s, float t) : fill(f), stroke(s), thickness(t) {}

PaintTimeline::PaintTimeline(const rapidjson::Value::ConstObject json) {
    setValue(json, "fill", fill);
    setValue(json, "stroke", stroke);
    setFloat(json, "thickness", thickness);
}

PaintTimeline PaintTimeline::blended(rgba color, float alpha) const {
    return PaintTimeline(fill.blended(color, alpha), stroke.blended(color, alpha), thickness);
}

PaintTimeline PaintTimeline::blended(const Color &color, float alpha) const {
    return PaintTimeline(fill.blended(color, alpha), stroke.blended(color, alpha), thickness);
}

PaintTimeline PaintTimeline::blended(const ColorTimeline &color, float alpha) const {
    return PaintTimeline(fill.blended(color, alpha), stroke.blended(color, alpha), thickness);
}

PaintTimeline PaintTimeline::blended(const PaintTimeline &other, float alpha) const {
    return PaintTimeline(fill.blended(other.fill, alpha), stroke.blended(other.stroke, alpha), (thickness + other.thickness) / 2.0f);
}

#include <multigauge/graphics/colors/StaticColor.h>

Paint PaintTimeline::getPaintAtPosition(float position) const {
    return Paint(
        fill.empty() ? nullptr : std::make_unique<StaticColor>(fill.getColor(position)),
        stroke.empty() ? nullptr : std::make_unique<StaticColor>(stroke.getColor(position)),
        thickness
    );
}
