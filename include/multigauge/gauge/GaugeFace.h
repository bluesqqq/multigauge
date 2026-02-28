#pragma once

#include <memory>
#include <multigauge/gauge/Element.h>
#include <multigauge/graphics/colors/Color.h>

#define NO_TITLE_TEXT "No Title"
#define NO_DESCRIPTION_TEXT "No description."

class GaugeFace : public Element {
    MG_EDITOR_NAME("Gauge Face")
    private:
        OwnedColor backgroundColor;

        const char* title = NO_TITLE_TEXT;

        const char* description = NO_DESCRIPTION_TEXT;

        unsigned long lastUpdateTime = 0;

        MG_EDITABLE_BEGIN()
            MG_EDITABLE_PROP(title)
            MG_EDITABLE_PROP(description)
            MG_EDITABLE_PROP(backgroundColor)
        MG_EDITABLE_END()

    public:
        explicit GaugeFace();

        void draw(Graphics& g) const override;
};