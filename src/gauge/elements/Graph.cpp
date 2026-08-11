#include <multigauge/gauge/elements/Graph.h>
#include <multigauge/graphics/Graphics.h>
#include <multigauge/utils/Math.h>

#include <algorithm>
#include <cmath>

namespace mg::gauge {
std::uint64_t Graph::timeAtX(
    int x,
    int left,
    int width,
    std::uint64_t currentTime,
    float windowMilliseconds
) const {
    const float position = static_cast<float>(x - left) / static_cast<float>(std::max(1, width));
    const auto offset = static_cast<std::uint64_t>(std::lround(
        (1.0f - position) * windowMilliseconds
    )) + static_cast<std::uint64_t>(std::max(0, bufferMilliseconds_));
    return currentTime > offset ? currentTime - offset : 0;
}

float Graph::valueAtTime(std::uint64_t time) const {
    if (valueMemory_.empty()) return 0.0f;
    if (valueMemory_.size() == 1) return valueMemory_.front().value;

    const TimeValue& newest = valueMemory_.front();
    const TimeValue& oldest = valueMemory_.back();
    if (time >= newest.time) return newest.value;
    if (time <= oldest.time) return oldest.value;

    for (std::size_t index = 0; index + 1 < valueMemory_.size(); ++index) {
        const TimeValue& newer = valueMemory_[index];
        const TimeValue& older = valueMemory_[index + 1];
        if (newer.time < time || time < older.time) continue;

        const std::uint64_t elapsed = newer.time - older.time;
        if (elapsed == 0) return newer.value;
        const float position = static_cast<float>(newer.time - time) /
                               static_cast<float>(elapsed);
        return ::mg::utils::lerp(newer.value, older.value, position);
    }
    return oldest.value;
}

void Graph::draw(::mg::graphics::Graphics& g, const ::mg::Rect<float>& bounds) const {
    const auto b = bounds.toInt();
    if (seconds_ <= 0.0f || samplePx_ <= 0) return;

    const float minimum = value_.minimumBase();
    const float maximum = value_.maximumBase();
    const auto currentTime = static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_).count()
    );
    const float windowMilliseconds = seconds_ * 1000.0f;

    if (backgroundColor_) {
        g.setPaint(backgroundColor_.get());
        g.drawRect(b);
    }

    if (graphColor_) {
        g.setPaint(graphColor_.get());
        if (style_ == Style::Line) {
            bool havePrevious = false;
            int previousX = 0;
            int previousY = 0;
            for (int x = b.getLeft(); x <= b.getRight(); x += samplePx_) {
                const float sample = valueAtTime(timeAtX(
                    x, b.getLeft(), b.width, currentTime, windowMilliseconds
                ));
                const int y = ::mg::utils::mapf(sample, minimum, maximum, b.getBottom(), b.getTop());
                if (havePrevious) g.drawLine(previousX, previousY, x, y);
                previousX = x;
                previousY = y;
                havePrevious = true;
            }
        } else {
            for (int x = b.getLeft(); x <= b.getRight(); x += samplePx_) {
                const float sample = valueAtTime(timeAtX(
                    x, b.getLeft(), b.width, currentTime, windowMilliseconds
                ));
                const int y = ::mg::utils::mapf(sample, minimum, maximum, b.getBottom(), b.getTop());
                if (style_ == Style::Dots) {
                    if (y >= b.getTop() && y <= b.getBottom()) g.drawPixel(x, y);
                } else if (y <= b.getBottom()) {
                    g.drawRect({x, y, std::max(1, samplePx_), b.getBottom() - y});
                }
            }
        }
    }

    if (secondsColor_) {
        g.setPaint(secondsColor_.get());
        const float secondWidth = b.width / seconds_;
        const long adjusted = static_cast<long>(currentTime) - bufferMilliseconds_;
        const float fraction = static_cast<float>(((adjusted % 1000) + 1000) % 1000) / 1000.0f;
        const int currentX = b.getRight() - static_cast<int>(std::lround(fraction * secondWidth));
        for (int index = 0; index < static_cast<int>(std::ceil(seconds_)) + 2; ++index) {
            const int x = currentX - static_cast<int>(std::lround(index * secondWidth));
            if (x < b.getLeft()) break;
            g.drawLine(x, b.getBottom(), x, b.getBottom() + 3);
        }
    }

    if (borderColor_) {
        g.setPaint(nullptr, borderColor_.get());
        g.drawRect(b);
    }
}

void Graph::update(std::chrono::microseconds delta) {
    elapsed_ += delta;
    const auto currentTime = static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_).count()
    );
    valueMemory_.insert(
        valueMemory_.begin(),
        {value_.valueBase(), currentTime}
    );
    const auto expired = std::find_if(valueMemory_.begin(), valueMemory_.end(),
        [this, currentTime](const TimeValue& sample) {
            return currentTime - sample.time > seconds_ * 1000.0f + bufferMilliseconds_;
        });
    if (expired != valueMemory_.end() && std::next(expired) != valueMemory_.end()) {
        valueMemory_.erase(std::next(expired), valueMemory_.end());
    }
}
} // namespace mg::gauge
