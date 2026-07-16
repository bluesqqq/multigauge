#include "GraphicsContextCanvas.h"

#include <algorithm>
#include <cstdint>
#include <cmath>
#include <emscripten.h>

using mg::Anchor;
using mg::images::Image;
using mg::graphics::FontSlant;
using mg::graphics::FontWeight;
using mg::graphics::rgba;

static inline float clamp01(float v) { return std::max(0.0f, std::min(1.0f, v)); }

// Assumes rgba components are 0..255 (typical). If yours are 0..1, adjust here.
static inline void unpackRGBA(rgba c, int& r, int& g, int& b, float& a) {
    r = (int)c.r;
    g = (int)c.g;
    b = (int)c.b;
    a = clamp01((float)c.a / 255.0f);
}

EM_JS(int, mg_canvas_init, (const char* canvasId), {
    const id = UTF8ToString(canvasId);
    const canvas = document.getElementById(id);
    if (!canvas) return 0;

    const ctx = canvas.getContext('2d');
    if (!ctx) return 0;

    Module.__mg_canvases = Module.__mg_canvases || new Map();
    Module.__mg_canvasHandles = Module.__mg_canvasHandles || new Map();
    Module.__mg_nextCanvasHandle = Module.__mg_nextCanvasHandle || 1;

    const syncSize = (entry) => {
        if (!entry?.canvas || !entry?.ctx) return;

        const canvas = entry.canvas;
        const ctx = entry.ctx;
        const dpr = window.devicePixelRatio || 1;
        // Prefer the element's layout size so workspace transforms do not leak
        // into the runtime's logical canvas size.
        const cssWidth = Math.max(1, canvas.clientWidth || canvas.getBoundingClientRect().width || canvas.width || 1);
        const cssHeight = Math.max(1, canvas.clientHeight || canvas.getBoundingClientRect().height || canvas.height || 1);
        const pixelWidth = Math.max(1, Math.round(cssWidth * dpr));
        const pixelHeight = Math.max(1, Math.round(cssHeight * dpr));

        if (canvas.width !== pixelWidth || canvas.height !== pixelHeight) {
            canvas.width = pixelWidth;
            canvas.height = pixelHeight;
        }

        const safeLogicalWidth = Math.max(1, entry.logicalWidth || pixelWidth);
        const safeLogicalHeight = Math.max(1, entry.logicalHeight || pixelHeight);
        const scaleX = pixelWidth / safeLogicalWidth;
        const scaleY = pixelHeight / safeLogicalHeight;

        ctx.setTransform(1, 0, 0, 1, 0, 0);
        ctx.imageSmoothingEnabled = true;
        ctx.setTransform(scaleX, 0, 0, scaleY, 0, 0);

        entry.scaleX = scaleX;
        entry.scaleY = scaleY;
        entry.pixelWidth = pixelWidth;
        entry.pixelHeight = pixelHeight;
    };

    const attachResizeTracking = (entry, handle) => {
        if (!entry || entry.resizeTrackingAttached || !entry.canvas) return;

        const onResize = () => syncSize(entry);

        if (typeof ResizeObserver !== 'undefined') {
            entry.resizeObserver = new ResizeObserver(onResize);
            entry.resizeObserver.observe(entry.canvas);
        }

        window.addEventListener('resize', onResize, { passive: true });
        entry.resizeTrackingAttached = true;
        entry.resizeTrackingHandle = handle;
    };

    const existingHandle = Module.__mg_canvasHandles.get(id);
    if (existingHandle) {
        ctx.imageSmoothingEnabled = true;
        const existingEntry = Module.__mg_canvases.get(existingHandle) || { id };
        existingEntry.canvas = canvas;
        existingEntry.ctx = ctx;
        existingEntry.id = id;
        existingEntry.renderMode = existingEntry.renderMode ?? 3;
        Module.__mg_canvases.set(existingHandle, existingEntry);
        syncSize(existingEntry);
        attachResizeTracking(existingEntry, existingHandle);
        return existingHandle;
    }

    const handle = Module.__mg_nextCanvasHandle++;
    Module.__mg_canvasHandles.set(id, handle);
    const entry = { canvas, ctx, id, renderMode: 3 };
    Module.__mg_canvases.set(handle, entry);

    ctx.imageSmoothingEnabled = true;
    syncSize(entry);
    attachResizeTracking(entry, handle);
    return handle;
});

