#pragma once

struct Image {
    int width = 0;
    int height = 0;

    void* native = nullptr;

    void (*destroy)(void*) = nullptr;

    Image() = default;
    Image(int w, int h, void* n, void (*d)(void*)) : width(w), height(h), native(n), destroy(d) {}

    ~Image() { if (native && destroy) destroy(native); }

    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

    Image(Image&& other) noexcept { *this = std::move(other); }

    Image& operator=(Image&& other) noexcept {
        width = other.width;
        height = other.height;
        native = other.native;
        destroy = other.destroy;

        other.native = nullptr;
        other.destroy = nullptr;
        return *this;
    }
};
