#include "GraphicsContextLovyanGFX.h"

//----------[ FONT REGISTRY SYSTEM ]----------//

const lgfx::IFont* LGFXFontStroke::getFont(bool wantBold, bool wantItalic) const {
    if (wantBold && wantItalic) {
        if (boldItalic) return boldItalic;
    }
    if (wantBold)   return bold ? bold : normal;
    if (wantItalic) return italic ? italic : normal;
    return normal;
}

auto LGFXFontFamily::findStrokeForPt(float pt) const {
    auto it = std::upper_bound(
        strokes.begin(), strokes.end(), pt,
        [](float value, const auto& elem) { return value < elem.first; }
    );
    if (it == strokes.begin()) return strokes.begin();
    --it;
    return it;
}

void LGFXFontFamily::addStroke(float pt, LGFXFontStroke stroke) {
    auto it = std::lower_bound(
        strokes.begin(), strokes.end(), pt,
        [](const auto& elem, float value) { return elem.first < value; }
    );

    if (it != strokes.end() && it->first == pt) it->second = stroke;
    else strokes.insert(it, {pt, stroke});
}

LGFXFontPick LGFXFontFamily::getFont(float pt, bool wantBold, bool wantItalic) const {
    if (strokes.empty()) return { &DEFAULT_LGFX_FONT, DEFAULT_LGFX_FONT_PT };

    auto it = findStrokeForPt(pt);
    const float chosenPt = it->first;
    const LGFXFontStroke& stroke = it->second;

    const lgfx::IFont* f = stroke.getFont(wantBold, wantItalic);
    if (!f) f = &DEFAULT_LGFX_FONT;

    return { f, chosenPt };
}

//----------[ Helpers ]----------//

lgfx::v1::textdatum::textdatum_t GraphicsContextLovyanGFX::anchorToDatum(Anchor anchor) const {
    using lgfx::v1::textdatum_t;

    switch (anchor) {
        case Anchor::TopLeft:      return textdatum_t::top_left;
        case Anchor::TopCenter:    return textdatum_t::top_center;
        case Anchor::TopRight:     return textdatum_t::top_right;

        case Anchor::CenterLeft:   return textdatum_t::middle_left;
        case Anchor::Center:       return textdatum_t::middle_center;
        case Anchor::CenterRight:  return textdatum_t::middle_right;

        case Anchor::BottomLeft:   return textdatum_t::bottom_left;
        case Anchor::BottomCenter: return textdatum_t::bottom_center;
        case Anchor::BottomRight:  return textdatum_t::bottom_right;
    }
    return textdatum_t::top_left;
}

void GraphicsContextLovyanGFX::setFontInternal(const std::string& family, float pt, FontWeight weight, FontSlant slant) {
    const lgfx::IFont* f = &DEFAULT_LGFX_FONT;
    float basePt = DEFAULT_LGFX_FONT_PT;

    auto it = fontFamilies.find(family);
    if (it != fontFamilies.end()) {
        auto pick = it->second.getFont(pt, weight == FontWeight::Bold, slant == FontSlant::Italic);
        f = pick.font;
        basePt = pick.strokePt;
    }

    buffer.setTextSize(1);
    buffer.setFont(f);

    currentFontPoint = (float)buffer.fontHeight();
    if (currentFontPoint < 1.0f) currentFontPoint = (float)basePt;

    float s = pt / currentFontPoint;
    if (s < 1.0f) s = 1.0f;
    buffer.setTextSize(s);
}

void GraphicsContextLovyanGFX::strokeRectThick(int x, int y, int w, int h, uint32_t color, int t) {
    if (t <= 1) {
        buffer.drawRect(x, y, w, h, color);
        return;
    }
    // Top
    buffer.fillRect(x, y, w, t, color);
    // Bottom
    buffer.fillRect(x, y + h - t, w, t, color);
    // Left
    buffer.fillRect(x, y, t, h, color);
    // Right
    buffer.fillRect(x + w - t, y, t, h, color);
}

void GraphicsContextLovyanGFX::strokeRoundRectThick(int x, int y, int w, int h, int radius, uint32_t color, int t) {
    if (t <= 1) {
        buffer.drawRoundRect(x, y, w, h, radius, color);
        return;
    }
    for (int i = 0; i < t; ++i) {
        int xx = x + i;
        int yy = y + i;
        int ww = w - 2 * i;
        int hh = h - 2 * i;
        if (ww <= 0 || hh <= 0) break;
        int rr = radius - i;
        if (rr < 0) rr = 0;
        buffer.drawRoundRect(xx, yy, ww, hh, rr, color);
    }
}

//----------[ Lifecycle ]----------//

