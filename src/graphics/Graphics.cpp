#include "Graphics.h"

#include <cmath>

rgba Graphics::getCachedColor(const Color& color) {
    auto it = colorCache.find(&color);
    if (it != colorCache.end())
        return it->second;

    rgba c = color.getColor();
    colorCache[&color] = c;
    return c;
}

Graphics::Graphics(GraphicsContext *context) : context(context) { }

Rect<int> Graphics::getScreenBounds() { return Rect<int>(0, 0, context->getScreenWidth(), context->getScreenHeight()); }

//----------[ COLOR ]----------//

void Graphics::clearColorCache() { colorCache.clear(); }

void Graphics::setFill(rgba color) { fillValue = color; }

void Graphics::setFill(const Color& color) { fillValue = getCachedColor(color); }

void Graphics::setStroke(rgba color) { strokeValue = color; }

void Graphics::setStroke(const Color& color) { strokeValue = getCachedColor(color); }

void Graphics::setStrokeThickness(float t) { thickness = t; }

void Graphics::setFillStroke(const FillStroke &fillStroke) {
    if (fillStroke.fill) {
        setFill(*fillStroke.fill);
    }
    
    if (fillStroke.stroke) {
        setStroke(*fillStroke.stroke);
        setStrokeThickness(fillStroke.thickness);
    }
}

//----------[ FILL ]----------//

void Graphics::fillAll() const { fillAll(fillValue); }

void Graphics::fillAll(const Color& color) { context->fillAll(getCachedColor(color)); }

void Graphics::fillAll(rgba color) const { context->fillAll(color); }

//----------[ PIXEL ]----------//

void Graphics::fillPixel(int x, int y) const { context->drawPixel(x, y, fillValue); }

void Graphics::fillPixel(const Point<int> &pos) const { context->drawPixel(pos.x, pos.y, fillValue); }

//----------[ LINE ]----------//

void Graphics::strokeLine(int x0, int y0, int x1, int y1) const { context->strokeLine(x0, y0, x1, y1, strokeValue); }

void Graphics::strokeLine(const Point<int> &p1, const Point<int> &p2) const { strokeLine(p1.x, p1.y, p2.x, p2.y); }

void Graphics::strokeLine(const Line<int> &line) const { strokeLine(line.p1.x, line.p1.y, line.p2.x, line.p2.y); }

void Graphics::fillWideLine(int x0, int y0, int x1, int y1, float t) const { context->fillWideLine(x0, y0, x1, y1, fillValue, t); }

void Graphics::fillWideLine(const Point<int> &p1, const Point<int> &p2, float t) const { context->fillWideLine(p1.x, p1.y, p2.x, p2.y, fillValue, t); }

void Graphics::fillWideLine(const Line<int> &line, float t) const { context->fillWideLine(line.p1.x, line.p1.y, line.p2.x, line.p2.y, fillValue, t); }

//----------[ RECTANGLE ]----------//

void Graphics::fillRect(int x, int y, int w, int h) const { context->fillRect(x, y, w, h, fillValue); }

void Graphics::fillRect(const Rect<int> &rectangle) const { fillRect(rectangle.x, rectangle.y, rectangle.width, rectangle.height); }

void Graphics::fillRoundedRect(int x, int y, int w, int h, float radius) const { context->fillRoundRect(x, y, w, h, radius, fillValue); }

void Graphics::fillRoundedRect(const Rect<int> &rect, float radius) const { context->fillRoundRect(rect.x, rect.y, rect.width, rect.height, radius, fillValue); }

void Graphics::strokeRect(int x, int y, int w, int h) const { context->strokeRect(x, y, w, h, strokeValue, thickness); }

void Graphics::strokeRect(const Rect<int> &rectangle) const { strokeRect(rectangle.x, rectangle.y, rectangle.width, rectangle.height); }

void Graphics::strokeRoundedRect(int x, int y, int w, int h, float radius) const { context->strokeRoundRect(x, y, w, h, radius, strokeValue, thickness); }

void Graphics::strokeRoundedRect(const Rect<int> &rect, float radius) const { context->strokeRoundRect(rect.x, rect.y, rect.width, rect.height, radius, strokeValue, thickness); }

//----------[ ELLIPSE ]----------//

void Graphics::fillEllipse(int cx, int cy, int rx, int ry) const { context->fillEllipse(cx, cy, rx, ry, fillValue); }

void Graphics::fillEllipse(const Point<int> &center, int rx, int ry) const { context->fillEllipse(center.x, center.y, rx, ry, fillValue); }

