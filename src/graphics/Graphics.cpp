#include <multigauge/graphics/Graphics.h>

#include <algorithm>
#include <cmath>

namespace mg::graphics {

rgba Graphics::resolveColor(const Color& color) noexcept { return colorFrame.resolve(color); }

Graphics::Graphics(GraphicsContext& context) : context(&context) { }

Rect<int> Graphics::getScreenBounds() { return Rect<int>(0, 0, context->width(), context->height()); }

//----------[ COLOR ]----------//

void Graphics::beginFrame(const ColorFrame& frame) noexcept { colorFrame = colorResolver.beginFrame(frame); }
void Graphics::endFrame() noexcept { colorFrame = {}; }
rgba Graphics::resolve(const Color& color) const noexcept { return colorFrame.resolve(color); }

void Graphics::setFill(rgba color) {
    fill.enabled = true;
    fill.value = color;
}

void Graphics::setFill(const Color *color) {
    if (color) setFill(resolveColor(*color));
    else fill.enabled = false;
}

void Graphics::setStroke(rgba color, float t) {
    stroke.enabled = true;
    stroke.value = color;
    thickness = std::max(0.0f, t);
}

void Graphics::setStroke(const Color *color, float t) {
    if (color) setStroke(resolveColor(*color), t);
    else stroke.enabled = false;
}

void Graphics::setStrokeThickness(float t) { thickness = t; }

void Graphics::setPaint(rgba f, rgba s, float t) {
    setFill(f);
    setStroke(s, t);
}

void Graphics::setPaint(const Color *f, const Color *s, float t) {
    setFill(f);

    setStroke(s, t);
}

void Graphics::setPaint(const Paint &paint) { setPaint(paint.fill.get(), paint.stroke.get(), paint.thickness); }

void Graphics::setPaint(const ResolvedPaint& paint) {
    if (paint.fillEnabled) setFill(paint.fill); else fill.enabled = false;
    if (paint.strokeEnabled) setStroke(paint.stroke, paint.thickness); else stroke.enabled = false;
}

//----------[ FILL ]----------//

void Graphics::fillAll() const { if (fill.enabled) context->clear(fill.value); }

void Graphics::fillAll(rgba color) const { context->clear(color); }

void Graphics::fillAll(const Color* color) { 
    if (color) fillAll(resolveColor(*color));
    else fillAll();
}

//----------[ PIXEL ]----------//

void Graphics::drawPixel(int x, int y) const { if (fill.enabled) context->pixel(x, y, fill.value); }

void Graphics::drawPixel(const Point<int> &pos) const { drawPixel(pos.x, pos.y); }

//----------[ LINE ]----------//

void Graphics::drawLine(int x0, int y0, int x1, int y1, float t) const {
    if (fill.enabled) context->line(x0, y0, x1, y1, fill.value, t);
    if (stroke.enabled) context->line(x0, y0, x1, y1, stroke.value, thickness);
}

void Graphics::drawLine(const Point<int> &p1, const Point<int> &p2, float t) const { drawLine(p1.x, p1.y, p2.x, p2.y, t); }

void Graphics::drawLine(const Line<int> &line, float t) const { drawLine(line.p1, line.p2, t); }

//----------[ RECTANGLE ]----------//

void Graphics::drawRect(int x, int y, int w, int h) const { 
    if (fill.enabled) context->rect(x, y, w, h, fill.value);
    if (stroke.enabled) context->strokeRect(x, y, w, h, stroke.value, thickness);
}

void Graphics::drawRect(const Rect<int> &rectangle) const { drawRect(rectangle.x, rectangle.y, rectangle.width, rectangle.height); }

void Graphics::drawRoundedRect(int x, int y, int w, int h, float radius) const {
    if (fill.enabled) context->roundRect(x, y, w, h, radius, fill.value);
    if (stroke.enabled) context->strokeRoundRect(x, y, w, h, radius, stroke.value, thickness);
}

void Graphics::drawRoundedRect(const Rect<int> &rect, float radius) const { drawRoundedRect(rect.x, rect.y, rect.width, rect.height, radius); }

void Graphics::drawRoundedRect(int x, int y, int width, int height, int topLeft, int topRight, int bottomRight, int bottomLeft) const
{
}

void Graphics::drawRoundedRect(const Rect<int> &rect, int topLeft, int topRight, int bottomRight, int bottomLeft) const
{
}

//----------[ ELLIPSE ]----------//

void Graphics::drawEllipse(int cx, int cy, int rx, int ry) const {
    if (fill.enabled) context->ellipse(cx, cy, rx, ry, fill.value);
    if (stroke.enabled) context->strokeEllipse(cx, cy, rx, ry, stroke.value, thickness);
}

void Graphics::drawEllipse(const Point<int> &center, int rx, int ry) const { drawEllipse(center.x, center.y, rx, ry); }

void Graphics::drawEllipseInRect(int x, int y, int w, int h) const { drawEllipseInRect(Rect<int>(x, y, w, h)); }

void Graphics::drawEllipseInRect(const Rect<int> &area) const {
    const auto center = area.getCenter();
    drawEllipse(center, area.width / 2, area.height / 2);
}

//----------[ CIRCLE ]----------//

void Graphics::drawCircle(int cx, int cy, int radius) const {
    if (fill.enabled) context->circle(cx, cy, radius, fill.value);
    if (stroke.enabled) context->strokeCircle(cx, cy, radius, stroke.value, thickness);
}

void Graphics::drawCircle(const Point<int> &center, int radius) const { drawCircle(center.x, center.y, radius); }

void Graphics::drawCircleInRect(int x, int y, int w, int h) const { drawCircleInRect(Rect<int>(x, y, w, h)); }

void Graphics::drawCircleInRect(const Rect<int> &area) const {
    const auto center = area.getCenter();
    drawCircle(center, std::min(area.width / 2, area.height / 2));
}

//----------[ RING ]----------//

void Graphics::drawRing(int cx, int cy, int r1, int r2) const {
    if (fill.enabled) context->ring(cx, cy, r1, r2, fill.value);
    if (stroke.enabled) context->strokeRing(cx, cy, r1, r2, stroke.value, thickness);
}

void Graphics::drawRing(const Point<int> &center, int r1, int r2) const { drawRing(center.x, center.y, r1, r2); }

//----------[ ARC ]----------//

void Graphics::drawArc(int cx, int cy, int r1, int r2, float startAngle, float endAngle) const {
    if (fill.enabled) context->arc(cx, cy, r1, r2, startAngle, endAngle, fill.value);
    if (stroke.enabled) context->strokeArc(cx, cy, r1, r2, startAngle, endAngle, stroke.value, thickness);
}

void Graphics::drawArc(const Point<int> &center, int r1, int r2, float startAngle, float endAngle) const { drawArc(center.x, center.y, r1, r2, startAngle, endAngle); }

//----------[ TRIANGLE ]----------//

void Graphics::drawTri(int x0, int y0, int x1, int y1, int x2, int y2) const {
    if (fill.enabled) context->tri(x0, y0, x1, y1, x2, y2, fill.value); 
    if (stroke.enabled) context->strokeTri(x0, y0, x1, y1, x2, y2, stroke.value, thickness);
}

void Graphics::drawTri(const Point<int> &p1, const Point<int> &p2, const Point<int> &p3) const { drawTri(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y); }

//----------[ FONT ]----------//

void Graphics::setFontFamily(const std::string &family) { this->family = family; }

void Graphics::setFontWeight(FontWeight weight) { this->weight = weight; }

void Graphics::setFontSlant(FontSlant slant) { this->slant = slant; }

void Graphics::setFontPoint(float pt) { this->pt = pt; }

void Graphics::setTextColor(rgba color) {
    textColor.enabled = true;
    textColor.value = color;
}

void Graphics::setTextColor(const Color *color) { 
    if (color) setTextColor(resolveColor(*color));
    else textColor.enabled = false;
}

void Graphics::setTextPaint(rgba color, const std::string &family, FontWeight weight, FontSlant slant, float pt) {
    setTextColor(color);
    setFontFamily(family);
    setFontPoint(pt);
    setFontWeight(weight);
    setFontSlant(slant);
}

void Graphics::setTextPaint(const Color *color, const std::string &family, FontWeight weight, FontSlant slant, float pt) {
    setTextColor(color);
    setFontFamily(family);
    setFontPoint(pt);
    setFontWeight(weight);
    setFontSlant(slant);
}

void Graphics::setTextPaint(const TextPaint &style) { setTextPaint(style.color.get(), style.family, style.weight, style.slant, style.pt); }

//----------[ TEXT ]----------//

void Graphics::drawText(const std::string &text, int x, int y, Anchor anchor) { if (textColor.enabled) context->drawText(text.c_str(), x, y, family, pt, weight, slant, textColor.value, anchor); }

void Graphics::drawText(const std::string &text, Point<int> pos, Anchor anchor) { drawText(text, pos.x, pos.y, anchor); }

void Graphics::drawTextVertical(const std::string& text, int x, int y, Anchor anchor) {
    if (!textColor.enabled) return;
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
        context->drawText(buf, drawX, drawY, family, pt, weight, slant, textColor.value, Anchor::TopLeft);

        drawY += dir * step;
    }
}

