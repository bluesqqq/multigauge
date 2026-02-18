#pragma once

#include <memory>
#include "graphics/gauge/Element.h"
#include "graphics/colors/Color.h"

#define NO_TITLE_TEXT "No Title"
#define NO_DESCRIPTION_TEXT "No description."

class GaugeFace : public Element {
    private:
        std::unique_ptr<Color> backgroundColor;

        const char* title = NO_TITLE_TEXT;

        const char* description = NO_DESCRIPTION_TEXT;

        unsigned long lastUpdateTime = 0;

    public:
        explicit GaugeFace();

        GaugeFace(const rapidjson::Document& json);

        void draw(Graphics& g) const override;
};