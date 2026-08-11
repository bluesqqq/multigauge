#include <algorithm>
#include <multigauge/gauge/elements/circular/CircularScale.h>
#include <multigauge/graphics/Graphics.h>

namespace mg::gauge {
void CircularScale::draw(::mg::graphics::Graphics& g, const ::mg::Rect<float>& b) const {
    const auto value = resolvedValueView();
    ticks_.drawCircular(g,
                        {b.x + b.width * .5f, b.y + b.height * .5f},
                        std::min(b.width, b.height) * .5f * radius_,
                        resolvedStartAngle(),
                        resolvedEndAngle(),
                        value.minimumBase(),
                        value.maximumBase());
}

void CircularScale::update(std::chrono::microseconds) {
    ticks_.setValueView(resolvedValueView().valueBase());
}
} // namespace mg::gauge
