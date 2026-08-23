#pragma once

#include <multigauge/graphics/geometry/alignment.h>
#include <multigauge/graphics/geometry/Rect.h>
#include <multigauge/graphics/geometry/Point.h>
#include <multigauge/graphics/geometry/Line.h>

#include <multigauge/graphics/GraphicsContext.h>
#include <multigauge/graphics/image/Image.h>
#include <multigauge/graphics/TextPaint.h>
#include <multigauge/graphics/colors/ColorTimeline.h>

#include <string>
#include <cstddef>

#define MIN_HYPHEN_PREFIX 3 // Prefix must be this number of chars or higher to be hyphenated in text wrap

namespace mg::graphics {

using ::mg::Anchor;
using ::mg::Line;
using ::mg::Point;
using ::mg::Rect;
using ::mg::images::Image;

struct PaintState {
    bool enabled = false;
    rgba value{};
};

class Graphics final {
    private:
        GraphicsContext* context;

        //----------[ COLORS ]----------//

        PaintState fill;
        PaintState stroke;
        PaintState textColor;

        float thickness = 0.0f; // for stroke

        ColorResolver colorResolver;
        ColorResolver::Frame colorFrame;

        rgba resolveColor(const Color& color) noexcept;

        //----------[ Font ]----------//
        std::string family = "default";
        FontWeight weight = FontWeight::Normal;
        FontSlant slant = FontSlant::Normal;
        float pt = 16.0f;


    public:
        Graphics(GraphicsContext& context);

        /// @brief Returns the full rawable bounds of the current target (in pixels).
        Rect<int> getScreenBounds();

        /// Starts color resolution for one immutable render frame. Color based
        /// drawing before this call deterministically resolves to transparent.
        void beginFrame(const ColorFrame& frame) noexcept;
        void endFrame() noexcept;
        /// Reserves color-resolution storage outside rendering for packages
        /// that use more than the default number of colors per frame.
        void reserveColorCache(std::size_t colorCount) { colorResolver.reserve(colorCount); }
        [[nodiscard]] rgba resolve(const Color& color) const noexcept;
        [[nodiscard]] const ColorResolver::Frame& colorFrameToken() const noexcept { return colorFrame; }
        
        //----------[ COLOR ]----------//

        /// @brief Sets the current fill color.
        void setFill(rgba color);
        /// @brief Sets the fill color. Passing `nullptr` disables fill.
        void setFill(const Color* color = nullptr);

        /// @brief Sets the current stroke color and thickness (in pixels).
        void setStroke(rgba color, float t = 1.0f);
        /// @brief Sets the stroke color and thickness (in pixels). Passing `nullptr` disables stroke.
        void setStroke(const Color* color = nullptr, float t = 1.0f);
        /// @brief Sets the stroke thickness (in pixels).
        void setStrokeThickness(float thickness);

        /// @brief Sets the current fill and stroke colors and stroke thickness (in pixels).
        void setPaint(rgba fill, rgba stroke, float thickness = 1.0f);
        /// @brief Sets the current fill and stroke colors and stroke thickness (in pixels). Passing `nullptr` for either color disables that color.
        void setPaint(const Color* fill = nullptr, const Color* stroke = nullptr, float thickness = 1.0f);
        /// @brief Sets the current fill and stroke colors and stroke thickness (in pixels).
        void setPaint(const Paint& paint);
        void setPaint(const ResolvedPaint& paint);

        //----------[ FILL ]----------//

        /// @brief Fills the entire drawing surface with the current fill color.
        void fillAll() const;
        /// @brief Fills the entire drawing surface with the specified color.
        void fillAll(rgba color) const;
        /// @brief Fills the entire drawing surface with the specified color. Passing `nullptr` uses the current fill color, if enabled.
        void fillAll(const Color* color);

        //----------[ PIXEL ]----------//
        void drawPixel(int x, int y) const;
        void drawPixel(const Point<int>& pos) const;

        //----------[ LINE ]----------//
        void drawLine(int x0, int y0, int x1, int y1, float thickness = 1.0f) const;
        void drawLine(const Point<int>& p1, const Point<int>& p2, float thickness = 1.0f) const;
        void drawLine(const Line<int>& line, float thickness = 1.0f) const;

        //----------[ RECTANGLE ]----------//
        void drawRect(int x, int y, int width, int height) const;
        void drawRect(const Rect<int>& rect) const;

        void drawRoundedRect(int x, int y, int width, int height, float radius) const;
        void drawRoundedRect(const Rect<int>& rect, float radius) const;
        void drawRoundedRect(int x, int y, int width, int height, int topLeft, int topRight, int bottomRight, int bottomLeft) const;
        void drawRoundedRect(const Rect<int>& rect, int topLeft, int topRight, int bottomRight, int bottomLeft) const;

        //----------[ ELLIPSE ]----------//
        void drawEllipse(int cx, int cy, int rx, int ry) const;
        void drawEllipse(const Point<int>& center, int rx, int ry) const;

        void drawEllipseInRect(int x, int y, int width, int height) const;
        void drawEllipseInRect(const Rect<int>& area) const;

        //----------[ CIRCLE ]----------//
        void drawCircle(int cx, int cy, int radius) const;
        void drawCircle(const Point<int>& center, int radius) const;

