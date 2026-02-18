#pragma once

#include "colors/Color.h"
#include "geometry/alignment.h"
#include "geometry/Rect.h"
#include "geometry/point.h"
#include "geometry/Line.h"
#include "geometry/path.h"

#include "GraphicsContext.h"
#include "images/Image.h"
#include "TextStyle.h"

#define MIN_HYPHEN_PREFIX 3 // Prefix must be this number of chars or higher to be hyphenated in text wrap

class Graphics final {
    private:
        GraphicsContext* context;

        //----------[ COLORS ]----------//

        rgba fillValue;
        rgba strokeValue;
        rgba textValue;

        float thickness = 0.0f; // for stroke

        /// @brief Cache that stores a color's value for quick lookup
        std::unordered_map<const Color*, rgba> colorCache;

        /// @brief Gets a color's value from the cache.
        /// @param color The Color object to get the color value of
        /// @return The current Color object's 16-bit color value
        rgba getCachedColor(const Color& color);

        //----------[ Font ]----------//
        std::string family = "default";
        FontWeight weight = FontWeight::Normal;
        FontSlant slant = FontSlant::Normal;
        float pt = 16.0f;

    public:
        Graphics(GraphicsContext* context);

        Rect<int> getScreenBounds();
        
        //----------[ COLOR ]----------//

        void clearColorCache();

        void setFill(rgba color);
        void setFill(const Color& color);

        void setStroke(rgba color);
        void setStroke(const Color& color);
        void setStrokeThickness(float thickness);

        void setFillStroke(const FillStroke& fillStroke);

        //----------[ FILL ]----------//
        void fillAll() const;
        void fillAll(const Color& color);
        void fillAll(rgba color) const;

        //----------[ PIXEL ]----------//
        void fillPixel(int x, int y) const;
        void fillPixel(const Point<int>& pos) const;

        //----------[ LINE ]----------//
        void strokeLine(int x0, int y0, int x1, int y1) const;
        void strokeLine(const Point<int>& p1, const Point<int>& p2) const;
        void strokeLine(const Line<int>& line) const;

        void fillWideLine(int x0, int y0, int x1, int y1, float thickness) const;
        void fillWideLine(const Point<int>& p1, const Point<int>& p2, float thickness) const;
        void fillWideLine(const Line<int>& line, float thickness) const;

        void strokeWideLine(int x0, int y0, int x1, int y1, float thickness) const;
        void strokeWideLine(const Point<int>& p1, const Point<int>& p2, float thickness) const;
        void strokeWideLine(const Line<int>& line, float thickness) const;

        //----------[ RECTANGLE ]----------//
        void fillRect(int x, int y, int width, int height) const;
        void fillRect(const Rect<int>& rect) const;

        void strokeRect(int x, int y, int width, int height) const;
        void strokeRect(const Rect<int>& rect) const;

        void fillRoundedRect(int x, int y, int width, int height, float radius) const;
        void fillRoundedRect(const Rect<int>& rect, float radius) const;
        void fillRoundedRect(int x, int y, int width, int height, int topLeft, int topRight, int bottomRight, int bottomLeft) const;
        void fillRoundedRect(const Rect<int>& rect, int topLeft, int topRight, int bottomRight, int bottomLeft) const;

        void strokeRoundedRect(int x, int y, int width, int height, float radius) const;
        void strokeRoundedRect(const Rect<int>& rect, float radius) const;
        void strokeRoundedRect(int x, int y, int width, int height, int topLeft, int topRight, int bottomRight, int bottomLeft) const;
        void strokeRoundedRect(const Rect<int>& rect, int topLeft, int topRight, int bottomRight, int bottomLeft) const;

        //----------[ ELLIPSE ]----------//
        void fillEllipse(int cx, int cy, int rx, int ry) const;
        void fillEllipse(const Point<int>& center, int rx, int ry) const;

        void fillEllipseInRect(int x, int y, int width, int height) const;
        void fillEllipseInRect(const Rect<int>& area) const;

        void strokeEllipse(int cx, int cy, int rx, int ry) const;
        void strokeEllipse(const Point<int>& center, int rx, int ry) const;

        void strokeEllipseInRect(int x, int y, int width, int height) const;
        void strokeEllipseInRect(const Rect<int>& area) const;

        //----------[ CIRCLE ]----------//
        void fillCircle(int cx, int cy, int radius) const;
        void fillCircle(const Point<int>& center, int radius) const;

        void fillCircleInRect(int x, int y, int width, int height) const;
        void fillCircleInRect(const Rect<int>& area) const;

        void strokeCircle(int cx, int cy, int radius) const;
        void strokeCircle(const Point<int>& center, int radius) const;

        void strokeCircleInRect(int x, int y, int width, int height) const;
        void strokeCircleInRect(const Rect<int>& area) const;

        //----------[ RING ]----------//
        void fillRing(int cx, int cy, int r1, int r2) const;
        void fillRing(const Point<int>& center, int r1, int r2) const;

        void strokeRing(int cx, int cy, int r1, int r2) const;
        void strokeRing(const Point<int>& center, int r1, int r2) const;

        //----------[ ARC ]----------//
        void fillArc(int cx, int cy, int r1, int r2, float startAngle, float endAngle) const;
        void fillArc(const Point<int>& center, int r1, int r2, float startAngle, float endAngle) const;

        void strokeArc(int cx, int cy, int r1, int r2, float startAngle, float endAngle) const;
        void strokeArc(const Point<int>& center, int r1, int r2, float startAngle, float endAngle) const;

        //----------[ TRIANGLE ]----------//
        void fillTri(int x0, int y0, int x1, int y1, int x2, int y2) const;
        void fillTri(const Point<int>& p1, const Point<int>& p2, const Point<int>& p3) const;

        void strokeTri(int x0, int y0, int x1, int y1, int x2, int y2) const;
        void strokeTri(const Point<int>& p1, const Point<int>& p2, const Point<int>& p3) const;

        //----------[ PATH ]----------//
        void fillPath(const Path<int>& path) const;
        void strokePath(const Path<int>& path) const;

        //----------[ FONT ]----------//
        void setFontFamily(const std::string& family);
        void setFontWeight(FontWeight weight);
        void setFontSlant(FontSlant slant);
        void setFontPoint(float pt);

        void setTextColor(rgba color);
        void setTextColor(const Color& color);

        void setTextStyle(const TextStyle& style);

        //----------[ TEXT ]----------//
        void drawText(const std::string& text, int x, int y, Anchor anchor);
        void drawText(const std::string& text, Point<int> pos, Anchor anchor);
        void drawTextVertical(const std::string& text, int x, int y, Anchor anchor);

        void drawTextArea(const std::string& text, int x, int y, int width, int height, Anchor anchor, bool useEllipses = true, bool useHyphens = false);
        void drawTextArea(const std::string& text, Rect<int> rectangle, Anchor anchor, bool useEllipses = true, bool useHyphens = false);

        //----------[ IMAGES ]----------//
        void drawImage(const Image& image, int x, int y, Anchor anchor) const;
        void drawImage(const Image& image, Rect<int> rectangle) const;
        
        //----------[ CLIP ]----------//
        void setClipRect(int x, int y, int width, int height);
        void setClipRect(const Rect<int>& rect);
        void clearClipRect();
};