EM_JS(void, mg_canvas_set_active, (int handle), {
    const entry = Module.__mg_canvases?.get(handle);
    Module.__mg_activeCanvasHandle = handle;
    Module.__mg_canvas = entry?.canvas || null;
    Module.__mg_ctx = entry?.ctx || null;
});

EM_JS(void, mg_canvas_set_render_mode, (int handle, int mode), {
    const entry = Module.__mg_canvases?.get(handle);
    if (!entry) return;
    entry.renderMode = mode | 0;
});

EM_JS(void, mg_canvas_begin_frame, (int handle, int logicalWidth, int logicalHeight), {
    const entry = Module.__mg_canvases?.get(handle);
    if (!entry) return;

    const canvas = entry.canvas;
    const ctx = entry.ctx;
    if (!canvas || !ctx) return;

    const safeLogicalWidth = Math.max(1, logicalWidth | 0);
    const safeLogicalHeight = Math.max(1, logicalHeight | 0);
    const renderMode = entry.renderMode ?? 3;
    if (typeof entry.resizeObserver === 'undefined' && typeof ResizeObserver !== 'undefined') {
        entry.resizeObserver = null;
    }
    const dpr = window.devicePixelRatio || 1;
    // Keep the render logic anchored to layout size, not any ancestor transforms.
    const cssWidth = Math.max(1, canvas.clientWidth || canvas.getBoundingClientRect().width || canvas.width || safeLogicalWidth);
    const cssHeight = Math.max(1, canvas.clientHeight || canvas.getBoundingClientRect().height || canvas.height || safeLogicalHeight);
    const pixelWidth = Math.max(1, Math.round(cssWidth * dpr));
    const pixelHeight = Math.max(1, Math.round(cssHeight * dpr));

    if (canvas.width !== pixelWidth || canvas.height !== pixelHeight) {
        canvas.width = pixelWidth;
        canvas.height = pixelHeight;
    }

    let contentWidth = safeLogicalWidth;
    let contentHeight = safeLogicalHeight;
    let fitScale = 1;
    let offsetX = 0;
    let offsetY = 0;

    if (renderMode === 3) {
        contentWidth = Math.max(1, Math.round(cssWidth));
        contentHeight = Math.max(1, Math.round(cssHeight));
    } else if (renderMode === 0) {
        fitScale = Math.min(cssWidth / safeLogicalWidth, cssHeight / safeLogicalHeight);
    } else if (renderMode === 1) {
        fitScale = Math.min(cssWidth / safeLogicalWidth, cssHeight / safeLogicalHeight);
    } else if (renderMode === 2) {
        fitScale = Math.max(cssWidth / safeLogicalWidth, cssHeight / safeLogicalHeight);
    }

    const scale = (window.devicePixelRatio || 1) * fitScale;
    const scaledWidth = contentWidth * fitScale;
    const scaledHeight = contentHeight * fitScale;
    offsetX = ((cssWidth - scaledWidth) * (window.devicePixelRatio || 1)) * 0.5;
    offsetY = ((cssHeight - scaledHeight) * (window.devicePixelRatio || 1)) * 0.5;

    entry.logicalWidth = contentWidth;
    entry.logicalHeight = contentHeight;
    entry.scaleX = scale;
    entry.scaleY = scale;
    entry.offsetX = offsetX;
    entry.offsetY = offsetY;
    entry.pixelWidth = pixelWidth;
    entry.pixelHeight = pixelHeight;

    ctx.imageSmoothingEnabled = true;
    ctx.setTransform(scale, 0, 0, scale, offsetX, offsetY);
    Module.__mg_canvas = canvas;
    Module.__mg_ctx = ctx;
    Module.__mg_activeCanvasHandle = handle;
});

EM_JS(int, mg_canvas_width, (), {
    const entry = Module.__mg_canvases?.get(Module.__mg_activeCanvasHandle);
    return entry ? (entry.logicalWidth | 0) : 0;
});

EM_JS(int, mg_canvas_height, (), {
    const entry = Module.__mg_canvases?.get(Module.__mg_activeCanvasHandle);
    return entry ? (entry.logicalHeight | 0) : 0;
});

EM_JS(void, mg_set_fill_rgba, (int r, int g, int b, double a), {
    const ctx = Module.__mg_ctx;
    ctx.fillStyle = `rgba(${r|0},${g|0},${b|0},${a})`;
});

