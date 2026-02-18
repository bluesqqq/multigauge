#pragma once

#include "graphics/gauge/Element.h"
#include "graphics/DisplayValue.h"

class RollOdometer : public Element {
    private:
        int segments = 7;
        int decimals = 2;
        int gap = 2;
        int margin = 2;

        Color* primaryColor;
        Color* secondaryColor;

        DisplayValue value;
        int t = 0;

        float targetVal = 0;

    public:
        RollOdometer(Element* parent, const rapidjson::Value::ConstObject json) : Element(parent, json) { }

        void draw(Graphics& g) const override {
            float val = value.getValue();

            Rect marginless = getBounds().reduced(margin);

            int totalGap = gap * (segments - 1);
            int segmentWidth = (marginless.width - totalGap) / segments;
            int segmentHeight = marginless.height;

            //canvas.setTextColor(primaryColor->getColor());
            //canvas.setTextDatum(MC_DATUM);

            g.setStroke(0xFFFF);
            g.strokeRect(getBounds().toInt());

            for (int i = 0; i < segments; ++i) {
                Rect<float> segment = Rect<float>(marginless.position.x + (segmentWidth + gap) * i, marginless.position.y, segmentWidth, segmentHeight);

                g.setFill(0xFFFF);
                g.fillRect(segment.toInt());
            }
            /*
            int v = floor(val * pow(10, decimals + 1));
            String valueString = String(v);
            valueString.remove(valueString.length() - 1);
            //valueString.replace(".", "");
            
            while (valueString.length() < segments) {
                valueString = "0" + valueString;
            }

            char charArray[segments];
            valueString.toCharArray(charArray, segments+1);
            
            LGFX_Sprite segment = LGFX_Sprite(canvas);
            segment.setColorDepth(8); // 8-bit color depth
            segment.createSprite(totalWidth, segmentHeight);
            segment.setTextDatum(MC_DATUM);

            for (int i = 0; i < segments; i++) {

                uint16_t boxColor = secondaryColor->getColor();
                uint16_t numberColor = primaryColor->getColor();

                int segmentValue = charArray[i] - 48;

                int _x = (segmentWidth + padding) * i;
                int yOffset = 0;

                if (i >= segments - decimals) 
                {
                    boxColor = primaryColor->getColor();
                    numberColor = secondaryColor->getColor();
                }
                
                if (i == segments - 1) 
                {
                    float interpolationValue = fmod(val * pow(10, -segments + decimals + 1 + i), 1);

                    float interpolationWidth = 1;

                    if (interpolationValue >= 1 - interpolationWidth) {

                        float interpolation = (interpolationValue - 1 + interpolationWidth) / interpolationWidth;
                        
                        yOffset = -sharpstep(interpolation) * segmentHeight;
                    }
                }
                else 
                {
                    float interpolationValue = fmod(val * pow(10, -segments + decimals + 2 + i), 10);

                    float interpolationWidth = pow(10, i - segments + 2); //1;

                    if (interpolationValue >= 10 - interpolationWidth) {

                        float interpolation = (interpolationValue - 10 + interpolationWidth) / interpolationWidth;
                        
                        yOffset = -sharpstep(interpolation) * segmentHeight;
                    }
                }
                
                //TFT_eSprite segment = TFT_eSprite(canvas);

                segment.setTextColor(numberColor);
                segment.fillRect(_x, 0, segmentWidth, segmentHeight, boxColor);
                segment.drawString(String(segmentValue),            _x + (segmentWidth / 2), yOffset + (segmentHeight / 2) + 2);
                segment.drawString(String((segmentValue + 1) % 10), _x + (segmentWidth / 2), yOffset + (segmentHeight / 2) + segmentHeight + 2);
            }

            //segment.pushToSprite(canvas, x + xOffset, y);

            canvas.setTextColor(TFT_WHITE);
            canvas.setTextDatum(TC_DATUM);
            canvas.drawString(value.getUnitType().getAbbreviation(DEFAULT), x, y + segmentHeight + (padding * 2));
            */
        }
};

REGISTER_ELEMENT_TYPE("roll-odometer", RollOdometer);