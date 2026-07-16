#pragma once

#include <multigauge/io/FileSystem.h>
#include <multigauge/graphics/GraphicsContext.h>
#include <multigauge/graphics/image/Image.h>
#include <multigauge/graphics/image/ImageDecoder.h>

#include <rapidjson/document.h>

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

namespace mg {

class AssetManager {
    private:
        enum class ImageType {
            Unknown,
            BMP,
            PNG,
            JPG
        };

        io::FileSystem* fs;

        static ImageType detectImageType(const uint8_t* data, size_t size);
        static const char* imageTypeName(ImageType type);
        static bool decodeBase64(const std::string_view& encoded, std::vector<uint8_t>& out);
        static bool decodeImageData(ImageType type, const std::vector<uint8_t>& data, images::ImageInfo& info);

    public:
        AssetManager(io::FileSystem& fs) : fs(&fs) {}

        bool loadJson(const std::string& path, rapidjson::Document& out);
        bool loadDocumentAssets(const rapidjson::Value::ConstArray& assets);
        bool loadImage(graphics::GraphicsContext& ctx, const std::string& path, images::Image& out);
};

}