EM_JS(void, mg_set_stroke_rgba, (int r, int g, int b, double a, double w), {
    const ctx = Module.__mg_ctx;
    ctx.strokeStyle = `rgba(${r|0},${g|0},${b|0},${a})`;
    ctx.lineWidth = w;
    ctx.lineCap = 'round';
    ctx.lineJoin = 'round';
});

EM_JS(void, mg_clear_canvas, (int r, int g, int b, double a), {
    const ctx = Module.__mg_ctx;
    const entry = Module.__mg_canvases?.get(Module.__mg_activeCanvasHandle);
    if (!ctx || !entry?.canvas) return;

    const pixelWidth = Math.max(1, entry.pixelWidth || entry.canvas.width || 1);
    const pixelHeight = Math.max(1, entry.pixelHeight || entry.canvas.height || 1);

    ctx.setTransform(1, 0, 0, 1, 0, 0);
    ctx.clearRect(0, 0, pixelWidth, pixelHeight);
    ctx.fillStyle = `rgba(${r|0},${g|0},${b|0},${a})`;
    ctx.fillRect(0, 0, pixelWidth, pixelHeight);
    ctx.setTransform(
        entry.scaleX || 1,
        0,
        0,
        entry.scaleY || 1,
        entry.offsetX || 0,
        entry.offsetY || 0
    );
});

EM_JS(void, mg_clear_rect, (int x, int y, int w, int h), {
    const ctx = Module.__mg_ctx;
    ctx.clearRect(x, y, w, h);
});

EM_JS(void, mg_fill_rect, (int x, int y, int w, int h), {
    const ctx = Module.__mg_ctx;
    ctx.fillRect(x, y, w, h);
});

EM_JS(void, mg_stroke_rect, (int x, int y, int w, int h), {
    const ctx = Module.__mg_ctx;
    ctx.strokeRect(x, y, w, h);
});

EM_JS(void, mg_fill_round_rect, (int x, int y, int w, int h, double r1, double r2, double r3, double r4), {
    const ctx = Module.__mg_ctx;
    if (!ctx || w <= 0 || h <= 0) return;

    let tl = Math.max(0, +r1);
    let tr = Math.max(0, +r2);
    let br = Math.max(0, +r3);
    let bl = Math.max(0, +r4);

    const maxRadius = Math.min(w, h) * 0.5;
    tl = Math.min(tl, maxRadius);
    tr = Math.min(tr, maxRadius);
    br = Math.min(br, maxRadius);
    bl = Math.min(bl, maxRadius);

    const topScale = (tl + tr) > w ? w / (tl + tr) : 1;
    const rightScale = (tr + br) > h ? h / (tr + br) : 1;
    const bottomScale = (bl + br) > w ? w / (bl + br) : 1;
    const leftScale = (tl + bl) > h ? h / (tl + bl) : 1;
    const scale = Math.min(topScale, rightScale, bottomScale, leftScale, 1);

    tl *= scale;
    tr *= scale;
    br *= scale;
    bl *= scale;

    ctx.beginPath();
    ctx.moveTo(x + tl, y);
    ctx.lineTo(x + w - tr, y);
    if (tr > 0) ctx.arcTo(x + w, y, x + w, y + tr, tr);
    else ctx.lineTo(x + w, y);
    ctx.lineTo(x + w, y + h - br);
    if (br > 0) ctx.arcTo(x + w, y + h, x + w - br, y + h, br);
    else ctx.lineTo(x + w, y + h);
    ctx.lineTo(x + bl, y + h);
    if (bl > 0) ctx.arcTo(x, y + h, x, y + h - bl, bl);
    else ctx.lineTo(x, y + h);
    ctx.lineTo(x, y + tl);
    if (tl > 0) ctx.arcTo(x, y, x + tl, y, tl);
    else ctx.lineTo(x, y);
    ctx.closePath();
    ctx.fill();
});

