#include <multigauge/gauge/elements/FrameElement.h>

#include <multigauge/graphics/Graphics.h>

#include <cmath>

namespace mg::gauge {

void FrameElement::draw(::mg::graphics::Graphics& graphics, const ::mg::Rect<float>& bounds) const {
    graphics.setFill(fill_);
    graphics.drawRect(static_cast<int>(std::lround(bounds.x)),
                      static_cast<int>(std::lround(bounds.y)),
                      static_cast<int>(std::lround(bounds.width)),
                      static_cast<int>(std::lround(bounds.height)));
}

} // namespace mg::gauge