void Graphics::drawTextArea(const std::string& text, int x, int y, int width, int height, Anchor anchor, bool useEllipses, bool useHyphens) {
    if (!textColor.enabled) return;
    if (width <= 0 || height <= 0 || text.empty()) return;

    Rect<int> rect(x, y, width, height);

    const int lineHeight = pt;
    if (lineHeight <= 0) return;

    const int maxLines = rect.height / lineHeight;
    if (maxLines <= 0) return;

    rgba textColor = this->textColor.value;

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

        context->drawText(line.c_str(), drawX, drawY, family, pt, weight, slant, this->textColor.value, Anchor::TopLeft);
    }
}

void Graphics::drawTextArea(const std::string &text, Rect<int> rectangle, Anchor anchor, bool useEllipses, bool useHyphens) { drawTextArea(text, rectangle.x, rectangle.y, rectangle.width, rectangle.height, anchor, useEllipses, useHyphens); }

//----------[ IMAGE ]----------//

void Graphics::drawImage(const Image &image, int x, int y, Anchor anchor) const { 
    if (image.empty()) return;
    auto pos = Point<int>::getAnchored(x, y, image.width, image.height, anchor);
    context->drawImage(image, pos.x, pos.y); 
}

void Graphics::drawImage(const Image &image, Point<int> pos, Anchor anchor) const { drawImage(image, pos.x, pos.y, anchor); }

