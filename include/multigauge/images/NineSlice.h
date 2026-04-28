#pragma once

#include <multigauge/images/Image.h>
#include <multigauge/AssetManager.h>
#include "rapidjson/document.h"

namespace mg::images {

struct NineSlice {
    const char* path = "";

    Image image;

    int left = 0, right = 0, top = 0, bottom = 0;

    bool stretch = true;

    NineSlice() = default;
    NineSlice(const char* p, int l, int t, int r, int b) : path(p), left(l), top(t), right(r), bottom(b) {}

    bool init(::mg::AssetManager& assetManager, ::mg::graphics::GraphicsContext& context) { return assetManager.loadImage(context, path, image); }
};

} // namespace mg::images
