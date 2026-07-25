#pragma once

#include <multigauge/gauge/Element.h>
#include <multigauge/graphics/image/NineSlice.h>

/*
class NineSliceElement : public Element {
    private:
        NineSlice nineSlice;

    public:
        explicit NineSliceElement(Element* parent);

        bool init(AssetManager& assetManager) override {
            return nineSlice.init(assetManager);
        }

        void draw(Graphics& g) const override {
            const auto& b = getBounds();

            g.drawNineSlice(nineSlice, b.toInt());
        }
};
*/
