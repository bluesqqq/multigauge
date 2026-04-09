#pragma once

#include <memory>
#include <type_traits>
#include <multigauge/gauge/elements/RootElement.h>
#include <multigauge/graphics/colors/Color.h>

class GaugeFace : public PropertyObject {
    MG_EDITOR_NAME("Gauge Face")

    private:
        using YogaConfigOwner = std::unique_ptr<std::remove_pointer_t<YGConfigRef>, decltype(&YGConfigFree)>;

        YogaConfigOwner config;
        RootElement root;

        unsigned long lastUpdateTime = 0;

    public:
        GaugeFace();
        ~GaugeFace() = default;

        void load(const rapidjson::Value& json);

        rapidjson::Document save() const;

        void layout(Graphics& g);

        void draw(Graphics& g) const;

        void update(int deltaTime);

        bool init(AssetManager& assetManager);

        Element* getRoot() { return &root; }
        const Element* getRoot() const { return &root; }
};
