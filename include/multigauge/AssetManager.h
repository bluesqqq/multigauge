#pragma once

#include <multigauge/io/FileSystem.h>
#include <multigauge/graphics/GraphicsContext.h>
#include <multigauge/images/Image.h>

#include <rapidjson/document.h>

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

class AssetManager {
    private:
        struct EmbeddedAsset {
            std::string mediaType;
            std::vector<uint8_t> data;
        };

        FileSystem& fs;
        GraphicsContext& ctx;
        std::unordered_map<std::string, EmbeddedAsset> embeddedAssets;

        enum class ImageType {
            Unknown,
            BMP,
            PNG,
            JPG
        };

        static ImageType detectImageType(const uint8_t* data, size_t size) {
            if (size >= 2 && data[0] == 'B' && data[1] == 'M') {
                return ImageType::BMP;
            }

            // PNG signature: 89 50 4E 47 0D 0A 1A 0A
            if (size >= 8 &&
                data[0] == 0x89 && data[1] == 0x50 &&
                data[2] == 0x4E && data[3] == 0x47 &&
                data[4] == 0x0D && data[5] == 0x0A &&
                data[6] == 0x1A && data[7] == 0x0A) {
                return ImageType::PNG;
            }

            // JPG signature: FF D8 FF
            if (size >= 3 &&
                data[0] == 0xFF && data[1] == 0xD8 && data[2] == 0xFF) {
                return ImageType::JPG;
            }

            return ImageType::Unknown;
        }

        static const char* imageTypeMediaType(ImageType type) {
            switch (type) {
                case ImageType::BMP: return "image/bmp";
                case ImageType::PNG: return "image/png";
                case ImageType::JPG: return "image/jpeg";
                default: return nullptr;
            }
        }

    public:
        AssetManager(FileSystem& fs, GraphicsContext& ctx) : fs(fs), ctx(ctx) {}

        bool loadJson(const std::string& path, rapidjson::Document& out);
        void clearEmbeddedAssets();
        bool loadDocumentAssets(const rapidjson::Value::ConstArray& assets);

        bool loadImage(const std::string& path, Image& out);
};
