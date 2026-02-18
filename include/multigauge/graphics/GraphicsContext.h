#pragma once

#include <multigauge/graphics/colors/Color.h>
#include <multigauge/geometry/alignment.h>

#include <multigauge/images/Image.h>

#include <multigauge/graphics/TextStyle.h>

class GraphicsContext {
    protected:
        int width;
        int height;

    public:
        /// @brief Called once during initialization
        virtual bool init() { return true; }

        /// @brief Called before drawing a new frame
        virtual void beginFrame() {};

        /// @brief Called after all draw calls for the current frame are finished
        virtual void endFrame() {};

        int getScreenWidth() const;
        int getScreenHeight() const;
        
        //----------[ FILL ]----------//
        virtual void fillAll(rgba color) = 0;

        //----------[ PIXEL ]----------//
        virtual void drawPixel(int x, int y, rgba color) = 0;

        //----------[ LINE ]----------//
        virtual void strokeLine(int x0, int y0, int x1, int y1, rgba color) = 0;
        virtual void fillWideLine(int x0, int y0, int x1, int y1, rgba color, float thickness) = 0;

        //----------[ RECTANGLE ]----------//
        virtual void fillRect(int x, int y, int w, int h, rgba color) = 0;
        virtual void strokeRect(int x, int y, int w, int h, rgba color, float thickness) = 0;

        virtual void fillRoundRect(int x, int y, int w, int h, float radius, rgba color) = 0;
        virtual void fillRoundRect(int x, int y, int w, int h, float r1, float r2, float r3, float r4, rgba color) = 0;

        virtual void strokeRoundRect(int x, int y, int w, int h, float radius, rgba color, float thickness) = 0;
        virtual void strokeRoundRect(int x, int y, int w, int h, float r1, float r2, float r3, float r4, rgba color, float thickness) = 0;

        //----------[ CIRCLE ]----------//
        virtual void fillCircle(int cx, int cy, int r, rgba color) = 0;
        virtual void strokeCircle(int cx, int cy, int r, rgba color, float thickness) = 0;

        //----------[ ELLIPSE ]----------//
        virtual void fillEllipse(int cx, int cy, int rx, int ry, rgba color) = 0;
        virtual void strokeEllipse(int cx, int cy, int rx, int ry, rgba color, float thickness) = 0;

        //----------[ RING ]----------//
        virtual void fillRing(int cx, int cy, int r, rgba color) = 0;
        virtual void strokeRing(int cx, int cy, int r, rgba color, float thickness) = 0;

        //----------[ ARC ]----------//
        virtual void fillArc(int cx, int cy, int r1, int r2, float start, float end, rgba color) = 0;
        virtual void strokeArc(int cx, int cy, int r1, int r2, float start, float end, rgba color, float thickness) = 0;

        //----------[ TRIANGLE ]----------//
        virtual void strokeTriangle(int x0, int y0, int x1, int y1, int x2, int y2, rgba color, float thickness) = 0;
        virtual void fillTriangle(int x0, int y0, int x1, int y1, int x2, int y2, rgba color) = 0;

        //----------[ TEXT ]----------//
        virtual float getTextWidth(const char* text, std::string family, float pt, FontWeight weight, FontSlant slant) = 0;
        float getTextWidth(std::string_view text, std::string family, float pt, FontWeight weight, FontSlant slant);

        virtual void drawText(const char* text, int x, int y, std::string family, float pt, FontWeight weight, FontSlant slant, rgba color, Anchor anchor = Anchor::TopLeft) = 0;

        //----------[ IMAGE ]----------//
        virtual Image createNativeImage(const rgba* pixels, int w, int h) = 0;

        virtual void drawImage(const Image& img, int x, int y, Anchor anchor = Anchor::TopLeft) = 0;
        virtual void drawImage(const Image& img, int x, int y, int width, int height) = 0;

        //----------[ CLIP ]----------//
        virtual void setClipRect(int x, int y, int width, int height) = 0;
        virtual void clearClipRect() = 0;
};