EM_JS(void, mg_stroke_round_rect, (int x, int y, int w, int h, double r1, double r2, double r3, double r4), {
    const ctx = Module.__mg_ctx;
    if (!ctx || w <= 0 || h <= 0) return;

    let tl = Math.max(0, +r1);
    let tr = Math.max(0, +r2);
    let br = Math.max(0, +r3);
    let bl = Math.max(0, +r4);

    const maxRadius = Math.min(w, h) * 0.5;
    tl = Math.min(tl, maxRadius);
    tr = Math.min(tr, maxRadius);
    br = Math.min(br, maxRadius);
    bl = Math.min(bl, maxRadius);

    const topScale = (tl + tr) > w ? w / (tl + tr) : 1;
    const rightScale = (tr + br) > h ? h / (tr + br) : 1;
    const bottomScale = (bl + br) > w ? w / (bl + br) : 1;
    const leftScale = (tl + bl) > h ? h / (tl + bl) : 1;
    const scale = Math.min(topScale, rightScale, bottomScale, leftScale, 1);

    tl *= scale;
    tr *= scale;
    br *= scale;
    bl *= scale;

    ctx.beginPath();
    ctx.moveTo(x + tl, y);
    ctx.lineTo(x + w - tr, y);
    if (tr > 0) ctx.arcTo(x + w, y, x + w, y + tr, tr);
    else ctx.lineTo(x + w, y);
    ctx.lineTo(x + w, y + h - br);
    if (br > 0) ctx.arcTo(x + w, y + h, x + w - br, y + h, br);
    else ctx.lineTo(x + w, y + h);
    ctx.lineTo(x + bl, y + h);
    if (bl > 0) ctx.arcTo(x, y + h, x, y + h - bl, bl);
    else ctx.lineTo(x, y + h);
    ctx.lineTo(x, y + tl);
    if (tl > 0) ctx.arcTo(x, y, x + tl, y, tl);
    else ctx.lineTo(x, y);
    ctx.closePath();
    ctx.stroke();
});

EM_JS(void, mg_pixel, (int x, int y), {
    const ctx = Module.__mg_ctx;
    ctx.fillRect(x, y, 1, 1);
});

EM_JS(void, mg_line, (int x0, int y0, int x1, int y1), {
    const ctx = Module.__mg_ctx;
    ctx.beginPath();
    ctx.moveTo(x0 + 0.5, y0 + 0.5);
    ctx.lineTo(x1 + 0.5, y1 + 0.5);
    ctx.stroke();
});

EM_JS(void, mg_circle_fill, (int cx, int cy, int r), {
    const ctx = Module.__mg_ctx;
    ctx.beginPath();
    ctx.arc(cx, cy, r, 0, Math.PI * 2);
    ctx.fill();
});

EM_JS(void, mg_circle_stroke, (int cx, int cy, int r), {
    const ctx = Module.__mg_ctx;
    ctx.beginPath();
    ctx.arc(cx, cy, r, 0, Math.PI * 2);
    ctx.stroke();
});

EM_JS(void, mg_ellipse_fill, (int cx, int cy, int rx, int ry), {
    const ctx = Module.__mg_ctx;
    ctx.beginPath();
    ctx.ellipse(cx, cy, rx, ry, 0, 0, Math.PI * 2);
    ctx.fill();
});

EM_JS(void, mg_ellipse_stroke, (int cx, int cy, int rx, int ry), {
    const ctx = Module.__mg_ctx;
    ctx.beginPath();
    ctx.ellipse(cx, cy, rx, ry, 0, 0, Math.PI * 2);
    ctx.stroke();
});

EM_JS(void, mg_arc_fill, (int cx, int cy, int r, double start, double end), {
    const ctx = Module.__mg_ctx;
    ctx.beginPath();
    ctx.moveTo(cx, cy);
    ctx.arc(cx, cy, r, start, end);
    ctx.closePath();
    ctx.fill();
});

EM_JS(void, mg_arc_stroke, (int cx, int cy, int r, double start, double end), {
    const ctx = Module.__mg_ctx;
    ctx.beginPath();
    ctx.arc(cx, cy, r, start, end);
    ctx.stroke();
});

EM_JS(void, mg_tri_fill, (int x0,int y0,int x1,int y1,int x2,int y2), {
    const ctx = Module.__mg_ctx;
    ctx.beginPath();
    ctx.moveTo(x0, y0);
    ctx.lineTo(x1, y1);
    ctx.lineTo(x2, y2);
    ctx.closePath();
    ctx.fill();
});

EM_JS(void, mg_tri_stroke, (int x0,int y0,int x1,int y1,int x2,int y2), {
    const ctx = Module.__mg_ctx;
    ctx.beginPath();
    ctx.moveTo(x0, y0);
    ctx.lineTo(x1, y1);
    ctx.lineTo(x2, y2);
    ctx.closePath();
    ctx.stroke();
});

