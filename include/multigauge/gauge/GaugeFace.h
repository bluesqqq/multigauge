#pragma once

#include <memory>
#include <multigauge/gauge/elements/RootElement.h>
#include <multigauge/graphics/colors/Color.h>

class GaugeFace : public PropertyObject {
    MG_EDITOR_NAME("Gauge Face")

    private:
        RootElement root;

        unsigned long lastUpdateTime = 0;

    public:
        explicit GaugeFace() : root(nullptr) {};

        void load(const rapidjson::Document& doc);

        rapidjson::Document save() const;

        void layout(Graphics& g);

        void draw(Graphics& g) const;

        void update(int deltaTime);

        bool init(AssetManager& assetManager);

        Element* getRoot() { return &root; }
        const Element* getRoot() const { return &root; }
};