        void drawCircleInRect(int x, int y, int width, int height) const;
        void drawCircleInRect(const Rect<int>& area) const;

        //----------[ RING ]----------//
        void drawRing(int cx, int cy, int r1, int r2) const;
        void drawRing(const Point<int>& center, int r1, int r2) const;

        //----------[ ARC ]----------//
        void drawArc(int cx, int cy, int r1, int r2, float startAngle, float endAngle) const;
        void drawArc(const Point<int>& center, int r1, int r2, float startAngle, float endAngle) const;

        //----------[ TRIANGLE ]----------//
        void drawTri(int x0, int y0, int x1, int y1, int x2, int y2) const;
        void drawTri(const Point<int>& p1, const Point<int>& p2, const Point<int>& p3) const;

        //----------[ PATH ]----------//

        //----------[ FONT ]----------//

        /// @brief Sets the current font family.
        void setFontFamily(const std::string& family);
        /// @brief Sets the current font weight.
        void setFontWeight(FontWeight weight);
        /// @brief Sets the current font slant.
        void setFontSlant(FontSlant slant);
        /// @brief Sets the current font size in points (pt).
        void setFontPoint(float pt);

        /// @brief Sets the current text color.
        void setTextColor(rgba color);
        /// @brief Sets the current text color. Passing `nullptr` disables text.
        void setTextColor(const Color* color = nullptr);

        /// @brief Sets the current text color and font settings.
        void setTextPaint(rgba color, const std::string& family, FontWeight weight, FontSlant slant, float pt);
        /// @brief Sets the current text color and font settings.
        void setTextPaint(const Color* color, const std::string& family, FontWeight weight, FontSlant slant, float pt);
        /// @brief Sets the current text color and font settings.
        void setTextPaint(const TextPaint& style);

        //----------[ TEXT ]----------//

        /// @brief Draws a single horizontal line of text.
        void drawText(const std::string& text, int x, int y, Anchor anchor);
        /// @brief Draws a single horizontal line of text.
        void drawText(const std::string& text, Point<int> pos, Anchor anchor);
        /// @brief Draws a single vertical text (each character on a new line).
        void drawTextVertical(const std::string& text, int x, int y, Anchor anchor);

        /// @brief Draws multi-line text wrapped withing a specific rectangle.
        void drawTextArea(const std::string& text, int x, int y, int width, int height, Anchor anchor, bool useEllipses = true, bool useHyphens = false);
        /// @brief Draws multi-line text wrapped withing a specific rectangle.
        void drawTextArea(const std::string& text, Rect<int> rectangle, Anchor anchor, bool useEllipses = true, bool useHyphens = false);

        /// @brief Draws multi-line text scaled to fit within a specific rectangle.
        void drawTextFit(const std::string& text, int x, int y, int width, int height, Anchor anchor, bool useEllipses = true, bool useHyphens = false  );
        /// @brief Draws multi-line text scaled to fit within a specific rectangle.
        void drawTextFit(const std::string& text, Rect<int> rectangle, Anchor anchor, bool useEllipses = true, bool useHyphens = false);

        /// @brief Measures the width and height of a block of text if it were to be drawn with the current font settings.
        Rect<int> measureText(const std::string& text) const;

        //----------[ IMAGES ]----------//

        /// @brief Draws an image at a specific position.
        void drawImage(const Image& image, int x, int y, Anchor anchor) const;
        /// @brief Draws an image at a specific position.
        void drawImage(const Image& image, Point<int> pos, Anchor anchor) const;

        /// @brief Draws an image rotated by a specific angle (in degrees) around a pivot point (relative to image).
        void drawImageRotated(const Image& image, int x, int y, float angle, Anchor anchor, int pivotX, int pivotY) const;
        /// @brief Draws an image rotated by a specific angle (in degrees) around a pivot point (relative to image).
        void drawImageRotated(const Image& image, Point<int> pos, float angle, Anchor anchor, Point<int> pivot) const;

        enum class ImageFit { 
            Fill, // Preserves image aspect ratio, may crop image
            Fit, // Preserves image aspect ratio, may leave empty space
            Stretch // Stretches image to fill area
        };

        /// @brief Draws an image fit within a specific rectangle.
        void drawImageArea(const Image& image, int x, int y, int width, int height, ImageFit fit = ImageFit::Fit) const;
        /// @brief Draws an image fit within a specific rectangle.
        void drawImageArea(const Image& image, Rect<int> rectangle, ImageFit fit = ImageFit::Fit) const;

        /// @brief Draws a region of an image fit within a specific rectangle.
        void drawImageRegion(const Image& image, int srcX, int srcY, int srcW, int srcH, int destX, int destY, int destW, int destH) const;
        /// @brief Draws a region of an image fit within a specific rectangle.
        void drawImageRegion(const Image& image, Rect<int> srcRect, Rect<int> destRect) const;

        //----------[ CLIP ]----------//

        /// @brief Sets the clipping region to a specific rectangle. Only drawing operations that intersect with this region will be visible.
        void setClip(int x, int y, int width, int height);
        /// @brief Sets the clipping region to a specific rectangle. Only drawing operations that intersect with this region will be visible.
        void setClip(const Rect<int>& rect);
        /// @brief Resets the clipping region, allowing all drawing operations to be visible again.
        void clearClip();
};

} // namespace mg::graphics
