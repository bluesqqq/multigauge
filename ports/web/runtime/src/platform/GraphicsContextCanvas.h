#pragma once

#include <multigauge/graphics/GraphicsContext.h>
#include <string>

class GraphicsContextCanvas : public mg::graphics::GraphicsContext {
public:
    enum class RenderMode {
        Intrinsic = 0,
        Contain = 1,
        Cover = 2,
        Responsive = 3
    };

    explicit GraphicsContextCanvas(std::string canvasId);

    bool init() override;
    void beginFrame() override;
    void setRenderMode(RenderMode mode);

    void clear(mg::graphics::rgba color) override;

    void pixel(int x, int y, mg::graphics::rgba color) override;
    void line(int x0, int y0, int x1, int y1, mg::graphics::rgba color, float thickness) override;

    void rect(int x, int y, int w, int h, mg::graphics::rgba color) override;
    void strokeRect(int x, int y, int w, int h, mg::graphics::rgba color, float thickness) override;

    void roundRect(int x, int y, int w, int h, float radius, mg::graphics::rgba color) override;
    void roundRect(int x, int y, int w, int h, float r1, float r2, float r3, float r4, mg::graphics::rgba color) override;

    void strokeRoundRect(int x, int y, int w, int h, float radius, mg::graphics::rgba color, float thickness) override;
    void strokeRoundRect(int x, int y, int w, int h, float r1, float r2, float r3, float r4, mg::graphics::rgba color, float thickness) override;

    void circle(int cx, int cy, int r, mg::graphics::rgba color) override;
    void strokeCircle(int cx, int cy, int r, mg::graphics::rgba color, float thickness) override;

    void ellipse(int cx, int cy, int rx, int ry, mg::graphics::rgba color) override;
    void strokeEllipse(int cx, int cy, int rx, int ry, mg::graphics::rgba color, float thickness) override;

    void ring(int cx, int cy, int r1, int r2, mg::graphics::rgba color) override;
    void strokeRing(int cx, int cy, int r1, int r2, mg::graphics::rgba color, float thickness) override;

    void arc(int cx, int cy, int r1, int r2, float start, float end, mg::graphics::rgba color) override;
    void strokeArc(int cx, int cy, int r1, int r2, float start, float end, mg::graphics::rgba color, float thickness) override;

    void tri(int x0, int y0, int x1, int y1, int x2, int y2, mg::graphics::rgba color) override;
    void strokeTri(int x0, int y0, int x1, int y1, int x2, int y2, mg::graphics::rgba color, float thickness) override;

    float getTextWidth(const char* text, std::string family, float pt, mg::graphics::FontWeight weight, mg::graphics::FontSlant slant) override;
    void drawText(const char* text, int x, int y, std::string family, float pt, mg::graphics::FontWeight weight, mg::graphics::FontSlant slant, mg::graphics::rgba color, mg::Anchor anchor) override;

    // Image integration depends on your Image class, see stubs in .cpp
    mg::images::Image createNativeImage(const mg::graphics::rgba* pixels, int w, int h) override;
    void drawImage(const mg::images::Image& img, int x, int y) override;
    void drawImageRotated(const mg::images::Image& img, int x, int y, float angle, int pivotX, int pivotY) override;
    void drawImageScaled(const mg::images::Image& img, int x, int y, float sx, float sy) override;
    void drawImageTransformed(const mg::images::Image& img, int x, int y, float angle, float sx, float sy, int pivotX, int pivotY) override;
    void drawImageStretched(const mg::images::Image& img, int x, int y, int width, int height) override;
    void drawImageRegion(const mg::images::Image& img, int srcX, int srcY, int srcW, int srcH, int destX, int destY, int destW, int destH) override;

    void clip(int x, int y, int width, int height) override;
    void clearClip() override;

private:
    std::string canvasId;
    int canvasHandle = 0;
    bool hasClip = false;
    int logicalWidth = 0;
    int logicalHeight = 0;
    RenderMode renderMode = RenderMode::Responsive;

public:
    bool resize(int w, int h) override;
};
