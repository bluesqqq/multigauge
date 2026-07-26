#include <multigauge/graphics/colors/ColorResolver.h>

#include <multigauge/graphics/colors/Color.h>

namespace mg::graphics {

ColorResolver::ColorResolver(std::size_t capacity) {
    cache_.reserve(capacity);
}

bool ColorResolver::Frame::valid() const noexcept {
    return resolver_ && resolver_->frame_ && generation_ == resolver_->generation_;
}

const ColorFrame* ColorResolver::Frame::data() const noexcept {
    return valid() ? resolver_->frame_ : nullptr;
}

rgba ColorResolver::Frame::resolve(const Color& color) const noexcept {
    return resolver_ ? resolver_->resolve(*this, color) : rgba{0, 0, 0, 0};
}

ColorResolver::Frame ColorResolver::beginFrame(const ColorFrame& frame) noexcept {
    frame_ = &frame;
    ++generation_;
    if (generation_ == 0) {
        generation_ = 1;
        for (CacheEntry& entry : cache_) entry.generation = 0;
    }
    return Frame(this, generation_);
}

rgba ColorResolver::resolve(const Frame& frame, const Color& color) noexcept {
    if (!frame.valid()) return rgba{0, 0, 0, 0};

    std::size_t index = cache_.size();
    std::size_t reusable = cache_.size();
    for (std::size_t candidateIndex = 0; candidateIndex < cache_.size(); ++candidateIndex) {
        CacheEntry& candidate = cache_[candidateIndex];
        if (candidate.id == color.id() && candidate.color == &color) {
            index = candidateIndex;
            break;
        }
        if (reusable == cache_.size() && candidate.generation != generation_) reusable = candidateIndex;
    }
    if (index == cache_.size()) {
        if (reusable != cache_.size()) {
            index = reusable;
            cache_[index] = CacheEntry{.id = color.id(), .color = &color};
        } else if (cache_.size() < cache_.capacity()) {
            cache_.push_back(CacheEntry{.id = color.id(), .color = &color});
            index = cache_.size() - 1;
        } else {
            // This is a configuration error, not an allocation opportunity in
            // a noexcept render path. The fallback remains deterministic.
            return rgba{0, 0, 0, 0};
        }
    }

    if (cache_[index].generation == generation_) {
        // A resolving entry is a cycle. Transparent black is the deterministic
        // fallback for malformed/future shared definition graphs.
        return cache_[index].state == State::Resolved ? cache_[index].value : rgba{0, 0, 0, 0};
    }

    cache_[index].generation = generation_;
    cache_[index].state = State::Resolving;
    const rgba resolved = color.resolveUncached(frame);
    cache_[index].value = resolved;
    cache_[index].state = State::Resolved;
    return resolved;
}

} // namespace mg::graphics