EM_JS(double, mg_measure_text, (const char* text, const char* font), {
    const ctx = Module.__mg_ctx;
    const t = UTF8ToString(text);
    const f = UTF8ToString(font);
    ctx.font = f;
    const m = ctx.measureText(t);
    return m.width || 0;
});

EM_JS(void, mg_draw_text, (const char* text, int x, int y, const char* font), {
    const ctx = Module.__mg_ctx;
    const t = UTF8ToString(text);
    const f = UTF8ToString(font);
    ctx.font = f;
    ctx.textBaseline = 'top';
    ctx.fillText(t, x, y);
});

EM_JS(void, mg_clip_rect, (int x, int y, int w, int h), {
    const ctx = Module.__mg_ctx;
    ctx.save();
    ctx.beginPath();
    ctx.rect(x, y, w, h);
    ctx.clip();
});

EM_JS(void, mg_clear_clip, (), {
    const ctx = Module.__mg_ctx;
    ctx.restore();
});

EM_JS(int, mg_image_create_rgba, (const unsigned char* pixels, int width, int height), {
    if (!pixels || width <= 0 || height <= 0) return 0;

    const byteLength = width * height * 4;
    const source = HEAPU8.subarray(pixels, pixels + byteLength);
    const copy = new Uint8ClampedArray(byteLength);
    copy.set(source);

    const canvas = document.createElement('canvas');
    canvas.width = width;
    canvas.height = height;
    const ctx = canvas.getContext('2d');
    if (!ctx) return 0;

    ctx.putImageData(new ImageData(copy, width, height), 0, 0);

    Module.__mg_images = Module.__mg_images || new Map();
    Module.__mg_nextImageHandle = Module.__mg_nextImageHandle || 1;

    const handle = Module.__mg_nextImageHandle++;
    Module.__mg_images.set(handle, { canvas, width, height });
    return handle;
});

EM_JS(void, mg_image_destroy, (int handle), {
    Module.__mg_images?.delete(handle);
});

EM_JS(void, mg_draw_image, (int handle, int x, int y), {
    const ctx = Module.__mg_ctx;
    const entry = Module.__mg_images?.get(handle);
    if (!ctx || !entry?.canvas) return;
    ctx.drawImage(entry.canvas, x, y);
});

EM_JS(void, mg_draw_image_rotated, (int handle, int x, int y, float angleDeg, int pivotX, int pivotY), {
    const ctx = Module.__mg_ctx;
    const entry = Module.__mg_images?.get(handle);
    if (!ctx || !entry?.canvas) return;

    ctx.save();
    ctx.translate(x + pivotX, y + pivotY);
    ctx.rotate((angleDeg * Math.PI) / 180);
    ctx.drawImage(entry.canvas, -pivotX, -pivotY);
    ctx.restore();
});

EM_JS(void, mg_draw_image_stretched, (int handle, int x, int y, int width, int height), {
    const ctx = Module.__mg_ctx;
    const entry = Module.__mg_images?.get(handle);
    if (!ctx || !entry?.canvas) return;
    ctx.drawImage(entry.canvas, x, y, width, height);
});

EM_JS(void, mg_draw_image_region, (int handle, int srcX, int srcY, int srcW, int srcH, int destX, int destY, int destW, int destH), {
    const ctx = Module.__mg_ctx;
    const entry = Module.__mg_images?.get(handle);
    if (!ctx || !entry?.canvas) return;
    ctx.drawImage(entry.canvas, srcX, srcY, srcW, srcH, destX, destY, destW, destH);
});

EM_JS(void, mg_draw_image_transformed, (int handle, int x, int y, float angleDeg, float scaleX, float scaleY, int pivotX, int pivotY), {
    const ctx = Module.__mg_ctx;
    const entry = Module.__mg_images?.get(handle);
    if (!ctx || !entry?.canvas) return;

    ctx.save();
    ctx.translate(x + pivotX, y + pivotY);
    ctx.rotate((angleDeg * Math.PI) / 180);
    ctx.scale(scaleX, scaleY);
    ctx.drawImage(entry.canvas, -pivotX, -pivotY);
    ctx.restore();
});