void Graphics::fillEllipseInRect(int x, int y, int w, int h) const { fillEllipseInRect(Rect<int>(x, y, w, h)); }

void Graphics::fillEllipseInRect(const Rect<int> &area) const {
    const auto center = area.getCenter();
    context->fillEllipse(center.x, center.y, area.width / 2, area.height / 2, fillValue);
}

void Graphics::strokeEllipse(int cx, int cy, int rx, int ry) const { context->strokeEllipse(cx, cy, rx, ry, strokeValue, 1); }

void Graphics::strokeEllipse(const Point<int> &center, int rx, int ry) const { context->strokeEllipse(center.x, center.y, rx, ry, fillValue, thickness); }

void Graphics::strokeEllipseInRect(int x, int y, int w, int h) const { strokeEllipseInRect(Rect<int>(x, y, w, h)); }

void Graphics::strokeEllipseInRect(const Rect<int> &area) const {
    const auto center = area.getCenter();
    context->strokeEllipse(center.x, center.y, area.width / 2, area.height / 2, strokeValue, thickness);
}

//----------[ CIRCLE ]----------//

void Graphics::fillCircle(int cx, int cy, int radius) const { context->fillCircle(cx, cy, radius, fillValue); }

void Graphics::fillCircle(const Point<int> &center, int radius) const { context->fillCircle(center.x, center.y, radius, fillValue); }

void Graphics::fillCircleInRect(int x, int y, int w, int h) const { fillCircleInRect(Rect<int>(x, y, w, h)); }

void Graphics::fillCircleInRect(const Rect<int> &area) const {
    const auto center = area.getCenter();
    context->fillCircle(center.x, center.y, std::min(area.width / 2, area.height / 2), fillValue);
}

void Graphics::strokeCircle(int cx, int cy, int radius) const { context->strokeCircle(cx, cy, radius, strokeValue, 1); }

void Graphics::strokeCircle(const Point<int> &center, int radius) const { context->strokeCircle(center.x, center.y, radius, strokeValue, thickness); }

void Graphics::strokeCircleInRect(int x, int y, int w, int h) const { strokeCircleInRect(Rect<int>(x, y, w, h));  }

void Graphics::strokeCircleInRect(const Rect<int> &area) const {
    const auto center = area.getCenter();
    context->strokeCircle(center.x, center.y, std::min(area.width / 2, area.height / 2), strokeValue, thickness);
}

//----------[ ARC ]----------//

void Graphics::fillArc(int cx, int cy, int r1, int r2, float startAngle, float endAngle) const { context->fillArc(cx, cy, r1, r2, startAngle, endAngle, fillValue); }

void Graphics::fillArc(const Point<int> &center, int r1, int r2, float startAngle, float endAngle) const { context->fillArc(center.x, center.y, r1, r2, startAngle, endAngle, fillValue); }

void Graphics::strokeArc(int cx, int cy, int r1, int r2, float startAngle, float endAngle) const { context->strokeArc(cx, cy, r1, r2, startAngle, endAngle, strokeValue, thickness); }

void Graphics::strokeArc(const Point<int> &center, int r1, int r2, float startAngle, float endAngle) const { context->strokeArc(center.x, center.y, r1, r2, startAngle, endAngle, strokeValue, thickness); }

//----------[ TRIANGLE ]----------//

void Graphics::fillTri(int x0, int y0, int x1, int y1, int x2, int y2) const { context->fillTriangle(x0, y0, x1, y1, x2, y2, fillValue); }

void Graphics::fillTri(const Point<int> &p1, const Point<int> &p2, const Point<int> &p3) const { context->fillTriangle(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, fillValue); }

void Graphics::strokeTri(int x0, int y0, int x1, int y1, int x2, int y2) const { context->strokeTriangle(x0, y0, x1, y1, x2, y2, strokeValue, thickness); }

void Graphics::strokeTri(const Point<int> &p1, const Point<int> &p2, const Point<int> &p3) const { context->strokeTriangle(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, strokeValue, thickness); }

//----------[ FONT ]----------//

void Graphics::setFontFamily(const std::string &family) { this->family = family; }

void Graphics::setFontWeight(FontWeight weight) { weight = weight; }

void Graphics::setFontSlant(FontSlant slant) { slant = slant; }

void Graphics::setFontPoint(float pt) { this->pt = pt; }

void Graphics::setTextColor(rgba color) { textValue = color; }

void Graphics::setTextColor(const Color &color) { textValue = getCachedColor(color); }

