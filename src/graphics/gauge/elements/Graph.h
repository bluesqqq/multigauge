#include "graphics/gauge/Element.h"

#include "graphics/DisplayValue.h"

struct TimeValue {
    float value;
    unsigned long time;
};

class Graph : public Element {
    private:
        float seconds = 1;
        int bufferMilliseconds = 0;
        
        int samplePx = 10;

        std::unique_ptr<Color> backgroundColor;
        std::unique_ptr<Color> secondsColor;
        std::unique_ptr<Color> graphColor;
        std::unique_ptr<Color> borderColor;

        DisplayValue value;
        std::vector<TimeValue> valueMemory = {};

        enum Style { Line, Bars, Dots };

        Style style = Bars;

        unsigned long timeAtX(int x, int left, int width, unsigned long currentTime, float windowMs) const {
            float t01 = float(x - left) / float(std::max(1, width)); // 0..1
            return currentTime - (unsigned long)std::lround((1.0f - t01) * windowMs) - (unsigned long)bufferMilliseconds;
        }

        float valueAtTime(unsigned long time) const {
            if (valueMemory.empty()) return 0.0f;
            if (valueMemory.size() == 1) return valueMemory[0].value;

            const TimeValue& newest = valueMemory.front();
            const TimeValue& oldest = valueMemory.back();

            if (time >= newest.time) return newest.value;

            if (time <= oldest.time) return oldest.value;

            for (size_t i = 0; i + 1 < valueMemory.size(); ++i) {
                const TimeValue& a = valueMemory[i];     // newer
                const TimeValue& b = valueMemory[i + 1]; // older

                if (a.time >= time && time >= b.time) {
                    const unsigned long dt = a.time - b.time;
                    if (dt == 0) return a.value;

                    const float t = float(a.time - time) / float(dt);
                    return lerp(a.value, b.value, t);
                }
            }

            return oldest.value;
        }

    public:
        Graph(Element* parent, const rapidjson::Value::ConstObject json) : Element(parent, json) {
            if (!json.HasMember("props") || !json["props"].IsObject()) return;
            const rapidjson::Value::ConstObject props = json["props"].GetObject();

            setFloat(props, "seconds", seconds);

            setColor(props, "backgroundColor", backgroundColor);
            setColor(props, "secondsColor", secondsColor);
            setColor(props, "graphColor", graphColor);
            setColor(props, "borderColor", borderColor);

            setObj(props, "value", value);
        }

        void draw(Graphics& g) const override {
            const auto b = getBounds();

            const float minimum = value.getMinimumBase();
            const float maximum = value.getMaximumBase();

            const unsigned long currentTime = 0; //millis(); // TODO HANGE TO ABSTRACT TIME CLASS
            const float secondLength = b.width / seconds; // length of 1 second

            if (backgroundColor) {
                g.setFill(*backgroundColor);
                g.fillRect(b.toInt());
            }

            if (graphColor) {

                size_t valueMemorySize = valueMemory.size();

                // X position of the right line
                int lineX = b.getRight();

                g.setFill(*graphColor);
                g.setStroke(*graphColor);

                switch (style) {
                    case Line: {
                        const float windowMs = seconds * 1000.0f;

                        bool havePrev = false;
                        int prevX = 0, prevY = 0;

                        for (int x = b.getLeft(); x <= b.getRight(); x += samplePx) {
                            const unsigned long t = timeAtX(x, b.getLeft(), b.width, currentTime, windowMs);
                            const float v = valueAtTime(t);
                            const int y = mapf(v, minimum, maximum, b.getBottom(), b.getTop());

                            if (!havePrev) {
                                havePrev = true;
                                prevX = x; prevY = y;
                                continue;
                            }

                            // simple clip: only draw if at least one endpoint is inside vertical bounds
                            // (optional, since x is already in-bounds)
                            if ((y >= b.getTop() && y <= b.getBottom()) ||
                                (prevY >= b.getTop() && prevY <= b.getBottom())) {
                                g.strokeLine(prevX, prevY, x, y);
                            }

                            prevX = x; prevY = y;
                        }
                        break;
                    }

                    case Bars: {
                        const float windowMs = seconds * 1000.0f;

                        for (int x = b.getLeft(); x <= b.getRight(); x += samplePx) {
                            const unsigned long t = timeAtX(x, b.getLeft(), b.width, currentTime, windowMs);
                            const float v = valueAtTime(t);
                            const int yVal = mapf(v, minimum, maximum, b.getBottom(), b.getTop());

                            const int barH = b.getBottom() - yVal;
                            if (barH <= 0) continue;

                            const int w = std::max(1, samplePx);
                            g.fillRect({x, yVal, w, barH});
                        }
                        break;
                    }

                    case Dots: {
                        const float windowMs = seconds * 1000.0f;

                        for (int x = b.getLeft(); x <= b.getRight(); x += samplePx) {
                            const unsigned long t = timeAtX(x, b.getLeft(), b.width, currentTime, windowMs);
                            const float v = valueAtTime(t);
                            const int y = mapf(v, minimum, maximum, b.getBottom(), b.getTop());

                            if (y >= b.getTop() && y <= b.getBottom()) {
                                g.fillPixel(x, y);
                            }
                        }
                        break;
                    }

                }
            }

            if (secondsColor) {
                g.setStroke(*secondsColor);

                const long adj = (long)currentTime - (long)bufferMilliseconds;
                const float frac = float(((adj % 1000) + 1000) % 1000) / 1000.0f;

                const int xNow = b.getRight() - (int)std::lround(frac * secondLength);

                const int n = (int)std::ceil(seconds) + 2;

                for (int i = 0; i < n; ++i) {
                    const int x = xNow - (int)std::lround(i * secondLength);
                    if (x < b.getLeft()) break;
                    g.strokeLine(x, b.getBottom(), x, b.getBottom() + 3);
                }
            }

            g.setTextColor(rgb(255, 255, 255));
            std::string s = std::string(value.getName()) + " : " + value.getValueString(true);
            g.drawText(s, b.getTopLeft().toInt().translated(2, 2), Anchor::TopLeft);

            if (borderColor) {
                g.setStroke(*borderColor);
                g.strokeRect(b.toInt()); 
            }
        }

        void update(int deltaTime) override {
            unsigned long currentTime = 0; //millis(); // TODO: CHANGE TO ABSTRACT TIME CLASS

            valueMemory.insert(valueMemory.begin(), {value.getValueBase(), currentTime});

            // remove expired values
            auto it = std::find_if(valueMemory.begin(), valueMemory.end(), [currentTime, this](const TimeValue& tv) {
                return (currentTime - tv.time) > (seconds * 1000 + bufferMilliseconds);
            });

            if (it != valueMemory.end() && (it + 1) != valueMemory.end()) {
                valueMemory.erase(it + 1, valueMemory.end());
            }
        }
};

REGISTER_ELEMENT_TYPE("graph", Graph);