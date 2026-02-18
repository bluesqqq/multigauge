#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <algorithm>

#include "io/Logger.h"
#include "utils.h"

#include "lodepng.h"
#include "tjpgd.h"

// BMP compression types (from BMP spec)
constexpr uint32_t BI_RGB            = 0; // no compression
constexpr uint32_t BI_RLE8           = 1;
constexpr uint32_t BI_RLE4           = 2;
constexpr uint32_t BI_BITFIELDS      = 3;
constexpr uint32_t BI_JPEG           = 4;
constexpr uint32_t BI_PNG            = 5;
constexpr uint32_t BI_ALPHABITFIELDS = 6; // adds alpha mask

struct ImageInfo {
    int width = 0;
    int height = 0;
    std::vector<rgba> pixels;
};

//----------------------[ BMP ]----------------------//

static inline bool canDecodeBMP(const uint8_t* data, size_t size) {
    if (!data || size < 2) return false;
    return data[0] == 'B' && data[1] == 'M';
}

static inline bool decodeBMP(const uint8_t* data, size_t size, ImageInfo& out) {
    out = ImageInfo{};

    if (!canDecodeBMP(data, size)) {
        LOG_ERROR("BMP", "Data is not a BMP.");
        return false;
    }

    // BITMAPFILEHEADER
    uint32_t pixelOffset = 0;
    if (!read_u32(data, size, 10, pixelOffset)) {
        LOG_ERROR("BMP", "Truncated BMP header.");
        return false;
    }
    if (pixelOffset >= size) {
        LOG_ERROR("BMP", "Pixel offset out of range.");
        return false;
    }

    // DIB header size
    uint32_t dibSize = 0;
    if (!read_u32(data, size, 14, dibSize)) {
        LOG_ERROR("BMP", "Missing DIB header.");
        return false;
    }
    if (14 + (size_t)dibSize > size) {
        LOG_ERROR("BMP", "Truncated DIB header.");
        return false;
    }
    if (dibSize < 40) {
        LOG_ERROR("BMP", "Unsupported DIB header size: %u", (unsigned)dibSize);
        return false;
    }

    int32_t width = 0, height = 0;
    uint16_t planes = 0, bpp = 0;
    uint32_t compression = 0, colorsUsed = 0;

    if (!read_i32(data, size, 18, width) ||
        !read_i32(data, size, 22, height) ||
        !read_u16(data, size, 26, planes) ||
        !read_u16(data, size, 28, bpp) ||
        !read_u32(data, size, 30, compression) ||
        !read_u32(data, size, 46, colorsUsed)) {
        LOG_ERROR("BMP", "Truncated BITMAPINFOHEADER.");
        return false;
    }

    if (planes != 1) {
        LOG_ERROR("BMP", "Invalid planes (expected 1, got %u)", (unsigned)planes);
        return false;
    }
    if (width <= 0 || height == 0) {
        LOG_ERROR("BMP", "Invalid dimensions (W=%d, H=%d)", (int)width, (int)height);
        return false;
    }

    const bool topDown = (height < 0);
    const int32_t absH = topDown ? -height : height;

    // Reject RLE
    if (compression == BI_RLE8 || compression == BI_RLE4) {
        LOG_ERROR("BMP", "RLE compressed BMP not supported (compression=%u).", (unsigned)compression);
        return false;
    }

    // Allow only these compression modes
    if (!(compression == BI_RGB || compression == BI_BITFIELDS || compression == BI_ALPHABITFIELDS)) {
        LOG_ERROR("BMP", "Unsupported BMP compression: %u", (unsigned)compression);
        return false;
    }

    // Masks (for 16/32bpp when BITFIELDS)
    uint32_t rMask = 0, gMask = 0, bMask = 0, aMask = 0;
    const size_t masksOffset = 14 + 40;

    if (compression == BI_BITFIELDS || compression == BI_ALPHABITFIELDS) {
        if (!read_u32(data, size, masksOffset + 0, rMask) ||
            !read_u32(data, size, masksOffset + 4, gMask) ||
            !read_u32(data, size, masksOffset + 8, bMask)) {
            LOG_ERROR("BMP", "Missing BITFIELDS masks.");
            return false;
        }
        if (masksOffset + 16 <= size) {
            (void)read_u32(data, size, masksOffset + 12, aMask);
        }
    } else {
        // Default masks for common BI_RGB encodings
        if (bpp == 16) {
            // Many BI_RGB 16bpp files are 555; some are 565 but then they should use BITFIELDS.
            rMask = 0x7C00; gMask = 0x03E0; bMask = 0x001F;
        } else if (bpp == 32) {
            rMask = 0x00FF0000; gMask = 0x0000FF00; bMask = 0x000000FF; aMask = 0xFF000000;
        }
    }

    // Palette handling for 1/4/8 bpp
    std::vector<uint32_t> palette;
    if (bpp <= 8) {
        uint32_t paletteEntries = colorsUsed;
        if (paletteEntries == 0) paletteEntries = 1u << bpp;

        const size_t palOffset = 14 + (size_t)dibSize;
        const size_t palBytes = (size_t)paletteEntries * 4;

        if (palOffset + palBytes > size) {
            LOG_ERROR("BMP", "Truncated palette.");
            return false;
        }

        palette.resize(paletteEntries);
        for (uint32_t i = 0; i < paletteEntries; ++i) {
            const size_t o = palOffset + (size_t)i * 4;
            const uint8_t b = data[o + 0];
            const uint8_t g = data[o + 1];
            const uint8_t r = data[o + 2];
            palette[i] = ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
        }
    }

    const int outW = (int)width;
    const int outH = (int)absH;

    out.width = outW;
    out.height = outH;
    out.pixels.assign((size_t)outW * (size_t)outH, rgba{0,0,0,0});

    auto rowStrideBytes = [&](int w, uint16_t bitsPerPixel) -> size_t {
        const size_t bits = (size_t)w * (size_t)bitsPerPixel;
        const size_t bytes = (bits + 7) / 8;
        return (bytes + 3) & ~size_t(3);
    };

    const size_t inStride = rowStrideBytes(outW, bpp);
    const size_t pixelBase = (size_t)pixelOffset;

    if (pixelBase >= size) {
        LOG_ERROR("BMP", "Pixel data starts out of bounds.");
        return false;
    }
    if (pixelBase + inStride * (size_t)outH > size) {
        LOG_ERROR("BMP", "Pixel data truncated (need %u bytes).", (unsigned)(inStride * (size_t)outH));
        return false;
    }

    auto writeRGBA = [&](int x, int y, rgba px) {
        out.pixels[(size_t)y * (size_t)outW + (size_t)x] = px;
    };

    // Decode rows
    for (int y = 0; y < outH; ++y) {
        const int srcY = topDown ? y : (outH - 1 - y);
        const size_t rowOff = pixelBase + (size_t)srcY * inStride;
        const uint8_t* row = data + rowOff;

        if (bpp == 24) {
            for (int x = 0; x < outW; ++x) {
                const size_t o = (size_t)x * 3;
                const uint8_t b = row[o + 0];
                const uint8_t g = row[o + 1];
                const uint8_t r = row[o + 2];
                writeRGBA(x, y, rgba{r, g, b, 255});
            }
        }
        else if (bpp == 32) {
            for (int x = 0; x < outW; ++x) {
                const size_t o = (size_t)x * 4;
                const uint32_t v =
                    (uint32_t)row[o + 0] |
                    ((uint32_t)row[o + 1] << 8) |
                    ((uint32_t)row[o + 2] << 16) |
                    ((uint32_t)row[o + 3] << 24);

                uint8_t r = 0, g = 0, b = 0, a = 255;

                if (compression == BI_BITFIELDS || compression == BI_ALPHABITFIELDS) {
                    r = extract_and_scale(v, rMask);
                    g = extract_and_scale(v, gMask);
                    b = extract_and_scale(v, bMask);
                    if (aMask) a = extract_and_scale(v, aMask);
                } else {
                    // BI_RGB default: BGRA in file
                    b = (uint8_t)(v & 0xFF);
                    g = (uint8_t)((v >> 8) & 0xFF);
                    r = (uint8_t)((v >> 16) & 0xFF);
                    a = (uint8_t)((v >> 24) & 0xFF);
                }

                writeRGBA(x, y, rgba{r, g, b, a});
            }
        }
        else if (bpp == 16) {
            for (int x = 0; x < outW; ++x) {
                const size_t o = (size_t)x * 2;
                const uint16_t v16 = (uint16_t)row[o] | ((uint16_t)row[o + 1] << 8);

                uint8_t r = 0, g = 0, b = 0;
                if (compression == BI_BITFIELDS || compression == BI_ALPHABITFIELDS || rMask != 0) {
                    r = extract_and_scale((uint32_t)v16, rMask);
                    g = extract_and_scale((uint32_t)v16, gMask);
                    b = extract_and_scale((uint32_t)v16, bMask);
                } else {
                    // Fallback: assume 555
                    r = (uint8_t)(((v16 >> 10) & 0x1F) * 255 / 31);
                    g = (uint8_t)(((v16 >> 5)  & 0x1F) * 255 / 31);
                    b = (uint8_t)(((v16)       & 0x1F) * 255 / 31);
                }
                writeRGBA(x, y, rgba{r, g, b, 255});
            }
        }
        else if (bpp == 8) {
            if (palette.empty()) {
                LOG_ERROR("BMP", "8bpp BMP missing palette.");
                return false;
            }
            for (int x = 0; x < outW; ++x) {
                const uint8_t idx = row[x];
                const uint32_t rgb = palette[(size_t)std::min<uint32_t>(idx, (uint32_t)palette.size() - 1)];
                const uint8_t r = (uint8_t)((rgb >> 16) & 0xFF);
                const uint8_t g = (uint8_t)((rgb >> 8) & 0xFF);
                const uint8_t b = (uint8_t)(rgb & 0xFF);
                writeRGBA(x, y, rgba{r, g, b, 255});
            }
        }
        else if (bpp == 4) {
            if (palette.empty()) {
                LOG_ERROR("BMP", "4bpp BMP missing palette.");
                return false;
            }
            for (int x = 0; x < outW; ++x) {
                const uint8_t byte = row[(size_t)x / 2];
                const uint8_t idx = (x % 2 == 0) ? (byte >> 4) : (byte & 0x0F);
                const uint32_t rgb = palette[(size_t)std::min<uint32_t>(idx, (uint32_t)palette.size() - 1)];
                const uint8_t r = (uint8_t)((rgb >> 16) & 0xFF);
                const uint8_t g = (uint8_t)((rgb >> 8) & 0xFF);
                const uint8_t b = (uint8_t)(rgb & 0xFF);
                writeRGBA(x, y, rgba{r, g, b, 255});
            }
        }
        else if (bpp == 1) {
            if (palette.empty()) {
                LOG_ERROR("BMP", "1bpp BMP missing palette.");
                return false;
            }
            for (int x = 0; x < outW; ++x) {
                const uint8_t byte = row[(size_t)x / 8];
                const uint8_t bit = 7 - (x % 8);
                const uint8_t idx = (byte >> bit) & 1u;
                const uint32_t rgb = palette[(size_t)idx];
                const uint8_t r = (uint8_t)((rgb >> 16) & 0xFF);
                const uint8_t g = (uint8_t)((rgb >> 8) & 0xFF);
                const uint8_t b = (uint8_t)(rgb & 0xFF);
                writeRGBA(x, y, rgba{r, g, b, 255});
            }
        }
        else {
            LOG_ERROR("BMP", "Unsupported bpp: %u", (unsigned)bpp);
            return false;
        }
    }

    return true;
}