void Graphics::drawImageRotated(const Image &image, int x, int y, float angle, Anchor anchor, int pivotX, int pivotY) const {
    if (image.empty()) return;
    auto pos = Point<int>::getAnchored(x, y, image.width, image.height, anchor);
    context->drawImageRotated(image, pos.x, pos.y, angle, pivotX, pivotY);
}

void Graphics::drawImageRotated(const Image &image, Point<int> pos, float angle, Anchor anchor, Point<int> pivot) const { drawImageRotated(image, pos.x, pos.y, angle, anchor, pivot.x, pivot.y); }

void Graphics::drawImageArea(const Image &image, int x, int y, int width, int height, ImageFit fit) const { drawImageArea(image, Rect<int>(x, y, width, height), fit); }

void Graphics::drawImageArea(const Image &image, Rect<int> rect, ImageFit fit) const {
    if (rect.width <= 0 || rect.height <= 0 || image.empty()) return;

    switch(fit) {
        case ImageFit::Fill: {
            const int iw = image.width;
            const int ih = image.height;

            const int64_t lhs = (int64_t)rect.width  * ih;
            const int64_t rhs = (int64_t)rect.height * iw;

            int srcX = 0, srcY = 0, srcW = iw, srcH = ih;

            if (lhs > rhs) {
                srcH = (int)((int64_t)iw * rect.height / rect.width);
                if (srcH < 1) srcH = 1;
                if (srcH > ih) srcH = ih;
                srcY = (ih - srcH) / 2;
            } else if (lhs < rhs) {
                srcW = (int)((int64_t)ih * rect.width / rect.height);
                if (srcW < 1) srcW = 1;
                if (srcW > iw) srcW = iw;
                srcX = (iw - srcW) / 2;
            }

            context->drawImageRegion(
                image,
                srcX, srcY, srcW, srcH,
                rect.x, rect.y, rect.width, rect.height
            );
            return;
        }

        case ImageFit::Fit: {
            const int iw = image.width;
            const int ih = image.height;

            const int64_t lhs = (int64_t)rect.width * ih;
            const int64_t rhs = (int64_t)rect.height * iw;

            int drawW = 0;
            int drawH = 0;

            if (lhs <= rhs) {
                drawW = rect.width;
                drawH = (int)((int64_t)rect.width * ih / iw);
            } else {
                drawH = rect.height;
                drawW = (int)((int64_t)rect.height * iw / ih);
            }

            const int dx = rect.x + (rect.width  - drawW) / 2;
            const int dy = rect.y + (rect.height - drawH) / 2;

            context->drawImageStretched(image, dx, dy, drawW, drawH);
            return;
        }

        case ImageFit::Stretch: {
            context->drawImageStretched(image, rect.x, rect.y, rect.width, rect.height);
            return;
        }
    }
}

void Graphics::drawImageRegion(const Image &image, int srcX, int srcY, int srcW, int srcH, int destX, int destY, int destW, int destH) const { if (!image.empty()) context->drawImageRegion(image, srcX, srcY, srcW, srcH, destX, destY, destW, destH); }

void Graphics::drawImageRegion(const Image &image, Rect<int> srcRect, Rect<int> destRect) const { drawImageRegion(image, srcRect.x, srcRect.y, srcRect.width, srcRect.height, destRect.x, destRect.y, destRect.width, destRect.height); }

//----------[ CLIP ]----------//

void Graphics::setClip(int x, int y, int w, int h) { context->clip(x, y, w, h); }

void Graphics::setClip(const Rect<int> &rect) { context->clip(rect.x, rect.y, rect.width, rect.height); }

void Graphics::clearClip() { context->clearClip(); }

} // namespace mg::graphics