static std::string makeFontCSS(std::string family, float pt, FontWeight weight, FontSlant slant) {
    const char* w = "400";
    switch (weight) {
        case FontWeight::Normal: w = "400"; break;
        case FontWeight::Bold: w = "700"; break;
        default: break;
    }

    const char* s = (slant == FontSlant::Italic) ? "italic" : "normal";
    char buf[256];
    std::snprintf(buf, sizeof(buf), "%s %s %.2fpx %s", s, w, pt, family.c_str());
    return std::string(buf);
}

namespace {
    int nativeImageHandle(const Image& image) {
        return static_cast<int>(reinterpret_cast<std::intptr_t>(image.native));
    }

    void destroyNativeImageHandle(void* native) {
        const int handle = static_cast<int>(reinterpret_cast<std::intptr_t>(native));
        if (handle > 0) {
            mg_image_destroy(handle);
        }
    }
}

GraphicsContextCanvas::GraphicsContextCanvas(std::string canvasId) : canvasId(std::move(canvasId)) {}

bool GraphicsContextCanvas::init() {
    canvasHandle = mg_canvas_init(canvasId.c_str());
    if (!canvasHandle) return false;
    mg_canvas_set_active(canvasHandle);
    mg_canvas_begin_frame(canvasHandle, logicalWidth > 0 ? logicalWidth : 1, logicalHeight > 0 ? logicalHeight : 1);
    const int initialWidth = mg_canvas_width();
    const int initialHeight = mg_canvas_height();
    logicalWidth = std::max(1, initialWidth);
    logicalHeight = std::max(1, initialHeight);
    w = logicalWidth;
    h = logicalHeight;
    return true;
}

void GraphicsContextCanvas::beginFrame() {
    mg_canvas_begin_frame(canvasHandle, logicalWidth, logicalHeight);
    logicalWidth = std::max(1, mg_canvas_width());
    logicalHeight = std::max(1, mg_canvas_height());
    w = logicalWidth;
    h = logicalHeight;
}

void GraphicsContextCanvas::setRenderMode(RenderMode mode) {
    renderMode = mode;
    if (canvasHandle) {
        mg_canvas_set_render_mode(canvasHandle, static_cast<int>(renderMode));
        mg_canvas_set_active(canvasHandle);
        mg_canvas_begin_frame(canvasHandle, logicalWidth, logicalHeight);
        logicalWidth = std::max(1, mg_canvas_width());
        logicalHeight = std::max(1, mg_canvas_height());
        w = logicalWidth;
        h = logicalHeight;
    }
}

bool GraphicsContextCanvas::resize(int width, int height) {
    logicalWidth = std::max(1, width);
    logicalHeight = std::max(1, height);
    w = logicalWidth;
    h = logicalHeight;
    if (canvasHandle) {
        mg_canvas_begin_frame(canvasHandle, logicalWidth, logicalHeight);
        logicalWidth = std::max(1, mg_canvas_width());
        logicalHeight = std::max(1, mg_canvas_height());
        w = logicalWidth;
        h = logicalHeight;
    }
    return true;
}

void GraphicsContextCanvas::clear(rgba color) {
    int r,g,b; float a;
    unpackRGBA(color, r,g,b,a);
    mg_clear_canvas(r,g,b,a);
}

void GraphicsContextCanvas::pixel(int x, int y, rgba color) {
    int r,g,b; float a;
    unpackRGBA(color, r,g,b,a);
    mg_set_fill_rgba(r,g,b,a);
    mg_pixel(x,y);
}

void GraphicsContextCanvas::line(int x0, int y0, int x1, int y1, rgba color, float thickness) {
    int r,g,b; float a;
    unpackRGBA(color, r,g,b,a);
    mg_set_stroke_rgba(r,g,b,a, thickness);
    mg_line(x0,y0,x1,y1);
}

void GraphicsContextCanvas::rect(int x, int y, int ww, int hh, rgba color) {
    int r,g,b; float a;
    unpackRGBA(color, r,g,b,a);
    mg_set_fill_rgba(r,g,b,a);
    mg_fill_rect(x,y,ww,hh);
}

void GraphicsContextCanvas::strokeRect(int x, int y, int ww, int hh, rgba color, float thickness) {
    int r,g,b; float a;
    unpackRGBA(color, r,g,b,a);
    mg_set_stroke_rgba(r,g,b,a, thickness);
    mg_stroke_rect(x,y,ww,hh);
}

