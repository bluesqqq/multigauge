#pragma once

#include <multigauge/images/Image.h>
#include <multigauge/AssetManager.h>
#include "rapidjson/document.h"
#include <multigauge/json/rj_helpers.h>

struct NineSlice {
    const char* path = "";

    Image image;

    int left = 0, right = 0, top = 0, bottom = 0;

    bool stretch = true;

    NineSlice() = default;
    NineSlice(const char* p, int l, int t, int r, int b) : path(p), left(l), top(t), right(r), bottom(b) {}
    NineSlice(const rapidjson::Value::ConstObject& json) {
        setCString(json, "image", path);
        setInt(json, "left", left);
        setInt(json, "top", top);
        setInt(json, "right", right);
        setInt(json, "bottom", bottom);
    }

    bool init(AssetManager& assetManager) { return assetManager.loadImage(path, image); }
};