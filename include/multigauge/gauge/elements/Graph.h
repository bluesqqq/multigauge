#pragma once

#include <chrono>
#include <cstdint>
#include <multigauge/gauge/Element.h>
#include <multigauge/graphics/colors/Color.h>
#include <multigauge/value/ValueView.h>
#include <vector>

namespace mg::gauge {
/// @brief Draws a scrolling graph element.
class Graph final : public Element {
    MG_EDITOR_NAME("Graph")
    MG_TYPE_ID("graph")
public:
    /// @brief Creates a graph element.
    Graph() : Element(staticTypeId()) {}

    /// @brief Draws the graph in its layout bounds.
    void draw(::mg::graphics::Graphics&, const ::mg::Rect<float>&) const override;

    /// @brief Advances the graph state.
    void update(std::chrono::microseconds) override;

private:
    float seconds_ = 1.0f;
    int bufferMilliseconds_ = 0;
    int samplePx_ = 10;
    ::mg::graphics::OwnedColor backgroundColor_, secondsColor_, graphColor_, borderColor_;
    ::mg::ValueView value_;

    struct TimeValue {
        float value;
        std::uint64_t time;
    };

    enum class Style { Line, Bars, Dots };

    std::vector<TimeValue> valueMemory_;
    std::chrono::microseconds elapsed_{};
    Style style_ = Style::Bars;

    std::uint64_t timeAtX(
        int x,
        int left,
        int width,
        std::uint64_t currentTime,
        float windowMilliseconds
    ) const;
    float valueAtTime(std::uint64_t time) const;

    MG_PROPS_PARENT(Element)
    MG_PROPS_BEGIN()
    MG_PROP(seconds_, "seconds")
    MG_PROP(backgroundColor_, "bgColor")
    MG_PROP(secondsColor_, "secondsColor")
    MG_PROP(graphColor_, "graphColor")
    MG_PROP(borderColor_, "borderColor")
    MG_PROP(value_, "value")
    MG_PROPS_END()
};
} // namespace mg::gauge
