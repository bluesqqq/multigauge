#include <multigauge/AssetManager.h>

#include <multigauge/io/Base64.h>
#include <multigauge/images/ImageDecoder.h>
#include <multigauge/io/Log.h>

bool AssetManager::loadJson(const std::string &path, rapidjson::Document &out) {
    constexpr const char* TAG = "AssetManager::loadJson";
    
    std::vector<uint8_t> bytes;
    if (!fs.readAll(path, bytes)) {
        LOG_ERROR(TAG, "loadJson: readAll failed: %s", path.c_str());
        return false;
    }

    if (bytes.empty()) {
        LOG_ERROR(TAG, "loadJson: empty file: %s", path.c_str());
        return false;
    }

    bytes.push_back(0);
    
    const char* json = reinterpret_cast<const char*>(bytes.data());
    const size_t length = bytes.size() - 1;

    out.Parse(json, length);

    if (out.HasParseError()) {
        const auto code = out.GetParseError();
        const size_t off = out.GetErrorOffset();
        LOG_ERROR(TAG, "loadJson: parse error: %s (offset=%u) file=%s", rapidjson::GetParseErrorFunc(code), (unsigned)off, path.c_str());
        return false;
    }

    return true;
}

void AssetManager::clearEmbeddedAssets() {
    embeddedAssets.clear();
}

bool AssetManager::loadDocumentAssets(const rapidjson::Value::ConstArray &assets) {
    constexpr const char* TAG = "AssetManager::loadDocumentAssets";

    clearEmbeddedAssets();

    for (const auto& asset : assets) {
        if (!asset.IsObject()) {
            LOG_ERROR(TAG, "Asset entry must be an object.");
            return false;
        }

        const auto obj = asset.GetObject();

        auto nameIt = obj.FindMember("name");
        auto mediaTypeIt = obj.FindMember("mediaType");
        auto dataIt = obj.FindMember("data");
        if (nameIt == obj.MemberEnd() || !nameIt->value.IsString()) {
            LOG_ERROR(TAG, "Asset is missing string field 'name'.");
            return false;
        }
        if (mediaTypeIt == obj.MemberEnd() || !mediaTypeIt->value.IsString()) {
            LOG_ERROR(TAG, "Asset '%s' is missing string field 'mediaType'.", nameIt->value.IsString() ? nameIt->value.GetString() : "<unnamed>");
            return false;
        }
        if (dataIt == obj.MemberEnd() || !dataIt->value.IsString()) {
            LOG_ERROR(TAG, "Asset '%s' is missing string field 'data'.", nameIt->value.IsString() ? nameIt->value.GetString() : "<unnamed>");
            return false;
        }

        const std::string name(nameIt->value.GetString(), nameIt->value.GetStringLength());
        const std::string mediaType(mediaTypeIt->value.GetString(), mediaTypeIt->value.GetStringLength());
        const std::string_view encoded(dataIt->value.GetString(), dataIt->value.GetStringLength());

        std::vector<uint8_t> decoded(base64DecodedMaxSize(encoded));
        size_t decodedLen = 0;
        if (!base64Decode(encoded, decoded.data(), decoded.size(), decodedLen)) {
            LOG_ERROR(TAG, "Failed to decode base64 asset: %s", name.c_str());
            return false;
        }

        decoded.resize(decodedLen);
        embeddedAssets[name] = EmbeddedAsset{std::move(mediaType), std::move(decoded)};
    }

    return true;
}

bool AssetManager::loadImage(const std::string &p, Image &out) {
    constexpr const char* TAG = "AssetManager::loadImage";

    std::vector<uint8_t> data;
    std::string source = p;

    auto it = embeddedAssets.find(p);
    const EmbeddedAsset* embeddedAsset = nullptr;
    if (it != embeddedAssets.end()) {
        embeddedAsset = &it->second;
        data = embeddedAsset->data;
    } else {
        const std::string path = "/assets/images/" + p;
        if (!fs.exists(path)) {
            LOG_ERROR(TAG, "Image does not exist: %s", path.c_str());
            return false;
        }

        if (!fs.readAll(path, data)) {
            LOG_ERROR(TAG, "Failed to read image: %s", path.c_str());
            return false;
        }

        source = path;
    }

    if (data.empty()) {
        LOG_ERROR(TAG, "Image is empty: %s", source.c_str());
        return false;
    }

    ImageType type = detectImageType(data.data(), data.size());
    if (embeddedAsset) {
        const char* detectedMediaType = imageTypeMediaType(type);
        if (detectedMediaType && embeddedAsset->mediaType != detectedMediaType) {
            LOG_WARN(TAG, "Embedded asset '%s' declared mediaType '%s' but decoded as '%s'.",
                p.c_str(), embeddedAsset->mediaType.c_str(), detectedMediaType);
        }
    }

    ImageInfo info;

    switch (type) {
        case ImageType::BMP: {
            LOG_DEBUG(TAG, "decoding BMP: %s", source.c_str());
            if (!decodeBMP(data.data(), data.size(), info)) return false;
            break;
        }

        case ImageType::PNG:
            LOG_DEBUG(TAG, "decoding PNG: %s", source.c_str());
            if (!decodePNG(data.data(), data.size(), info)) return false;
            break;

        case ImageType::JPG:
            LOG_DEBUG(TAG, "decoding JPG: %s", source.c_str());
            if (!decodeJPG(data.data(), data.size(), info)) return false;
            break;

        default:
            LOG_DEBUG(TAG, "Unsupported image format: %s", source.c_str());
            return false;
    }

    LOG_INFO(TAG, "Successfully loaded image: %s, w=%d, h=%d", source.c_str(), info.width, info.height);
    
    out = ctx.createNativeImage(info.pixels.data(), info.width, info.height);
    return true;
}