void Graphics::setTextStyle(const TextStyle &style) {
    setFontFamily(style.family);
    setFontPoint(style.pt);
    setFontWeight(style.weight);
    setFontSlant(style.slant);

    if (style.color) setTextColor(*style.color);
}

//----------[ TEXT ]----------//

void Graphics::drawText(const std::string &text, int x, int y, Anchor anchor) { context->drawText(text.c_str(), x, y, family, pt, weight, slant, textValue, anchor); }

void Graphics::drawText(const std::string &text, Point<int> pos, Anchor anchor) { drawText(text, pos.x, pos.y, anchor); }

void Graphics::drawTextVertical(const std::string& text, int x, int y, Anchor anchor) {
    if (text.empty()) return;

    const int step = pt;
    if (step <= 0) return;

    int glyphCount = 0;
    for (char c : text) if (c != '\n') glyphCount++;
    if (glyphCount <= 0) return;

    const int blockH = glyphCount * step;

    Rect<int> box(x, y, 0, blockH);
    Point<int> start = Point<int>::getAnchored(box, anchor);

    int drawX = start.x;
    int drawY = start.y;

    Point<int> topP    = Point<int>::getAnchored(box, Anchor::TopLeft);
    Point<int> midP    = Point<int>::getAnchored(box, Anchor::CenterLeft);
    Point<int> bottomP = Point<int>::getAnchored(box, Anchor::BottomLeft);

    int dir = (start.y == bottomP.y) ? -1 : 1;

    if (start.y == midP.y) {
        drawY -= (blockH / 2);
        dir = 1;
    } else if (start.y == bottomP.y) {
        drawY -= step;
        dir = -1;
    }

    char buf[2] = {0, 0};

    for (char c : text) {
        if (c == '\n') continue;

        buf[0] = c;
        context->drawText(buf, drawX, drawY, family, pt, weight, slant, textValue, Anchor::TopLeft);

        drawY += dir * step;
    }
}