bool GraphicsContextLovyanGFX::init() {
    if (!display.init()) return false;

    display.setBrightness(255);
    display.setRotation(0);

    w = display.width();
    h = display.height();
    if (w <= 0 || h <= 0) return false;

    buffer.setColorDepth(16);
    buffer.setPsram(false);
    if (!buffer.createSprite(w, h)) return false;

    {
        auto& sans = fontFamilies["sans"];
        sans.addStroke(9,  {&lgfx::fonts::FreeSans9pt7b,  &lgfx::fonts::FreeSansBold9pt7b,  &lgfx::fonts::FreeSansOblique9pt7b,  &lgfx::fonts::FreeSansBoldOblique9pt7b});
        sans.addStroke(12, {&lgfx::fonts::FreeSans12pt7b, &lgfx::fonts::FreeSansBold12pt7b, &lgfx::fonts::FreeSansOblique12pt7b, &lgfx::fonts::FreeSansBoldOblique12pt7b});
        sans.addStroke(18, {&lgfx::fonts::FreeSans18pt7b, &lgfx::fonts::FreeSansBold18pt7b, &lgfx::fonts::FreeSansOblique18pt7b, &lgfx::fonts::FreeSansBoldOblique18pt7b});
        sans.addStroke(24, {&lgfx::fonts::FreeSans24pt7b, &lgfx::fonts::FreeSansBold24pt7b, &lgfx::fonts::FreeSansOblique24pt7b, &lgfx::fonts::FreeSansBoldOblique24pt7b});
    }

    {
        auto& serif = fontFamilies["serif"];
        serif.addStroke(9,  {&lgfx::fonts::FreeSerif9pt7b,  &lgfx::fonts::FreeSerifBold9pt7b,  &lgfx::fonts::FreeSerifItalic9pt7b,  &lgfx::fonts::FreeSerifBoldItalic9pt7b});
        serif.addStroke(12, {&lgfx::fonts::FreeSerif12pt7b, &lgfx::fonts::FreeSerifBold12pt7b, &lgfx::fonts::FreeSerifItalic12pt7b, &lgfx::fonts::FreeSerifBoldItalic12pt7b});
        serif.addStroke(18, {&lgfx::fonts::FreeSerif18pt7b, &lgfx::fonts::FreeSerifBold18pt7b, &lgfx::fonts::FreeSerifItalic18pt7b, &lgfx::fonts::FreeSerifBoldItalic18pt7b});
        serif.addStroke(24, {&lgfx::fonts::FreeSerif24pt7b, &lgfx::fonts::FreeSerifBold24pt7b, &lgfx::fonts::FreeSerifItalic24pt7b, &lgfx::fonts::FreeSerifBoldItalic24pt7b});
    }

    {
        auto& mono = fontFamilies["mono"];
        mono.addStroke(9,  {&lgfx::fonts::FreeMono9pt7b,  &lgfx::fonts::FreeMonoBold9pt7b,  &lgfx::fonts::FreeMonoOblique9pt7b,  &lgfx::fonts::FreeMonoBoldOblique9pt7b});
        mono.addStroke(12, {&lgfx::fonts::FreeMono12pt7b, &lgfx::fonts::FreeMonoBold12pt7b, &lgfx::fonts::FreeMonoOblique12pt7b, &lgfx::fonts::FreeMonoBoldOblique12pt7b});
        mono.addStroke(18, {&lgfx::fonts::FreeMono18pt7b, &lgfx::fonts::FreeMonoBold18pt7b, &lgfx::fonts::FreeMonoOblique18pt7b, &lgfx::fonts::FreeMonoBoldOblique18pt7b});
        mono.addStroke(24, {&lgfx::fonts::FreeMono24pt7b, &lgfx::fonts::FreeMonoBold24pt7b, &lgfx::fonts::FreeMonoOblique24pt7b, &lgfx::fonts::FreeMonoBoldOblique9pt7b}); // double-check this last font pointer
    }

    {
        auto& orbitron = fontFamilies["orbitron"];
        orbitron.addStroke(24, {&lgfx::fonts::Orbitron_Light_24});
        orbitron.addStroke(32, {&lgfx::fonts::Orbitron_Light_32});
    }

    {
        auto& dejavu = fontFamilies["dejavu"];
        dejavu.addStroke(9,  {&lgfx::fonts::DejaVu9});
        dejavu.addStroke(12, {&lgfx::fonts::DejaVu12});
        dejavu.addStroke(18, {&lgfx::fonts::DejaVu18});
        dejavu.addStroke(24, {&lgfx::fonts::DejaVu24});
        dejavu.addStroke(40, {&lgfx::fonts::DejaVu40});
        dejavu.addStroke(56, {&lgfx::fonts::DejaVu56});
        dejavu.addStroke(72, {&lgfx::fonts::DejaVu72});
    }

    {
        auto& roboto = fontFamilies["roboto"];
        roboto.addStroke(24, {&lgfx::fonts::Roboto_Thin_24});
    }

    buffer.setFont(&lgfx::fonts::FreeSans12pt7b);
    currentFontPoint = 12.0f;

    return true;
}