void GraphicsContextCanvas::roundRect(int x,int y,int ww,int hh,float radius, rgba color) {
    roundRect(x, y, ww, hh, radius, radius, radius, radius, color);
}

void GraphicsContextCanvas::roundRect(int x,int y,int ww,int hh,float r1,float r2,float r3,float r4, rgba color) {
    int r,g,b; float a;
    unpackRGBA(color, r,g,b,a);
    mg_set_fill_rgba(r,g,b,a);
    mg_fill_round_rect(x, y, ww, hh, r1, r2, r3, r4);
}

void GraphicsContextCanvas::strokeRoundRect(int x,int y,int ww,int hh,float radius, rgba color, float thickness) {
    strokeRoundRect(x, y, ww, hh, radius, radius, radius, radius, color, thickness);
}

void GraphicsContextCanvas::strokeRoundRect(int x,int y,int ww,int hh,float r1,float r2,float r3,float r4, rgba color, float thickness) {
    int r,g,b; float a;
    unpackRGBA(color, r,g,b,a);
    mg_set_stroke_rgba(r,g,b,a, thickness);
    mg_stroke_round_rect(x, y, ww, hh, r1, r2, r3, r4);
}

void GraphicsContextCanvas::circle(int cx, int cy, int r, rgba color) {
    int rr,gg,bb; float a;
    unpackRGBA(color, rr,gg,bb,a);
    mg_set_fill_rgba(rr,gg,bb,a);
    mg_circle_fill(cx,cy,r);
}
void GraphicsContextCanvas::strokeCircle(int cx, int cy, int r, rgba color, float thickness) {
    int rr,gg,bb; float a;
    unpackRGBA(color, rr,gg,bb,a);
    mg_set_stroke_rgba(rr,gg,bb,a, thickness);
    mg_circle_stroke(cx,cy,r);
}

void GraphicsContextCanvas::ellipse(int cx, int cy, int rx, int ry, rgba color) {
    int rr,gg,bb; float a;
    unpackRGBA(color, rr,gg,bb,a);
    mg_set_fill_rgba(rr,gg,bb,a);
    mg_ellipse_fill(cx,cy,rx,ry);
}
void GraphicsContextCanvas::strokeEllipse(int cx, int cy, int rx, int ry, rgba color, float thickness) {
    int rr,gg,bb; float a;
    unpackRGBA(color, rr,gg,bb,a);
    mg_set_stroke_rgba(rr,gg,bb,a, thickness);
    mg_ellipse_stroke(cx,cy,rx,ry);
}

void GraphicsContextCanvas::ring(int cx, int cy, int r1, int r2, rgba color) {
    int rr,gg,bb; float a;
    unpackRGBA(color, rr,gg,bb,a);
    mg_set_stroke_rgba(rr,gg,bb,a, (float)std::abs(r2 - r1));
    mg_circle_stroke(cx,cy, (r1 + r2) / 2);
}

void GraphicsContextCanvas::strokeRing(int cx, int cy, int r1, int r2, rgba color, float thickness) {
    strokeCircle(cx,cy,r1,color,thickness);
    strokeCircle(cx,cy,r2,color,thickness);
}

void GraphicsContextCanvas::arc(int cx, int cy, int r1, int r2, float start, float end, rgba color) {
    int rr,gg,bb; float a;
    unpackRGBA(color, rr,gg,bb,a);
    mg_set_fill_rgba(rr,gg,bb,a);
    mg_arc_fill(cx,cy,r2, start, end);
}

void GraphicsContextCanvas::strokeArc(int cx, int cy, int r1, int r2, float start, float end, rgba color, float thickness) {
    int rr,gg,bb; float a;
    unpackRGBA(color, rr,gg,bb,a);
    mg_set_stroke_rgba(rr,gg,bb,a, thickness);
    mg_arc_stroke(cx,cy,r2, start, end);
}

void GraphicsContextCanvas::tri(int x0,int y0,int x1,int y1,int x2,int y2, rgba color) {
    int rr,gg,bb; float a;
    unpackRGBA(color, rr,gg,bb,a);
    mg_set_fill_rgba(rr,gg,bb,a);
    mg_tri_fill(x0,y0,x1,y1,x2,y2);
}

