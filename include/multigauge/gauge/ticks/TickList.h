#pragma once

#include <multigauge/gauge/ticks/RootTick.h>
#include <multigauge/gauge/ticks/SubTick.h>
#include <multigauge/gauge/ticks/TickStyle.h>
#include <multigauge/graphics/colors/ColorTimeline.h>

#include <multigauge/graphics/Graphics.h>

#include <vector>
#include <optional>
#include <utility>
#include <cstdint>
#include <multigauge/constants.h>
#include <cmath>
#include <algorithm>

class TickList : public Editable {
    MG_EDITOR_NAME("Tick List")
    
    private:
        RootTick root;
        std::vector<SubTick> subs;

        LengthAlignment align = LengthAlignment::OUTER;

        float offset = 0;

        float displayValue = 0;
        
        float lengthFactor = 2;
        float thicknessFactor = 0;
        float textSizeFactor = 1;

        float leftHighlightBase = 0.0f;
        float leftHighlightFactor = 1.0f;
        float leftHighlightDistance = 1000.0f;

        float rightHighlightBase = 0.0f;
        float rightHighlightFactor = 1.0f;
        float rightHighlightDistance = 1000;

        MG_EDITABLE_BEGIN()
            MG_EDITABLE_PROP(root)
            MG_EDITABLE_PROP(subs)
            MG_EDITABLE_PROP(offset)
        MG_EDITABLE_END()

        float getLength(uint8_t index) const;
        float getThickness(uint8_t index) const;
        const TickStyle& getStyle(uint8_t index) const;
        const PaintTimeline& getColor(uint8_t index) const;
        const std::optional<TickValueStyle>& getValueStyle(uint8_t index) const;

        std::vector<std::vector<float>> getTickPositions(float startValue, float endValue) const {
            auto allPositions = std::vector<std::vector<float>>(subs.size() + 1);

            std::vector<float> positions = root.getPositions(startValue, endValue, 0);
            const float interval = root.getInterval(startValue, endValue);

            for (const float& position : positions) {
                if (inRange(position, 0.f, 1.f)) allPositions[0].push_back(position);

                getSeqTickPositions(position, position + interval, 0, allPositions);
            }

            return allPositions;
        }

        void getSeqTickPositions(float startTickPosition, float endTickPosition, uint8_t index, std::vector<std::vector<float>>& out) const {
            if (index >= subs.size()) return;

            const SubTick& tick = subs[index];

            std::vector<float> positions = tick.getPositions(startTickPosition, endTickPosition);
            const float interval = tick.getInterval(startTickPosition, endTickPosition);

            for (int i = 0; i < positions.size(); ++i) {
                const float position = positions[i];

                if (inRange(position, 0.f, 1.f)) out[index + 1].push_back(position);
                getSeqTickPositions(position, position + interval, index + 1, out);
            }
        }

        void drawLineTick(Graphics& g, Line<float> line, float thickness, Paint& paint) const {
            g.setPaint(paint);
            g.drawLine(line.toInt(), thickness);
        }

        void drawCircleTick(Graphics& g, Line<float> line, float thickness, Paint& paint) const {
            
        }

        void drawTriangleTick(Graphics& g, Line<float> line, float thickness, Paint& paint) const {
            
        }

        void drawCircularTick(Graphics& g, uint8_t index, Point<float> pos, float radius, float position, float angle, float value) const;

        float getHighlightFactor(float value) const {
            const float delta = (value - displayValue);

            if (delta < 0.f) {
                if (leftHighlightDistance <= 0.f || delta <= -leftHighlightDistance) return leftHighlightBase;
                
                return lerp(leftHighlightBase, leftHighlightFactor, 1.f - std::min(1.f, (float)abs(delta) / leftHighlightDistance));
            } else if (delta > 0.f) {
                if (rightHighlightDistance <= 0.f || delta >= rightHighlightDistance) return rightHighlightBase;

                return lerp(rightHighlightBase, rightHighlightFactor, 1.f - std::min(1.f, (float)abs(delta) / rightHighlightDistance));
            }

            return std::max(leftHighlightFactor, rightHighlightFactor);
        }

    public:
        TickList() = default;

        void drawCircular(Graphics& g, Point<float> pos, float radius, float startAngle, float endAngle, float startValue, float endValue) const {
            auto tickPositions = getTickPositions(startValue, endValue);

            float rStartAngle = startAngle * (PI / 180.0); 
            float rEndAngle = endAngle * (PI / 180.0);

            for (int i = tickPositions.size() - 1; i >= 0; --i) {
                for (float& position : tickPositions[i]) {
                    float angle = lerp(rStartAngle, rEndAngle, position);
                    float value = lerp(startValue, endValue, position);

                    drawCircularTick(g, i, pos, radius, position, angle, value);
                }
            }
        }

        void setDisplayValue(float newValue) { displayValue = newValue; }
};