void GraphicsContextLovyanGFX::endFrame() {
    buffer.pushSprite(0, 0);
}

//----------[ FILL ]----------//

void GraphicsContextLovyanGFX::clear(rgba color) {
    buffer.fillScreen(lgfxColor888(color));
}

//----------[ PIXEL ]----------//

void GraphicsContextLovyanGFX::pixel(int x, int y, rgba color) {
    buffer.drawPixel(x, y, lgfxColor888(color));
}

//----------[ LINE ]----------//

void GraphicsContextLovyanGFX::line(int x0, int y0, int x1, int y1, rgba color, float thickness) {
    const uint32_t c = lgfxColor888(color);
    if (thickness <= 1.0f) {
        buffer.drawLine(x0, y0, x1, y1, c);
        return;
    }
    buffer.drawWideLine(x0, y0, x1, y1, thickness, c);
}

//----------[ RECTANGLE ]----------//

void GraphicsContextLovyanGFX::rect(int x, int y, int ww, int hh, rgba color) {
    buffer.fillRect(x, y, ww, hh, lgfxColor888(color));
}

void GraphicsContextLovyanGFX::strokeRect(int x, int y, int ww, int hh, rgba color, float thickness) {
    line(x, y, x + ww - 1, y, color, thickness);           // Top
    line(x, y + hh - 1, x + ww - 1, y + hh - 1, color, thickness); // Bottom
    line(x, y, x, y + hh - 1, color, thickness);         // Left
    line(x + ww - 1, y, x + ww - 1, y + hh - 1, color, thickness); // Right
}

void GraphicsContextLovyanGFX::roundRect(int x, int y, int ww, int hh, float radius, rgba color) {
    buffer.fillRoundRect(x, y, ww, hh, (int)std::lround(radius), lgfxColor888(color));
}

void GraphicsContextLovyanGFX::roundRect(int x, int y, int ww, int hh, float r1, float r2, float r3, float r4, rgba color) {
    float rr = std::min(std::min(r1, r2), std::min(r3, r4));
    if (rr < 0) rr = 0;
    buffer.fillRoundRect(x, y, ww, hh, (int)std::lround(rr), lgfxColor888(color));
}

void GraphicsContextLovyanGFX::strokeRoundRect(int x, int y, int ww, int hh, float radius, rgba color, float thickness) {
    int t = (int)std::ceil(thickness);
    if (t < 1) t = 1;
    strokeRoundRectThick(x, y, ww, hh, (int)std::lround(radius), lgfxColor888(color), t);
}

void GraphicsContextLovyanGFX::strokeRoundRect(int x, int y, int ww, int hh, float r1, float r2, float r3, float r4, rgba color, float thickness) {
    float rr = std::min(std::min(r1, r2), std::min(r3, r4));
    if (rr < 0) rr = 0;
    strokeRoundRect(x, y, ww, hh, rr, color, thickness);
}

//----------[ CIRCLE ]----------//

void GraphicsContextLovyanGFX::circle(int cx, int cy, int r, rgba color) {
    buffer.fillCircle(cx, cy, r, lgfxColor888(color));
}

void GraphicsContextLovyanGFX::strokeCircle(int cx, int cy, int r, rgba color, float thickness) {
    const uint32_t c = lgfxColor888(color);
    int t = (int)std::ceil(thickness);
    if (t < 1) t = 1;

    if (t == 1) {
        buffer.drawCircle(cx, cy, r, c);
        return;
    }

    int half = t / 2;
    for (int i = -half; i <= half; ++i) {
        int rr = r + i;
        if (rr > 0) buffer.drawCircle(cx, cy, rr, c);
    }
}

//----------[ ELLIPSE ]----------//

void GraphicsContextLovyanGFX::ellipse(int cx, int cy, int rx, int ry, rgba color) {
    buffer.fillEllipse(cx, cy, rx, ry, lgfxColor888(color));
}

