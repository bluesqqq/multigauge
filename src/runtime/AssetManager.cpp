#include <multigauge/runtime/AssetManager.h>

#include "../AppPaths.h"

#include <multigauge/io/Base64.h>
#include <multigauge/io/Log.h>
#include <multigauge/utils/Json.h>
#include <multigauge/utils/Text.h>

namespace mg {

bool AssetManager::loadJson(const std::string &path, json::Document &out) {
    constexpr const char* TAG = "AssetManager::loadJson";

    LOG_DEBUG(TAG, "Reading gauge document: %s", path.c_str());

    std::vector<uint8_t> bytes;
    if (!fs->readBytes(path, bytes)) {
        LOG_ERROR(TAG, "readBytes failed: %s", path.c_str());
        return false;
    }

    if (bytes.empty()) {
        LOG_ERROR(TAG, "empty file: %s", path.c_str());
        return false;
    }

    const std::string_view text(reinterpret_cast<const char*>(bytes.data()), bytes.size());
    out = json::parse(text);
    if (!out.valid()) {
        LOG_ERROR(TAG, "parse error: file=%s", path.c_str());
        return false;
    }

    LOG_INFO(TAG, "Loaded gauge document: %s (%u bytes)", path.c_str(), (unsigned)bytes.size());
    return true;
}

bool AssetManager::writeAsset(std::string_view packageId, std::string_view name, std::string_view data) {
    constexpr const char* TAG = "AssetManager::writeAsset";
    if (!utils::isSafeId(packageId, true) || !utils::isSafeFileName(name) || data.empty() || data.size() > 64 * 1024) return false;

    std::vector<uint8_t> decoded;
    if (!io::base64Decode(data, decoded) || decoded.empty()) {
        LOG_ERROR(TAG, "Failed to decode asset: %.*s", static_cast<int>(name.size()), name.data());
        return false;
    }
    const std::string path = paths::assetPath(dataRoot, packageId, paths::imagesDir, name);
    if (!fs->writeBytes(path, decoded.data(), decoded.size())) return false;
    LOG_DEBUG(TAG, "Wrote asset '%s' (%u bytes)", path.c_str(), static_cast<unsigned>(decoded.size()));
    return true;
}

bool AssetManager::removeAsset(std::string_view packageId, std::string_view name) {
    if (!utils::isSafeId(packageId, true) || !utils::isSafeFileName(name)) return false;
    const std::string path = paths::assetPath(dataRoot, packageId, paths::imagesDir, name);
    return !fs->exists(path) || fs->remove(path);
}

bool AssetManager::loadImage(graphics::GraphicsContext& ctx, std::string_view packageId, const std::string& p, images::Image& out) {
    constexpr const char* TAG = "AssetManager::loadImage";

    if (!utils::isSafeId(packageId, true) || !utils::isSafeFileName(p)) {
        LOG_ERROR(TAG, "Invalid image asset name: %s", p.c_str());
        return false;
    }

    const std::string path = paths::assetPath(dataRoot, packageId, paths::imagesDir, p);

    if (!fs->exists(path)) {
        LOG_ERROR(TAG, "Image does not exist: %s", path.c_str());
        return false;
    }

    std::vector<uint8_t> data;
    if (!fs->readBytes(path, data) || data.empty()) {
        LOG_ERROR(TAG, "Failed to read image: %s", path.c_str());
        return false;
    }

    images::ImageInfo info;
    if (!images::decodeImage(data.data(), data.size(), info)) {
        LOG_ERROR(TAG, "Failed to decode image: %s", path.c_str());
        return false;
    }

    out = ctx.createNativeImage(info.pixels.data(), info.width, info.height);

    if (out.empty()) {
        LOG_ERROR(TAG, "Failed to create native image: %s", path.c_str());
        return false;
    }

    LOG_INFO(TAG, "Image ready: %s (%d x %d)", path.c_str(), out.width, out.height);

    return true;
}

}
