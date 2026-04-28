#pragma once

#include <multigauge/io/FileSystem.h>
#include <multigauge/graphics/GraphicsContext.h>
#include <multigauge/images/Image.h>
#include <multigauge/images/ImageDecoder.h>

#include <rapidjson/document.h>

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

class AssetManager {
    private:
        enum class ImageType {
            Unknown,
            BMP,
            PNG,
            JPG
        };

        FileSystem* fs;

        static ImageType detectImageType(const uint8_t* data, size_t size);
        static const char* imageTypeName(ImageType type);
        static bool decodeBase64(const std::string_view& encoded, std::vector<uint8_t>& out);
        static bool decodeImageData(ImageType type, const std::vector<uint8_t>& data, ImageInfo& info);

    public:
        AssetManager(FileSystem& fs) : fs(&fs) {}

        bool loadJson(const std::string& path, rapidjson::Document& out);
        bool loadDocumentAssets(const rapidjson::Value::ConstArray& assets);
        bool loadImage(GraphicsContext& ctx, const std::string& path, Image& out);
};