void GraphicsContextLovyanGFX::strokeEllipse(int cx, int cy, int rx, int ry, rgba color, float thickness) {
    const uint32_t c = lgfxColor888(color);
    int t = (int)std::ceil(thickness);
    if (t < 1) t = 1;

    if (t == 1) {
        buffer.drawEllipse(cx, cy, rx, ry, c);
        return;
    }

    int half = t / 2;
    for (int i = -half; i <= half; ++i) {
        int rrx = rx + i;
        int rry = ry + i;
        if (rrx > 0 && rry > 0) buffer.drawEllipse(cx, cy, rrx, rry, c);
    }
}

//----------[ RING ]----------//

void GraphicsContextLovyanGFX::ring(int cx, int cy, int r1, int r2, rgba color) {
    arc(cx, cy, r1, r2, 0.0f, 360.0f, color);
}

void GraphicsContextLovyanGFX::strokeRing(int cx, int cy, int r1, int r2, rgba color, float thickness) {
    strokeCircle(cx, cy, r1, color, thickness);
    strokeCircle(cx, cy, r2, color, thickness);
}

//----------[ ARC ]----------//

void GraphicsContextLovyanGFX::arc(int cx, int cy, int r1, int r2, float start, float end, rgba color) {
    buffer.fillArc(cx, cy, r1, r2, start, end, lgfxColor888(color));
}

void GraphicsContextLovyanGFX::strokeArc(int cx, int cy, int r1, int r2, float start, float end, rgba color, float thickness) {
    const float halfThickness = thickness / 2.0f;
    const uint32_t c = lgfxColor888(color);
    buffer.fillArc(cx, cy, r1 - halfThickness, r1 + halfThickness, start, end, c);
    buffer.fillArc(cx, cy, r2 - halfThickness, r2 + halfThickness, start, end, c);
    line(cx + (int)std::round(r1 * std::cos(start * M_PI / 180.0f)), cy + (int)std::round(r1 * std::sin(start * M_PI / 180.0f)),
         cx + (int)std::round(r2 * std::cos(start * M_PI / 180.0f)), cy + (int)std::round(r2 * std::sin(start * M_PI / 180.0f)),
         color, thickness);
}

//----------[ TRIANGLE ]----------//

void GraphicsContextLovyanGFX::tri(int x0, int y0, int x1, int y1, int x2, int y2, rgba color) {
    buffer.fillTriangle(x0, y0, x1, y1, x2, y2, lgfxColor888(color));
}

void GraphicsContextLovyanGFX::strokeTri(int x0, int y0, int x1, int y1, int x2, int y2, rgba color, float thickness) {
    const uint32_t c = lgfxColor888(color);

    if (thickness <= 1.0f) {
        buffer.drawTriangle(x0, y0, x1, y1, x2, y2, c);
        return;
    }

    // Approx: draw 3 wide lines
    buffer.drawWideLine(x0, y0, x1, y1, thickness, c);
    buffer.drawWideLine(x1, y1, x2, y2, thickness, c);
    buffer.drawWideLine(x2, y2, x0, y0, thickness, c);
}

//----------[ TEXT ]----------//

float GraphicsContextLovyanGFX::getTextWidth(const char* text, std::string family, float pt, FontWeight weight, FontSlant slant) {
    setFontInternal(family, pt, weight, slant);
    return (float)buffer.textWidth(text);
}

void GraphicsContextLovyanGFX::drawText(const char* text, int x, int y, std::string family, float pt, FontWeight weight, FontSlant slant, rgba color, Anchor anchor) {
    setFontInternal(family, pt, weight, slant);
    buffer.setTextDatum(anchorToDatum(anchor));
    buffer.setTextColor(lgfxColor888(color));
    buffer.drawString(text, x, y);
}

//----------[ IMAGE ]----------//

Image GraphicsContextLovyanGFX::createNativeImage(const rgba* pixels, int ww, int hh) {
    auto* spr = new lgfx::LGFX_Sprite(&buffer);

    spr->setColorDepth(16);
    spr->setPsram(true);

    if (!spr->createSprite(ww, hh)) {
        delete spr;
        return Image(0, 0, nullptr, nullptr);
    }

    spr->setSwapBytes(true);

    std::vector<uint16_t> tmp;
    tmp.resize((size_t)ww * (size_t)hh);

    for (size_t i = 0; i < tmp.size(); ++i) {
        const rgba px = pixels[i];
        if (px.a == 0) tmp[i] = TRANSPARENT_KEY_565;
        else tmp[i] = rgb888_to_565(px.r, px.g, px.b);
    }

    spr->pushImage(0, 0, ww, hh, tmp.data());
    spr->setSwapBytes(false);

    return Image(ww, hh, spr, [](void* ptr) { delete static_cast<lgfx::LGFX_Sprite*>(ptr); });
}