//----------------------[ PNG ]----------------------//

static inline bool canDecodePNG(const uint8_t* data, size_t size) {
    if (!data || size < 8) return false;
    // 89 50 4E 47 0D 0A 1A 0A
    return data[0] == 0x89 && data[1] == 0x50 && data[2] == 0x4E && data[3] == 0x47 &&
           data[4] == 0x0D && data[5] == 0x0A && data[6] == 0x1A && data[7] == 0x0A;
}

static inline bool decodePNG(const uint8_t* data, size_t size, ImageInfo& out) {
    out = ImageInfo{};

    if (!canDecodePNG(data, size)) {
        LOG_ERROR("PNG", "Data is not a PNG.");
        return false;
    }

    std::vector<unsigned char> pngBytes; // RGBA8888 bytes
    unsigned w = 0, h = 0;

    const unsigned err = lodepng::decode(pngBytes, w, h, data, size);
    if (err != 0) {
        LOG_ERROR("PNG", "Decode failed: %s", lodepng_error_text(err));
        return false;
    }

    if (w == 0 || h == 0) {
        LOG_ERROR("PNG", "Invalid dimensions (W=%u, H=%u).", w, h);
        return false;
    }

    const size_t pxCount = (size_t)w * (size_t)h;
    const size_t needBytes = pxCount * 4;

    if (pngBytes.size() < needBytes) {
        LOG_ERROR("PNG", "Truncated RGBA buffer (have=%u need=%u).",
                  (unsigned)pngBytes.size(), (unsigned)needBytes);
        return false;
    }

    out.width  = (int)w;
    out.height = (int)h;
    out.pixels.resize(pxCount);

    for (size_t i = 0; i < pxCount; ++i) {
        out.pixels[i] = rgba{
            (uint8_t)pngBytes[i * 4 + 0],
            (uint8_t)pngBytes[i * 4 + 1],
            (uint8_t)pngBytes[i * 4 + 2],
            (uint8_t)pngBytes[i * 4 + 3]
        };
    }

    LOG_INFO("PNG", "Successfully loaded png.");

    return true;
}