void Graphics::drawTextArea(const std::string& text, int x, int y, int width, int height, Anchor anchor, bool useEllipses, bool useHyphens) {
    if (width <= 0 || height <= 0 || text.empty()) return;

    Rect<int> rect(x, y, width, height);

    const int lineHeight = pt;
    if (lineHeight <= 0) return;

    const int maxLines = rect.height / lineHeight;
    if (maxLines <= 0) return;

    rgba textColor = fillValue;

    std::string scratch;
    scratch.reserve(128);

    auto measureString = [&](const std::string& s) -> int { return s.empty() ? 0 : context->getTextWidth(s.c_str(), family, pt, weight, slant); };

    auto measureRange = [&](int start, int len) -> int {
        if (len <= 0) return 0;
        scratch.assign(text.data() + start, (size_t)len);
        return context->getTextWidth(scratch.c_str(), family, pt, weight, slant);
    };

    const int n = (int)text.size();
    const int ellW = context->getTextWidth("...", family, pt, weight, slant);
    const int hyW  = context->getTextWidth("-", family, pt, weight, slant);

    std::vector<std::string> lines;
    lines.reserve(maxLines);

    int cursor = 0;

    auto skipSpaces = [&]() { while (cursor < n && text[cursor] == ' ') cursor++; };

    while (cursor < n && (int)lines.size() < maxLines) {
        if (text[cursor] == '\n') {
            lines.emplace_back("");
            cursor++;
            continue;
        }

        skipSpaces();
        if (cursor >= n) break;

        std::string line;

        while (cursor < n && text[cursor] != '\n') {
            int wordStart = cursor;
            while (cursor < n && text[cursor] != '\n' && text[cursor] != ' ') cursor++;
            int wordEnd = cursor;
            int wordLen = wordEnd - wordStart;

            while (cursor < n && text[cursor] == ' ') cursor++;

            std::string candidate = line;
            if (!candidate.empty()) candidate.push_back(' ');
            candidate.append(text.data() + wordStart, (size_t)wordLen);

            if (measureString(candidate) <= rect.width) {
                line.swap(candidate);
                continue;
            }

            if (!useHyphens) {
                cursor = wordStart;
                break;
            }

            std::string base = line;
            if (!base.empty()) base.push_back(' ');
            int baseW = measureString(base);

            if (baseW > rect.width) {
                cursor = wordStart;
                break;
            }

            int available = rect.width - baseW - hyW;
            if (available <= 0) {
                cursor = wordStart;
                break;
            }

            // Binary search largest prefix that fits
            int lo = 1;
            int hi = wordLen;
            int best = 0;

            while (lo <= hi) {
                int mid = (lo + hi) / 2;
                int w = measureRange(wordStart, mid);
                if (w <= available) {
                    best = mid;
                    lo = mid + 1;
                } else {
                    hi = mid - 1;
                }
            }

            if (best <= 0) {
                cursor = wordStart;
                break;
            }

            if (best < MIN_HYPHEN_PREFIX) {
                cursor = wordStart;
                break;
            }

            line = base;
            line.append(text.data() + wordStart, (size_t)best);
            if (best < wordLen) line.push_back('-');

            cursor = wordStart + best;
            break;
        }

        while (!line.empty() && line.back() == ' ') line.pop_back();
        lines.push_back(line);

        if (cursor < n && text[cursor] == '\n') cursor++;
    }

    const bool truncatedVertically = (cursor < n);

    if (truncatedVertically && useEllipses && !lines.empty()) {
        std::string& last = lines.back();

        if ((int)last.size() >= 3) {
            last.resize(last.size() - 3);
            last += "...";
        } else {
            last = "...";
        }

        while (measureString(last) > rect.width) {
            if ((int)last.size() <= 3) break;
            last.erase(last.size() - 4, 1);
        }

        if (measureString(last) > rect.width) last.clear();
    }

    enum class HAlign { Left, Center, Right };
    enum class VAlign { Top, Middle, Bottom };

    auto anchorToH = [&](Anchor a) -> HAlign {
        switch (a) {
            case Anchor::TopLeft:
            case Anchor::CenterLeft:
            case Anchor::BottomLeft:
                return HAlign::Left;

            case Anchor::TopCenter:
            case Anchor::Center:
            case Anchor::BottomCenter:
                return HAlign::Center;

            case Anchor::TopRight:
            case Anchor::CenterRight:
            case Anchor::BottomRight:
                return HAlign::Right;
        }
        return HAlign::Left;
    };

    auto anchorToV = [&](Anchor a) -> VAlign {
        switch (a) {
            case Anchor::TopLeft:
            case Anchor::TopCenter:
            case Anchor::TopRight:
                return VAlign::Top;

            case Anchor::CenterLeft:
            case Anchor::Center:
            case Anchor::CenterRight:
                return VAlign::Middle;

            case Anchor::BottomLeft:
            case Anchor::BottomCenter:
            case Anchor::BottomRight:
                return VAlign::Bottom;
        }
        return VAlign::Top;
    };

    HAlign hAlign = anchorToH(anchor);
    VAlign vAlign = anchorToV(anchor);

    const int visibleLines = (int)lines.size();
    if (visibleLines <= 0) return;

    const int blockHeight = visibleLines * lineHeight;

    int blockTop = rect.y;
    if (vAlign == VAlign::Middle) blockTop = rect.y + (rect.height - blockHeight) / 2;
    if (vAlign == VAlign::Bottom) blockTop = rect.y + (rect.height - blockHeight);

    for (int i = 0; i < visibleLines; ++i) {
        int drawY = blockTop + i * lineHeight;
        if (drawY + lineHeight > rect.y + rect.height) break;

        const std::string& line = lines[i];
        if (line.empty()) continue;

        int lineW = context->getTextWidth(line.c_str(), family, pt, weight, slant);

        int drawX = rect.x;
        if (hAlign == HAlign::Center) drawX = rect.x + (rect.width - lineW) / 2;
        if (hAlign == HAlign::Right)  drawX = rect.x + rect.width - lineW;

        context->drawText(line.c_str(), drawX, drawY, family, pt, weight, slant, textValue, anchor);
    }
}

void Graphics::drawTextArea(const std::string &text, Rect<int> rectangle, Anchor anchor, bool useEllipses, bool useHyphens) { drawTextArea(text, rectangle.x, rectangle.y, rectangle.width, rectangle.height, anchor, useEllipses, useHyphens); }

void Graphics::drawImage(const Image &image, int x, int y, Anchor anchor) const { context->drawImage(image, x, y, anchor); }

void Graphics::drawImage(const Image &image, Rect<int> rect) const { context->drawImage(image, rect.x, rect.y, rect.width, rect.height); }

void Graphics::setClipRect(int x, int y, int w, int h) { context->setClipRect(x, y, w, h); }

void Graphics::setClipRect(const Rect<int> &rect) { context->setClipRect(rect.x, rect.y, rect.width, rect.height); }

void Graphics::clearClipRect() { context->clearClipRect(); }

//----------[ CLIP ]----------//