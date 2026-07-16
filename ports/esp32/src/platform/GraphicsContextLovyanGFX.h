#pragma once

#include "LGFX_definition.h"

#include <multigauge/graphics/GraphicsContext.h>
#include <multigauge/graphics/font/Font.h>
#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>

using mg::Anchor;
using mg::graphics::FontSlant;
using mg::graphics::FontWeight;
using mg::graphics::GraphicsContext;
using mg::graphics::rgba;
using mg::images::Image;

//----------[ FONT REGISTRY SYSTEM ]----------//

#ifndef DEFAULT_LGFX_FONT
#define DEFAULT_LGFX_FONT lgfx::fonts::Font2
#endif

#ifndef DEFAULT_LGFX_FONT_PT
#define DEFAULT_LGFX_FONT_PT 16
#endif

struct LGFXFontPick {
    const lgfx::IFont* font = nullptr;
    float strokePt = 0.0f;
};

struct LGFXFontStroke {
    const lgfx::IFont* normal = nullptr;
    const lgfx::IFont* bold = nullptr;
    const lgfx::IFont* italic = nullptr;
    const lgfx::IFont* boldItalic = nullptr;

    const lgfx::IFont* getFont(bool wantBold, bool wantItalic) const;
};

class LGFXFontFamily {
private:
    std::vector<std::pair<float, LGFXFontStroke>> strokes;

    auto findStrokeForPt(float pt) const;

public:
    void addStroke(float pt, LGFXFontStroke stroke);
    LGFXFontPick getFont(float pt, bool wantBold, bool wantItalic) const;
};

class GraphicsContextLovyanGFX final : public GraphicsContext {
private:
    LGFX display;
    LGFX_Sprite buffer;

    float currentFontPoint = 16.0f;
    std::unordered_map<std::string, LGFXFontFamily> fontFamilies;

    lgfx::v1::textdatum::textdatum_t anchorToDatum(Anchor anchor) const;
    void setFontInternal(const std::string& family, float pt, FontWeight weight, FontSlant slant);

    static uint16_t rgb888_to_565(uint8_t r, uint8_t g, uint8_t b) {
        return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    }

    uint32_t lgfxColor888(rgba c) const { return display.color888(c.r, c.g, c.b); }

    void strokeRectThick(int x, int y, int w, int h, uint32_t color, int t);
    void strokeRoundRectThick(int x, int y, int w, int h, int radius, uint32_t color, int t);

    // LovyanGFX transparency key handling for 16-bit sprites
    static constexpr uint16_t TRANSPARENT_KEY_565 = TFT_TRANSPARENT;
    static constexpr uint16_t swapped565(uint16_t c) { return (uint16_t)((c << 8) | (c >> 8)); }

public:
    GraphicsContextLovyanGFX() : display(LGFX()), buffer(&display) {}

    bool init() override;
    void beginFrame() override {}
    void endFrame() override;

    //----------[ FILL ]----------//
    void clear(rgba color) override;

    //----------[ PIXEL ]----------//
    void pixel(int x, int y, rgba color) override;

    //----------[ LINE ]----------//
    void line(int x0, int y0, int x1, int y1, rgba color, float thickness) override;

    //----------[ RECTANGLE ]----------//
    void rect(int x, int y, int w, int h, rgba color) override;
    void strokeRect(int x, int y, int w, int h, rgba color, float thickness) override;

    void roundRect(int x, int y, int w, int h, float radius, rgba color) override;
    void roundRect(int x, int y, int w, int h, float r1, float r2, float r3, float r4, rgba color) override;

    void strokeRoundRect(int x, int y, int w, int h, float radius, rgba color, float thickness) override;
    void strokeRoundRect(int x, int y, int w, int h, float r1, float r2, float r3, float r4, rgba color, float thickness) override;

    //----------[ CIRCLE ]----------//
    void circle(int cx, int cy, int r, rgba color) override;
    void strokeCircle(int cx, int cy, int r, rgba color, float thickness) override;

    //----------[ ELLIPSE ]----------//
    void ellipse(int cx, int cy, int rx, int ry, rgba color) override;
    void strokeEllipse(int cx, int cy, int rx, int ry, rgba color, float thickness) override;

    //----------[ RING ]----------//
    void ring(int cx, int cy, int r1, int r2, rgba color) override;
    void strokeRing(int cx, int cy, int r1, int r2, rgba color, float thickness) override;

    //----------[ ARC ]----------//
    void arc(int cx, int cy, int r1, int r2, float start, float end, rgba color) override;
    void strokeArc(int cx, int cy, int r1, int r2, float start, float end, rgba color, float thickness) override;

    //----------[ TRIANGLE ]----------//
    void tri(int x0, int y0, int x1, int y1, int x2, int y2, rgba color) override;
    void strokeTri(int x0, int y0, int x1, int y1, int x2, int y2, rgba color, float thickness) override;

    //----------[ TEXT ]----------//
    float getTextWidth(const char* text, std::string family, float pt, FontWeight weight, FontSlant slant) override;
    void drawText(const char* text, int x, int y, std::string family, float pt, FontWeight weight, FontSlant slant, rgba color, Anchor anchor) override;

    //----------[ IMAGE ]----------//
    Image createNativeImage(const rgba* pixels, int w, int h) override;

    void drawImage(const Image& img, int x, int y) override;
    void drawImageRotated(const Image& img, int x, int y, float angle, int pivotX, int pivotY) override;
    void drawImageScaled(const Image& img, int x, int y, float sx, float sy) override;
    void drawImageTransformed(const Image& img, int x, int y, float angle, float sx, float sy, int pivotX, int pivotY) override;

    void drawImageStretched(const Image& img, int x, int y, int width, int height) override;
    void drawImageRegion(const Image& img, int srcX, int srcY, int srcW, int srcH, int destX, int destY, int destW, int destH) override;

    //----------[ CLIP ]----------//
    void clip(int x, int y, int width, int height) override;
    void clearClip() override;
};
