#pragma once

#include <multigauge/gauge/Element.h>
#include <multigauge/images/NineSlice.h>

/*
class NineSliceElement : public Element {
    private:
        NineSlice nineSlice;

    public:
        explicit NineSliceElement(Element* parent);

        NineSliceElement(Element* parent, const rapidjson::Value::ConstObject json) : Element(parent, json) {
            if (!json.HasMember("props") || !json["props"].IsObject()) return;
            const rapidjson::Value::ConstObject props = json["props"].GetObject();

            setObj(props, "nineSlice", nineSlice);
        }

        bool init(AssetManager& assetManager) override {
            return nineSlice.init(assetManager);
        }

        void draw(Graphics& g) const override {
            const auto& b = getBounds();

            g.drawNineSlice(nineSlice, b.toInt());
        }
};
*/