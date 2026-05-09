#include <multigauge/AssetManager.h>

#include <multigauge/io/Base64.h>
#include <multigauge/io/Log.h>

namespace mg {

AssetManager::ImageType AssetManager::detectImageType(const uint8_t* data, size_t size) {
    if (size >= 2 && data[0] == 'B' && data[1] == 'M') return ImageType::BMP;

    if (size >= 8 &&
        data[0] == 0x89 && data[1] == 0x50 &&
        data[2] == 0x4E && data[3] == 0x47 &&
        data[4] == 0x0D && data[5] == 0x0A &&
        data[6] == 0x1A && data[7] == 0x0A) {
        return ImageType::PNG;
    }

    if (size >= 3 && data[0] == 0xFF && data[1] == 0xD8 && data[2] == 0xFF) return ImageType::JPG;

    return ImageType::Unknown;
}

const char* AssetManager::imageTypeName(ImageType type) {
    switch (type) {
        case ImageType::BMP: return "BMP";
        case ImageType::PNG: return "PNG";
        case ImageType::JPG: return "JPG";
        default: return "Unknown";
    }
}


bool AssetManager::decodeBase64(const std::string_view& encoded, std::vector<uint8_t>& out) {
    out.resize(io::base64DecodedMaxSize(encoded));

    size_t decodedLen = 0;
    if (!io::base64Decode(encoded, out.data(), out.size(), decodedLen)) {
        return false;
    }

    out.resize(decodedLen);
    return true;
}

bool AssetManager::decodeImageData(ImageType type, const std::vector<uint8_t>& data, images::ImageInfo& info) {
    switch (type) {
        case ImageType::BMP: return images::decodeBMP(data.data(), data.size(), info);
        case ImageType::PNG: return images::decodePNG(data.data(), data.size(), info);
        case ImageType::JPG: return images::decodeJPG(data.data(), data.size(), info);
        default: return false;
    }
}


bool AssetManager::loadJson(const std::string &path, rapidjson::Document &out) {
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

    bytes.push_back(0);

    const char* json = reinterpret_cast<const char*>(bytes.data());
    const size_t length = bytes.size() - 1;

    out.Parse(json, length);

    if (out.HasParseError()) {
        LOG_ERROR(TAG, "parse error: %s (offset=%u) file=%s",
            rapidjson::GetParseErrorFunc(out.GetParseError()),
            (unsigned)out.GetErrorOffset(),
            path.c_str());
        return false;
    }

    LOG_INFO(TAG, "Loaded gauge document: %s (%u bytes)", path.c_str(), (unsigned)length);
    return true;
}

bool AssetManager::loadDocumentAssets(const rapidjson::Value::ConstArray &assets) {
    constexpr const char* TAG = "AssetManager::loadDocumentAssets";

    LOG_INFO(TAG, "Loading %u embedded assets", (unsigned)assets.Size());

    size_t totalDecodedBytes = 0;

    for (const auto& asset : assets) {
        if (!asset.IsObject()) {
            LOG_ERROR(TAG, "Asset must be an object.");
            return false;
        }

        const auto obj = asset.GetObject();

        auto nameIt = obj.FindMember("name");
        auto dataIt = obj.FindMember("data");

        if (nameIt == obj.MemberEnd() || !nameIt->value.IsString()) {
            LOG_ERROR(TAG, "Missing 'name'");
            return false;
        }

        if (dataIt == obj.MemberEnd() || !dataIt->value.IsString()) {
            LOG_ERROR(TAG, "Missing 'data' for '%s'", nameIt->value.GetString());
            return false;
        }

        const std::string name(nameIt->value.GetString(), nameIt->value.GetStringLength());
        const std::string_view encoded(dataIt->value.GetString(), dataIt->value.GetStringLength());

        std::vector<uint8_t> decoded;
        if (!decodeBase64(encoded, decoded)) {
            LOG_ERROR(TAG, "Failed to decode asset: %s", name.c_str());
            return false;
        }

        totalDecodedBytes += decoded.size();

        const std::string path = "/assets/images/" + name;

        if (!fs->writeBytes(path, decoded.data(), decoded.size())) {
            LOG_ERROR(TAG, "Failed to write asset: %s", path.c_str());
            return false;
        }

        LOG_DEBUG(TAG, "Wrote asset '%s' (%u bytes)", path.c_str(), (unsigned)decoded.size());
    }

    LOG_INFO(TAG, "Processed %u assets (%u bytes)", (unsigned)assets.Size(), (unsigned)totalDecodedBytes);

    return true;
}


bool AssetManager::loadImage(graphics::GraphicsContext& ctx, const std::string &p, images::Image &out) {
    constexpr const char* TAG = "AssetManager::loadImage";

    const std::string path = "/assets/images/" + p;

    if (!fs->exists(path)) {
        LOG_ERROR(TAG, "Image does not exist: %s", path.c_str());
        return false;
    }

    std::vector<uint8_t> data;
    if (!fs->readBytes(path, data) || data.empty()) {
        LOG_ERROR(TAG, "Failed to read image: %s", path.c_str());
        return false;
    }

    ImageType type = detectImageType(data.data(), data.size());

    images::ImageInfo info;
    if (!decodeImageData(type, data, info)) {
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