//----------------------[ JPG ]----------------------//

static inline bool canDecodeJPG(const uint8_t* data, size_t size) {
    if (!data || size < 3) return false;
    // FF D8 FF
    return data[0] == 0xFF && data[1] == 0xD8 && data[2] == 0xFF;
}

struct TjpgdMemSrc {
    const uint8_t* data = nullptr;
    size_t size = 0;
    size_t pos  = 0;

    ImageInfo* out = nullptr;
};

// Input callback: read "nbyte" bytes into "buf". If buf==nullptr, skip bytes.
static size_t tjpgd_in_cb(JDEC* jd, uint8_t* buf, size_t nbyte) {
    auto* src = static_cast<TjpgdMemSrc*>(jd->device);
    if (!src || !src->data) return 0;

    const size_t remain = (src->pos < src->size) ? (src->size - src->pos) : 0;
    if (nbyte > remain) nbyte = remain;

    if (buf) {
        std::copy(src->data + src->pos, src->data + src->pos + nbyte, buf);
    }
    src->pos += nbyte;
    return nbyte;
}

// Output callback: receive a rectangle and its pixel data, then write to out->pixels.
// Pixel format depends on JD_FORMAT in tjpgd config.
// Most common: JD_FORMAT==0 => RGB888 (3 bytes per pixel).
static int tjpgd_out_cb(JDEC* jd, void* bitmap, JRECT* rect) {
    auto* src = static_cast<TjpgdMemSrc*>(jd->device);
    if (!src || !src->out || !bitmap || !rect) return 0;

    ImageInfo& out = *src->out;
    const int outW = out.width;
    const int outH = out.height;

    int left   = rect->left;
    int right  = rect->right;
    int top    = rect->top;
    int bottom = rect->bottom;

    // Clip to output (safety)
    if (left < 0) left = 0;
    if (top < 0) top = 0;
    if (right >= outW) right = outW - 1;
    if (bottom >= outH) bottom = outH - 1;

    const int rw = (right - left + 1);
    const int rh = (bottom - top + 1);

#if JD_FORMAT == 0
    // RGB888: bitmap is uint8_t*, 3 bytes per pixel, row-major for the rect size
    const uint8_t* srcPx = static_cast<const uint8_t*>(bitmap);

    // Provided bitmap corresponds to the original rect width/height
    const int rectW = (rect->right - rect->left + 1);

    for (int y = 0; y < rh; ++y) {
        const int dstY = top + y;

        const int blockRow = (dstY - rect->top);
        const int blockColStart = (left - rect->left);

        const uint8_t* row = srcPx + (size_t)(blockRow * rectW + blockColStart) * 3;

        rgba* dst = out.pixels.data() + (size_t)dstY * (size_t)outW + (size_t)left;

        for (int x = 0; x < rw; ++x) {
            const uint8_t r = row[x * 3 + 0];
            const uint8_t g = row[x * 3 + 1];
            const uint8_t b = row[x * 3 + 2];
            dst[x] = rgba{r, g, b, 255};
        }
    }

#elif JD_FORMAT == 1
    // RGB565: bitmap is uint16_t*. Expand to RGBA.
    const uint16_t* src565 = static_cast<const uint16_t*>(bitmap);
    const int rectW = (rect->right - rect->left + 1);

    for (int y = 0; y < rh; ++y) {
        const int dstY = top + y;
        const int blockRow = (dstY - rect->top);
        const int blockColStart = (left - rect->left);

        const uint16_t* row = src565 + (size_t)(blockRow * rectW + blockColStart);
        rgba* dst = out.pixels.data() + (size_t)dstY * (size_t)outW + (size_t)left;

        for (int x = 0; x < rw; ++x) {
            dst[x] = rgb565_to_rgba(row[x]);
        }
    }

#else
    (void)rw; (void)rh;
    return 0;
#endif

    return 1; // non-zero tells tjpgd to continue
}

