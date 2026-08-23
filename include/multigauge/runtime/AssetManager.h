#pragma once

#include <multigauge/io/FileSystem.h>
#include <multigauge/graphics/GraphicsContext.h>
#include <multigauge/graphics/image/Image.h>
#include <multigauge/graphics/image/ImageDecoder.h>

#include <multigauge/json/Json.h>

#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace mg {

class AssetManager {
    private:
        io::FileSystem* fs;
        std::string dataRoot;

    public:
        AssetManager(io::FileSystem& fs, std::string dataRoot) : fs(&fs), dataRoot(std::move(dataRoot)) {}

        /// @brief Reads and parses a JSON document from the filesystem.
        /// @param path Filesystem path to the JSON document.
        /// @param out Receives the parsed document when this function returns true.
        /// @return True when the file is non-empty and contains valid JSON.
        bool loadJson(const std::string& path, json::Document& out);
        
        /// @brief Decodes and writes one Base64-encoded asset for a package, or the raw asset directory when `packageId` is empty.
        /// @param packageId Installed package ID, or empty for raw/editor assets.
        /// @param name Flat asset filename.
        /// @param data Base64-encoded asset content.
        bool writeAsset(std::string_view packageId, std::string_view name, std::string_view data);

        /// @brief Removes one asset for a package, or the raw asset directory when `packageId` is empty.
        /// @param packageId Installed package ID, or empty for raw/editor assets.
        /// @param name Flat asset filename.
        bool removeAsset(std::string_view packageId, std::string_view name);

        /// @brief Loads an image by logical filename from a package, or from the raw asset directory when `packageId` is empty.
        /// @param packageId Installed package ID, or empty for raw/editor assets.
        /// @param path Flat image filename.
        bool loadImage(graphics::GraphicsContext& ctx, std::string_view packageId, const std::string& path, images::Image& out);
};

}
