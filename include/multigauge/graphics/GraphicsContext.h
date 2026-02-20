#pragma once

#include <multigauge/graphics/colors/Color.h>
#include <multigauge/geometry/alignment.h>

#include <multigauge/images/Image.h>

#include <multigauge/graphics/TextPaint.h>

class GraphicsContext {
    protected:
        int w, h;

    public:
        /// @brief Called once during initialization
        virtual bool init() { return true; }

        /// @brief Called before drawing a new frame
        virtual void beginFrame() {};

        /// @brief Called after all draw calls for the current frame are finished
        virtual void endFrame() {};

        /// @brief  Width of the drawing surface in pixels
        int width() const;

        /// @brief Height of the drawing surface in pixels 
        int height() const;
        
        //----------[ FILL ]----------//
        virtual void clear(rgba color) = 0;

        //----------[ PIXEL ]----------//
        virtual void pixel(int x, int y, rgba color) = 0;

        //----------[ LINE ]----------//
        virtual void line(int x0, int y0, int x1, int y1, rgba color, float thickness) = 0;

        //----------[ RECTANGLE ]----------//
        virtual void rect(int x, int y, int w, int h, rgba color) = 0;
        virtual void strokeRect(int x, int y, int w, int h, rgba color, float thickness) = 0;

        virtual void roundRect(int x, int y, int w, int h, float radius, rgba color) = 0;
        virtual void roundRect(int x, int y, int w, int h, float r1, float r2, float r3, float r4, rgba color) = 0;

        virtual void strokeRoundRect(int x, int y, int w, int h, float radius, rgba color, float thickness) = 0;
        virtual void strokeRoundRect(int x, int y, int w, int h, float r1, float r2, float r3, float r4, rgba color, float thickness) = 0;

        //----------[ CIRCLE ]----------//
        virtual void circle(int cx, int cy, int r, rgba color) = 0;
        virtual void strokeCircle(int cx, int cy, int r, rgba color, float thickness) = 0;

        //----------[ ELLIPSE ]----------//
        virtual void ellipse(int cx, int cy, int rx, int ry, rgba color) = 0;
        virtual void strokeEllipse(int cx, int cy, int rx, int ry, rgba color, float thickness) = 0;

        //----------[ RING ]----------//
        virtual void ring(int cx, int cy, int r1, int r2, rgba color) = 0;
        virtual void strokeRing(int cx, int cy, int r1, int r2, rgba color, float thickness) = 0;

        //----------[ ARC ]----------//
        virtual void arc(int cx, int cy, int r1, int r2, float start, float end, rgba color) = 0;
        virtual void strokeArc(int cx, int cy, int r1, int r2, float start, float end, rgba color, float thickness) = 0;

        //----------[ TRIANGLE ]----------//
        virtual void tri(int x0, int y0, int x1, int y1, int x2, int y2, rgba color) = 0;
        virtual void strokeTri(int x0, int y0, int x1, int y1, int x2, int y2, rgba color, float thickness) = 0;

        //----------[ TEXT ]----------//
        virtual float getTextWidth(const char* text, std::string family, float pt, FontWeight weight, FontSlant slant) = 0;
        float getTextWidth(std::string_view text, std::string family, float pt, FontWeight weight, FontSlant slant);

        virtual void drawText(const char* text, int x, int y, std::string family, float pt, FontWeight weight, FontSlant slant, rgba color, Anchor anchor = Anchor::TopLeft) = 0;

        //----------[ IMAGE ]----------//
        virtual Image createNativeImage(const rgba* pixels, int w, int h) = 0;

        virtual void drawImage(const Image& img, int x, int y) = 0;
        virtual void drawImageRotated(const Image& img, int x, int y, float angle, int pivotX, int pivotY) = 0;
        virtual void drawImageScaled(const Image& img, int x, int y, float sx, float sy) = 0;
        virtual void drawImageTransformed(const Image& img, int x, int y, float angle, float sx, float sy, int pivotX, int pivotY) = 0;

        virtual void drawImageStretched(const Image& img, int x, int y, int width, int height) = 0;
        virtual void drawImageRegion(const Image& img, int srcX, int srcY, int srcW, int srcH, int destX, int destY, int destW, int destH) = 0;
        
        //----------[ CLIP ]----------//
        virtual void clip(int x, int y, int width, int height) = 0;
        virtual void clearClip() = 0;
};