static inline bool decodeJPG(const uint8_t* data, size_t size, ImageInfo& out) {
    out = ImageInfo{};

    if (!canDecodeJPG(data, size)) {
        LOG_ERROR("JPG", "Data is not a JPG.");
        return false;
    }

    // Work buffer: tjpgd needs a scratch buffer.
    static uint8_t work[8192];

    TjpgdMemSrc src;
    src.data = data;
    src.size = size;
    src.pos  = 0;
    src.out  = &out;

    JDEC jd;
    JRESULT res = jd_prepare(&jd, tjpgd_in_cb, work, sizeof(work), &src);
    if (res != JDR_OK) {
        LOG_ERROR("JPG", "jd_prepare failed (%d).", (int)res);
        return false;
    }

    if (jd.width == 0 || jd.height == 0) {
        LOG_ERROR("JPG", "Invalid dimensions (W=%u H=%u).", (unsigned)jd.width, (unsigned)jd.height);
        return false;
    }

    out.width  = (int)jd.width;
    out.height = (int)jd.height;
    out.pixels.assign((size_t)out.width * (size_t)out.height, rgba{0,0,0,0});

    // scale: 0=full, 1=1/2, 2=1/4, 3=1/8
    const uint8_t scale = 0;

    res = jd_decomp(&jd, tjpgd_out_cb, scale);
    if (res != JDR_OK) {
        LOG_ERROR("JPG", "jd_decomp failed (%d).", (int)res);
        return false;
    }

    return true;
}