#pragma once

#include <multigauge/gauge/Element.h>
#include <multigauge/graphics/colors/Color.h>

#define NO_TITLE_TEXT "No Title"
#define NO_DESCRIPTION_TEXT "No description."

class RootElement : public Element {
    MG_EDITOR_NAME("Gauge Face")

    private:
        OwnedColor backgroundColor;
        const char* title = NO_TITLE_TEXT;
        const char* description = NO_DESCRIPTION_TEXT;

        MG_PROPS_PARENT(Element)

        MG_PROPS_BEGIN()
            MG_PROP(title, "title", "Title", "Title to display.")
            MG_PROP(description, "description", "Description", "Description to display.")
            MG_PROP(backgroundColor, "bgColor", "Background Color", "Color to fill the background with.")
        MG_PROPS_END()

    public:
        using Element::Element;
        
        void draw(Graphics& g) const override {
            g.fillAll(backgroundColor.get());
        }
};