void GraphicsContextCanvas::strokeTri(int x0,int y0,int x1,int y1,int x2,int y2, rgba color, float thickness) {
    int rr,gg,bb; float a;
    unpackRGBA(color, rr,gg,bb,a);
    mg_set_stroke_rgba(rr,gg,bb,a, thickness);
    mg_tri_stroke(x0,y0,x1,y1,x2,y2);
}

float GraphicsContextCanvas::getTextWidth(const char* text, std::string family, float pt, FontWeight weight, FontSlant slant) {
    std::string font = makeFontCSS(std::move(family), pt, weight, slant);
    return (float)mg_measure_text(text, font.c_str());
}

void GraphicsContextCanvas::drawText(const char* text, int x, int y, std::string family, float pt, FontWeight weight, FontSlant slant, rgba color, Anchor anchor) {
    int rr,gg,bb; float a;
    unpackRGBA(color, rr,gg,bb,a);
    mg_set_fill_rgba(rr,gg,bb,a);

    std::string font = makeFontCSS(std::move(family), pt, weight, slant);

    float wText = (float)mg_measure_text(text, font.c_str());

    int drawX = x;
    int drawY = y;

    if (anchor == Anchor::TopCenter || anchor == Anchor::Center || anchor == Anchor::BottomCenter) {
        drawX = (int)std::lround((double)x - wText * 0.5);
    } else if (anchor == Anchor::TopRight || anchor == Anchor::CenterRight || anchor == Anchor::BottomRight) {
        drawX = (int)std::lround((double)x - wText);
    }

    if (anchor == Anchor::CenterLeft || anchor == Anchor::Center || anchor == Anchor::CenterRight) {
        drawY = (int)std::lround((double)y - pt * 0.5);
    } else if (anchor == Anchor::BottomLeft || anchor == Anchor::BottomCenter || anchor == Anchor::BottomRight) {
        drawY = (int)std::lround((double)y - pt);
    }

    mg_draw_text(text, drawX, drawY, font.c_str());
}

// ----- Images -----
Image GraphicsContextCanvas::createNativeImage(const rgba* pixels, int ww, int hh) {
    if (!pixels || ww <= 0 || hh <= 0) {
        return Image{};
    }

    const int handle = mg_image_create_rgba(reinterpret_cast<const std::uint8_t*>(pixels), ww, hh);
    if (handle <= 0) {
        return Image{};
    }

    return Image(ww, hh, reinterpret_cast<void*>(static_cast<std::intptr_t>(handle)), &destroyNativeImageHandle);
}

void GraphicsContextCanvas::drawImage(const Image& img, int x, int y) {
    if (img.empty()) return;
    mg_draw_image(nativeImageHandle(img), x, y);
}

void GraphicsContextCanvas::drawImageRotated(const Image& img, int x, int y, float angle, int pivotX, int pivotY) {
    if (img.empty()) return;
    mg_draw_image_rotated(nativeImageHandle(img), x, y, angle, pivotX, pivotY);
}

void GraphicsContextCanvas::drawImageScaled(const Image& img, int x, int y, float sx, float sy) {
    if (img.empty()) return;
    const int width = std::max(1, static_cast<int>(std::lround(img.width * sx)));
    const int height = std::max(1, static_cast<int>(std::lround(img.height * sy)));
    mg_draw_image_stretched(nativeImageHandle(img), x, y, width, height);
}

void GraphicsContextCanvas::drawImageTransformed(const Image& img, int x, int y, float angle, float sx, float sy, int pivotX, int pivotY) {
    if (img.empty()) return;
    mg_draw_image_transformed(nativeImageHandle(img), x, y, angle, sx, sy, pivotX, pivotY);
}

void GraphicsContextCanvas::drawImageStretched(const Image& img, int x, int y, int width, int height) {
    if (img.empty()) return;
    mg_draw_image_stretched(nativeImageHandle(img), x, y, width, height);
}

void GraphicsContextCanvas::drawImageRegion(const Image& img, int srcX, int srcY, int srcW, int srcH, int destX, int destY, int destW, int destH) {
    if (img.empty()) return;
    mg_draw_image_region(nativeImageHandle(img), srcX, srcY, srcW, srcH, destX, destY, destW, destH);
}

void GraphicsContextCanvas::clip(int x, int y, int width, int height) {
    if (hasClip) clearClip();
    mg_clip_rect(x,y,width,height);
    hasClip = true;
}

void GraphicsContextCanvas::clearClip() {
    if (!hasClip) return;
    mg_clear_clip();
    hasClip = false;
}
