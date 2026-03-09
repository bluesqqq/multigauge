#pragma once

#include <memory>
#include <multigauge/gauge/Element.h>
#include <multigauge/graphics/colors/Color.h>

#define NO_TITLE_TEXT "No Title"
#define NO_DESCRIPTION_TEXT "No description."

class GaugeFace : public PropertyObject {
    MG_EDITOR_NAME("Gauge Face")

    private:
        OwnedElement root;

        OwnedColor backgroundColor;

        const char* title = NO_TITLE_TEXT;

        const char* description = NO_DESCRIPTION_TEXT;

        unsigned long lastUpdateTime = 0;

        MG_PROPS_BEGIN()
            MG_PROP(title, "title", "Title", "Title to display.")
            MG_PROP(description, "description", "Description", "Description to display.")
            MG_PROP(backgroundColor, "bgColor", "Background Color", "Color to fill the background with.")
        MG_PROPS_END()

    public:
        explicit GaugeFace() = default;

        void load(const rapidjson::Document& doc);

        rapidjson::Document save() const;

        void layout(Graphics& g);

        void draw(Graphics& g) const;

        void update(int deltaTime);

        bool init(AssetManager& assetManager);

        Element* getRoot() const { return root.get(); }
};