void GraphicsContextLovyanGFX::drawImage(const Image& img, int x, int y) {
    if (!img.native) return;
    auto* spr = static_cast<lgfx::LGFX_Sprite*>(img.native);
    if (!spr) return;

    const uint16_t key = swapped565(TRANSPARENT_KEY_565);
    spr->pushSprite(&buffer, x, y, key);
}

void GraphicsContextLovyanGFX::drawImageStretched(const Image& img, int x, int y, int width, int height) {
    if (!img.native || width <= 0 || height <= 0 || img.width <= 0 || img.height <= 0) return;
    auto* spr = static_cast<lgfx::LGFX_Sprite*>(img.native);
    if (!spr) return;

    const float zx = (float)width  / (float)img.width;
    const float zy = (float)height / (float)img.height;

    const uint16_t key = swapped565(TRANSPARENT_KEY_565);
    spr->setPivot(img.width / 2, img.height / 2);
    spr->pushRotateZoom(&buffer, x + width / 2, y + height / 2, 0.0f, zx, zy, key);
}

void GraphicsContextLovyanGFX::drawImageScaled(const Image& img, int x, int y, float sx, float sy) {
    if (!img.native || img.width <= 0 || img.height <= 0) return;
    auto* spr = static_cast<lgfx::LGFX_Sprite*>(img.native);
    if (!spr) return;

    if (sx <= 0.0f || sy <= 0.0f) return;

    const int scaledW = (int)std::lround((float)img.width * sx);
    const int scaledH = (int)std::lround((float)img.height * sy);
    if (scaledW <= 0 || scaledH <= 0) return;

    const uint16_t key = swapped565(TRANSPARENT_KEY_565);
    spr->pushRotateZoom(&buffer, x + scaledW / 2, y + scaledH / 2, 0.0f, sx, sy, key);
}

void GraphicsContextLovyanGFX::drawImageRotated(const Image& img, int x, int y, float angle, int pivotX, int pivotY) {
    drawImageTransformed(img, x, y, angle, 1.0f, 1.0f, pivotX, pivotY);
}

void GraphicsContextLovyanGFX::drawImageTransformed(const Image& img, int x, int y, float angleDeg, float sx, float sy, int pivotX, int pivotY) {
    if (!img.native || img.width <= 0 || img.height <= 0) return;
    if (sx <= 0.0f || sy <= 0.0f) return;

    auto* spr = static_cast<lgfx::LGFX_Sprite*>(img.native);

    const int scaledW = (int)std::lround((float)img.width  * sx);
    const int scaledH = (int)std::lround((float)img.height * sy);
    if (scaledW <= 0 || scaledH <= 0) return;

    const int destPivotX = x + (int)std::lround((float)pivotX * sx);
    const int destPivotY = y + (int)std::lround((float)pivotY * sy);

    spr->setPivot(pivotX, pivotY);

    const uint16_t key = swapped565(TRANSPARENT_KEY_565);

    spr->pushRotateZoom(&buffer, destPivotX, destPivotY, angleDeg, sx, sy, key);

    spr->setPivot(0, 0);
}

void GraphicsContextLovyanGFX::drawImageRegion(const Image& img, int srcX, int srcY, int srcW, int srcH, int destX, int destY, int destW, int destH) {
    // TODO: this is a temporary implementation until I can find something better.
    if (!img.native) return;
    if (srcW <= 0 || srcH <= 0 || destW <= 0 || destH <= 0) return;

    auto* spr = static_cast<lgfx::LGFX_Sprite*>(img.native);
    if (!spr) return;

    const int iw = img.width;
    const int ih = img.height;

    if (srcX < 0) { srcW += srcX; srcX = 0; }
    if (srcY < 0) { srcH += srcY; srcY = 0; }
    if (srcX + srcW > iw) srcW = iw - srcX;
    if (srcY + srcH > ih) srcH = ih - srcY;
    if (srcW <= 0 || srcH <= 0) return;

    clip(destX, destY, destW, destH);

    const float sx = (float)destW / (float)srcW;
    const float sy = (float)destH / (float)srcH;
    const int originX = destX - (int)std::lround((float)srcX * sx);
    const int originY = destY - (int)std::lround((float)srcY * sy);

    const uint16_t key = swapped565(TRANSPARENT_KEY_565);

    spr->setPivot(0, 0);

    spr->pushRotateZoom(&buffer, originX, originY, 0.0f, sx, sy, key);
    clearClip();
}

//----------[ CLIP ]----------//

void GraphicsContextLovyanGFX::clip(int x, int y, int width, int height) {
    buffer.setClipRect(x, y, width, height);
}

void GraphicsContextLovyanGFX::clearClip() {
    buffer.clearClipRect();
}