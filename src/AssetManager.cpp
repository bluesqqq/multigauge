#include <multigauge/AssetManager.h>

#include <multigauge/io/Base64.h>
#include <multigauge/images/ImageDecoder.h>
#include <multigauge/io/Log.h>

bool AssetManager::loadJson(const std::string &path, rapidjson::Document &out) {
    constexpr const char* TAG = "AssetManager::loadJson";

    LOG_DEBUG(TAG, "Reading gauge document: %s", path.c_str());

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

    LOG_INFO(TAG, "Loaded gauge document: %s (%u bytes)", path.c_str(), (unsigned)length);
    return true;
}

void AssetManager::clearEmbeddedAssets() {
    constexpr const char* TAG = "AssetManager::clearEmbeddedAssets";
    if (!embeddedAssets.empty()) {
        LOG_DEBUG(TAG, "Clearing %u embedded assets", (unsigned)embeddedAssets.size());
    }
    embeddedAssets.clear();
}

bool AssetManager::loadDocumentAssets(const rapidjson::Value::ConstArray &assets) {
    constexpr const char* TAG = "AssetManager::loadDocumentAssets";

    clearEmbeddedAssets();
    LOG_INFO(TAG, "Loading %u embedded assets from gauge document", (unsigned)assets.Size());

    size_t totalDecodedBytes = 0;

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

        LOG_DEBUG(TAG, "Decoding embedded asset '%s' mediaType='%s' encodedBytes=%u",
            name.c_str(), mediaType.c_str(), (unsigned)encoded.size());

        std::vector<uint8_t> decoded(base64DecodedMaxSize(encoded));
        size_t decodedLen = 0;
        if (!base64Decode(encoded, decoded.data(), decoded.size(), decodedLen)) {
            LOG_ERROR(TAG, "Failed to decode base64 asset: %s", name.c_str());
            return false;
        }

        decoded.resize(decodedLen);
        if (embeddedAssets.find(name) != embeddedAssets.end()) {
            LOG_WARN(TAG, "Embedded asset '%s' is duplicated; overwriting previous entry.", name.c_str());
        }
        embeddedAssets[name] = EmbeddedAsset{std::move(mediaType), std::move(decoded)};
        totalDecodedBytes += decodedLen;

        LOG_DEBUG(TAG, "Registered embedded asset '%s' decodedBytes=%u",
            name.c_str(), (unsigned)decodedLen);
    }

    LOG_INFO(TAG, "Registered %u embedded assets (%u decoded bytes)",
        (unsigned)embeddedAssets.size(), (unsigned)totalDecodedBytes);
    return true;
}

bool AssetManager::loadImage(const std::string &p, Image &out) {
    constexpr const char* TAG = "AssetManager::loadImage";

    LOG_DEBUG(TAG, "Loading image asset '%s'", p.c_str());

    std::vector<uint8_t> data;
    std::string source = p;

    auto it = embeddedAssets.find(p);
    const EmbeddedAsset* embeddedAsset = nullptr;
    if (it != embeddedAssets.end()) {
        embeddedAsset = &it->second;
        data = embeddedAsset->data;
        LOG_DEBUG(TAG, "Resolved image '%s' from embedded assets (mediaType='%s', bytes=%u)",
            p.c_str(), embeddedAsset->mediaType.c_str(), (unsigned)data.size());
    } else {
        const std::string path = "/assets/images/" + p;
        LOG_DEBUG(TAG, "Resolved image '%s' to filesystem path '%s'", p.c_str(), path.c_str());
        if (!fs.exists(path)) {
            LOG_ERROR(TAG, "Image does not exist: %s", path.c_str());
            return false;
        }

        if (!fs.readAll(path, data)) {
            LOG_ERROR(TAG, "Failed to read image: %s", path.c_str());
            return false;
        }

        source = path;
        LOG_DEBUG(TAG, "Read image '%s' from filesystem (%u bytes)", source.c_str(), (unsigned)data.size());
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

    LOG_DEBUG(TAG, "Detected image type for '%s': %s (%u bytes)",
        source.c_str(), imageTypeName(type), (unsigned)data.size());

    ImageInfo info;

    switch (type) {
        case ImageType::BMP: {
            LOG_DEBUG(TAG, "decoding BMP: %s", source.c_str());
            if (!decodeBMP(data.data(), data.size(), info)) {
                LOG_ERROR(TAG, "Failed to decode BMP image: %s", source.c_str());
                return false;
            }
            break;
        }

        case ImageType::PNG:
            LOG_DEBUG(TAG, "decoding PNG: %s", source.c_str());
            if (!decodePNG(data.data(), data.size(), info)) {
                LOG_ERROR(TAG, "Failed to decode PNG image: %s", source.c_str());
                return false;
            }
            break;

        case ImageType::JPG:
            LOG_DEBUG(TAG, "decoding JPG: %s", source.c_str());
            if (!decodeJPG(data.data(), data.size(), info)) {
                LOG_ERROR(TAG, "Failed to decode JPG image: %s", source.c_str());
                return false;
            }
            break;

        default:
            LOG_ERROR(TAG, "Unsupported image format: %s (size=%u)", source.c_str(), (unsigned)data.size());
            return false;
    }

    LOG_INFO(TAG, "Decoded image: %s (type=%s, bytes=%u, w=%d, h=%d)",
        source.c_str(), imageTypeName(type), (unsigned)data.size(), info.width, info.height);

    LOG_DEBUG(TAG, "Creating native image for '%s'", source.c_str());
    out = ctx.createNativeImage(info.pixels.data(), info.width, info.height);
    if (out.empty()) {
        LOG_ERROR(TAG, "Failed to create native image: %s (w=%d, h=%d)",
            source.c_str(), info.width, info.height);
        return false;
    }

    LOG_INFO(TAG, "Native image ready: %s (w=%d, h=%d)", source.c_str(), out.width, out.height);
    return true;
}
