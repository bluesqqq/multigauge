#include "PackageManager.h"

#include "../AppPaths.h"

#include <multigauge/io/Base64.h>
#include <multigauge/io/FileSystem.h>
#include <multigauge/io/Log.h>
#include <multigauge/utils/Json.h>
#include <multigauge/utils/Text.h>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <algorithm>
#include <cctype>
#include <set>
#include <string_view>
#include <utility>
#include <vector>

namespace mg {

namespace {

constexpr const char* TAG = "PackageManager";

using mg::json::getArrayMember;
using mg::json::getObjectMember;
using mg::json::getStringMember;

bool isSafeRelativePath(const std::string& value) {
    if (value.empty()) return false;
    if (value.front() == '/' || value.front() == '\\') return false;
    if (value.find(':') != std::string::npos) return false;

    size_t start = 0;
    while (start < value.size()) {
        size_t end = value.find_first_of("/\\", start);
        const std::string part = value.substr(start, end == std::string::npos ? std::string::npos : end - start);
        if (part.empty() || part == "." || part == "..") return false;
        if (end == std::string::npos) break;
        start = end + 1;
    }

    return true;
}

bool makeDirectoryChain(io::FileSystem& fs, const std::string& path) {
    if (path.empty() || fs.exists(path)) return true;

    std::string current;
    size_t index = 0;

    if (path.size() >= 2 && path[1] == ':') {
        current = path.substr(0, 2);
        index = 2;
        if (index < path.size() && mg::utils::isPathSeparator(path[index])) {
            current.push_back(path[index]);
            ++index;
        }
    } else if (mg::utils::isPathSeparator(path[0])) {
        current.push_back(path[0]);
        index = 1;
    }

    while (index < path.size()) {
        while (index < path.size() && mg::utils::isPathSeparator(path[index])) ++index;
        if (index >= path.size()) break;

        const size_t end = path.find_first_of("/\\", index);
        const std::string segment = path.substr(index, end == std::string::npos ? std::string::npos : end - index);
        if (segment.empty()) return false;

        if (!current.empty() && !mg::utils::isPathSeparator(current.back())) {
            current.push_back('/');
        }
        current.append(segment);

        if (!fs.exists(current) && !fs.makeDirectory(current)) {
            return false;
        }

        if (end == std::string::npos) break;
        index = end + 1;
    }

    return true;
}

bool ensureParentDirectories(io::FileSystem& fs, const std::string& path) {
    const size_t lastSep = path.find_last_of("/\\");
    if (lastSep == std::string::npos) return true;
    return makeDirectoryChain(fs, path.substr(0, lastSep));
}

bool buildLibraryIndex(io::FileSystem& fs, std::string_view dataRoot) {
    if (!makeDirectoryChain(fs, std::string(dataRoot)) ||
        !makeDirectoryChain(fs, paths::packagesRootPath(dataRoot))) {
        return false;
    }

    std::set<std::string> favoriteIds;
    rapidjson::Document previousLibrary;
    if (mg::json::readJsonFile(fs, paths::libraryPath(dataRoot), previousLibrary) && previousLibrary.IsObject()) {
        if (const auto* packages = getArrayMember(previousLibrary, "packages")) {
            for (const auto& item : packages->GetArray()) {
                if (!item.IsObject()) continue;

                std::string id;
                bool favorite = false;
                if (getStringMember(item, "id", id) &&
                    mg::json::getBoolMember(item, "favorite", favorite) &&
                    favorite) {
                    favoriteIds.insert(id);
                }
            }
        }
    }

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    writer.StartObject();
    writer.Key("packages");
    writer.StartArray();

    std::vector<std::string> packageIds;
    fs.listDirectories(paths::packagesRootPath(dataRoot), packageIds);
    std::sort(packageIds.begin(), packageIds.end());

    for (const auto& packageId : packageIds) {
        rapidjson::Document manifest;
        if (!mg::json::readJsonFile(fs, paths::manifestPath(dataRoot, packageId), manifest) || !manifest.IsObject()) {
            continue;
        }

        std::string name;
        std::string author;
        std::string description;
        if (!getStringMember(manifest, "name", name) ||
            !getStringMember(manifest, "author", author) ||
            !getStringMember(manifest, "description", description)) {
            continue;
        }

        writer.StartObject();
        writer.Key("id");
        writer.String(packageId.c_str());
        writer.Key("name");
        writer.String(name.c_str());
        writer.Key("author");
        writer.String(author.c_str());
        writer.Key("description");
        writer.String(description.c_str());
        writer.Key("favorite");
        writer.Bool(favoriteIds.count(packageId) != 0);
        writer.EndObject();
    }

    writer.EndArray();
    writer.EndObject();

    return fs.writeText(paths::libraryPath(dataRoot), buffer.GetString());
}

bool removeTree(io::FileSystem& fs, const std::string& path) {
    if (!fs.exists(path)) {
        return true;
    }

    std::vector<std::string> children;
    if (fs.listDirectories(path, children)) {
        for (const auto& child : children) {
            const std::string childPath = paths::joinPath(path, child);
            if (!removeTree(fs, childPath)) {
                return false;
            }
        }
    }

    return fs.remove(path);
}

Result readJsonFile(io::FileSystem& fs, const std::string& path) {
    if (!fs.exists(path)) {
        return Error("File not found");
    }

    rapidjson::Document json;
    if (!mg::json::readJsonFile(fs, path, json)) {
        return Error("Invalid JSON");
    }

    Result result;
    result.ok = true;
    result.data.Swap(json);
    return result;
}

std::string slugify(const std::string& name) {
    std::string out;
    bool lastHyphen = false;

    for (unsigned char c : name) {
        if (std::isalnum(c)) {
            out.push_back(static_cast<char>(std::tolower(c)));
            lastHyphen = false;
        } else if (!out.empty() && !lastHyphen) {
            out.push_back('-');
            lastHyphen = true;
        }
    }

    while (!out.empty() && out.back() == '-') {
        out.pop_back();
    }

    if (out.empty()) out = "face";
    return out;
}

std::string uniqueSlug(const std::string& name, std::set<std::string>& usedIds) {
    const std::string base = slugify(name);
    std::string candidate = base;
    int suffix = 2;

    while (usedIds.count(candidate) != 0) {
        candidate = base + "-" + std::to_string(suffix++);
    }

    usedIds.insert(candidate);
    return candidate;
}

bool decodeAssetBytes(const rapidjson::Value& assetValue, std::vector<uint8_t>& out) {
    std::string data;
    if (!getStringMember(assetValue, "data", data)) {
        return false;
    }

    const std::string_view encoded(data);
    out.resize(io::base64DecodedMaxSize(encoded));

    size_t decodedLen = 0;
    if (!io::base64Decode(encoded, out.data(), out.size(), decodedLen)) {
        return false;
    }

    out.resize(decodedLen);
    return true;
}

bool writeFaceFile(io::FileSystem& fs, const std::string& path, const rapidjson::Document& faceDoc) {
    if (!ensureParentDirectories(fs, path)) return false;
    return mg::json::writeJsonFile(fs, path, faceDoc);
}

bool writeAssetFile(io::FileSystem& fs, const std::string& path, const std::vector<uint8_t>& bytes) {
    if (!ensureParentDirectories(fs, path)) return false;
    return fs.writeBytes(path, bytes.data(), bytes.size());
}

std::vector<std::string> readPackageDirectories(io::FileSystem& fs, const std::string& root) {
    std::vector<std::string> out;
    fs.listDirectories(root, out);
    return out;
}

} // namespace

PackageManager::PackageManager(io::FileSystem& fs, std::string dataRoot)
    : fs(fs), dataRoot(std::move(dataRoot)) {
}

Result PackageManager::listPackages() const {
    const std::string path = paths::libraryPath(dataRoot);
    if (!fs.exists(path)) {
        return Error("File not found");
    }

    Result result = readJsonFile(fs, path);
    if (result.ok) return result;

    return Error("Invalid library index");
}

Result PackageManager::getPackage(const std::string& packageId) const {
    return readJsonFile(fs, paths::manifestPath(dataRoot, packageId));
}

Result PackageManager::getFace(const std::string& packageId, const std::string& faceId) const {
    return readJsonFile(fs, paths::facePath(dataRoot, packageId, faceId));
}

Result PackageManager::importPackage(const std::string& json) {
    LOG_INFO(TAG, "importPackage: input bytes=%u", static_cast<unsigned>(json.size()));

    rapidjson::Document input = mg::json::parseJson(json);
    if (input.HasParseError() || !input.IsObject()) {
        LOG_ERROR(TAG, "importPackage: invalid package JSON");
        return Error("Invalid package JSON");
    }

    std::string packageName;
    std::string packageAuthor;
    std::string packageDescription;

    if (!getStringMember(input, "name", packageName) ||
        !getStringMember(input, "author", packageAuthor) ||
        !getStringMember(input, "description", packageDescription)) {
        LOG_ERROR(TAG, "importPackage: missing required package fields");
        return Error("Package JSON is missing required fields");
    }

    const rapidjson::Value* facesValue = getArrayMember(input, "faces");
    if (!facesValue) {
        LOG_ERROR(TAG, "importPackage: missing faces array");
        return Error("Package has no faces");
    }

    const rapidjson::Value* assetsValue = getArrayMember(input, "assets");
    std::set<std::string> usedPackageIds;
    for (const auto& packageId : readPackageDirectories(fs, paths::packagesRootPath(dataRoot))) {
        usedPackageIds.insert(packageId);
    }
    const std::string packageId = uniqueSlug(packageName, usedPackageIds);
    LOG_INFO(TAG,
             "importPackage: packageId=%s name=%s faces=%u assets=%s",
             packageId.c_str(),
             packageName.c_str(),
             static_cast<unsigned>(facesValue->Size()),
             assetsValue ? "yes" : "no");

    const std::string packageRoot = paths::packagePath(dataRoot, packageId);
    if (!makeDirectoryChain(fs, dataRoot) ||
        !makeDirectoryChain(fs, paths::packagesRootPath(dataRoot)) ||
        !makeDirectoryChain(fs, packageRoot) ||
        !makeDirectoryChain(fs, paths::facesRootPath(dataRoot, packageId)) ||
        !makeDirectoryChain(fs, paths::assetsPath(dataRoot, packageId))) {
        LOG_ERROR(TAG, "importPackage: failed to create package directories for %s", packageId.c_str());
        return Error("Failed to create package directories");
    }

    std::set<std::string> usedFaceIds;
    rapidjson::StringBuffer manifestBuffer;
    rapidjson::Writer<rapidjson::StringBuffer> manifestWriter(manifestBuffer);
    manifestWriter.StartObject();
    manifestWriter.Key("id");
    manifestWriter.String(packageId.c_str());
    manifestWriter.Key("name");
    manifestWriter.String(packageName.c_str());
    manifestWriter.Key("author");
    manifestWriter.String(packageAuthor.c_str());
    manifestWriter.Key("description");
    manifestWriter.String(packageDescription.c_str());
    manifestWriter.Key("faces");
    manifestWriter.StartArray();
    unsigned faceIndex = 0;
    for (const auto& faceValue : facesValue->GetArray()) {
        ++faceIndex;
        if (!faceValue.IsObject()) {
            LOG_ERROR(TAG, "importPackage: face %u is not an object", faceIndex);
            return Error("Face entries must be objects");
        }

        std::string faceName;
        if (!getStringMember(faceValue, "name", faceName)) {
            LOG_ERROR(TAG, "importPackage: face %u missing name", faceIndex);
            removeTree(fs, packageRoot);
            return Error("Face entry is missing a name");
        }

        const rapidjson::Value* faceDocValue = getObjectMember(faceValue, "face");
        if (!faceDocValue || !faceDocValue->IsObject()) {
            LOG_ERROR(TAG, "importPackage: face %u missing payload", faceIndex);
            removeTree(fs, packageRoot);
            return Error("Face entry is missing face payload");
        }

        const std::string faceId = uniqueSlug(faceName, usedFaceIds);
        const std::string faceStoragePath = paths::facePath(dataRoot, packageId, faceId);
        const std::string faceJson = mg::json::toString(*faceDocValue);
        LOG_INFO(TAG,
                 "importPackage: face %u id=%s",
                 faceIndex,
                 faceId.c_str());
        LOG_DEBUG(TAG, "importPackage: writing face file %s (%u bytes)", faceStoragePath.c_str(), static_cast<unsigned>(faceJson.size()));
        if (!fs.writeText(faceStoragePath, faceJson)) {
            LOG_ERROR(TAG, "importPackage: failed to write face file %s", faceStoragePath.c_str());
            removeTree(fs, packageRoot);
            return Error("Failed to write face file");
        }

        manifestWriter.StartObject();
        manifestWriter.Key("id");
        manifestWriter.String(faceId.c_str());
        manifestWriter.Key("name");
        manifestWriter.String(faceName.c_str());
        manifestWriter.Key("path");
        const std::string faceRelPath = paths::joinPath(paths::facesDir, faceId + ".json");
        manifestWriter.String(faceRelPath.c_str());
        manifestWriter.Key("face");
        faceDocValue->Accept(manifestWriter);
        manifestWriter.EndObject();
    }

    manifestWriter.EndArray();
    manifestWriter.EndObject();

    if (assetsValue) {
        if (!assetsValue->IsArray()) {
            LOG_ERROR(TAG, "importPackage: assets is not an array");
            removeTree(fs, packageRoot);
            return Error("Package assets must be an array");
        }

        for (const auto& assetValue : assetsValue->GetArray()) {
            if (!assetValue.IsObject()) {
                LOG_ERROR(TAG, "importPackage: asset entry is not an object");
                removeTree(fs, packageRoot);
                return Error("Asset entries must be objects");
            }

            std::string assetPath;
            if (!getStringMember(assetValue, "path", assetPath)) {
                if (!getStringMember(assetValue, "name", assetPath)) {
                    LOG_ERROR(TAG, "importPackage: asset entry missing path");
                    removeTree(fs, packageRoot);
                    return Error("Asset entry is missing a path");
                }
            }

            if (!isSafeRelativePath(assetPath)) {
                LOG_ERROR(TAG, "importPackage: unsafe asset path %s", assetPath.c_str());
                removeTree(fs, packageRoot);
                return Error("Invalid asset path");
            }

            std::vector<uint8_t> bytes;
            if (!decodeAssetBytes(assetValue, bytes)) {
                LOG_ERROR(TAG, "importPackage: failed to decode asset data for %s", assetPath.c_str());
                removeTree(fs, packageRoot);
                return Error("Failed to decode asset data");
            }

            const std::string fullPath = paths::joinPath(paths::assetsPath(dataRoot, packageId), assetPath);
            LOG_DEBUG(TAG, "importPackage: writing asset %s (%u bytes)", fullPath.c_str(), static_cast<unsigned>(bytes.size()));
            if (!writeAssetFile(fs, fullPath, bytes)) {
                LOG_ERROR(TAG, "importPackage: failed to write asset %s", fullPath.c_str());
                removeTree(fs, packageRoot);
                return Error("Failed to write asset file");
            }
        }
    }

    const std::string manifestJson = manifestBuffer.GetString();
    LOG_INFO(TAG, "importPackage: writing manifest %s (%u bytes)", paths::manifestPath(dataRoot, packageId).c_str(), static_cast<unsigned>(manifestJson.size()));
    if (!fs.writeText(paths::manifestPath(dataRoot, packageId), manifestJson)) {
        LOG_ERROR(TAG, "importPackage: failed to write manifest");
        removeTree(fs, packageRoot);
        return Error("Failed to write package manifest");
    }

    std::string manifestCheck;
    if (fs.readText(paths::manifestPath(dataRoot, packageId), manifestCheck)) {
        const std::string preview = manifestCheck.substr(0, std::min<size_t>(manifestCheck.size(), 180));
        LOG_INFO(TAG, "importPackage: manifest readback bytes=%u preview=%s",
                 static_cast<unsigned>(manifestCheck.size()),
                 preview.c_str());
    } else {
        LOG_WARN(TAG, "importPackage: failed to read back manifest for verification");
    }

    LOG_INFO(TAG, "importPackage: writing library index");
    if (!buildLibraryIndex(fs, dataRoot)) {
        LOG_ERROR(TAG, "importPackage: failed to write library index");
        removeTree(fs, packageRoot);
        return Error("Failed to update library index");
    }

    LOG_INFO(TAG, "importPackage: done");
    return OkObject();
}

Result PackageManager::exportPackage(const std::string& packageId) const {
    rapidjson::Document manifest;
    if (!mg::json::readJsonFile(fs, paths::manifestPath(dataRoot, packageId), manifest) || !manifest.IsObject()) {
        return Error("Package not found");
    }

    const rapidjson::Value* facesValue = getArrayMember(manifest, "faces");
    if (!facesValue) {
        return Error("Package manifest is invalid");
    }

    rapidjson::Document exportDoc;
    exportDoc.SetObject();
    auto& allocator = exportDoc.GetAllocator();

    const char* keys[] = {"name", "author", "description"};
    for (const char* key : keys) {
        if (!manifest.HasMember(key)) {
            return Error("Package manifest is invalid");
        }
        exportDoc.AddMember(rapidjson::Value(key, allocator), manifest[key], allocator);
    }

    rapidjson::Value facesOut(rapidjson::kArrayType);
    for (const auto& faceEntry : facesValue->GetArray()) {
        if (!faceEntry.IsObject()) {
            return Error("Package manifest is invalid");
        }

        std::string faceId;
        std::string faceName;
        std::string facePath;
        if (!getStringMember(faceEntry, "id", faceId) ||
            !getStringMember(faceEntry, "name", faceName) ||
            !getStringMember(faceEntry, "path", facePath)) {
            return Error("Package manifest is invalid");
        }

        rapidjson::Document faceDoc;
        const std::string fullFacePath = paths::joinPath(paths::packagePath(dataRoot, packageId), facePath);
        if (!mg::json::readJsonFile(fs, fullFacePath, faceDoc) || !faceDoc.IsObject()) {
            if (!mg::json::readJsonFile(fs, paths::facePath(dataRoot, packageId, faceId), faceDoc) || !faceDoc.IsObject()) {
                return Error("Face file not found");
            }
        }

        rapidjson::Value entry(rapidjson::kObjectType);
        entry.AddMember("name", rapidjson::Value(faceName.c_str(), allocator), allocator);
        rapidjson::Value faceValue;
        faceValue.CopyFrom(faceDoc, allocator);
        entry.AddMember("face", faceValue, allocator);
        facesOut.PushBack(entry, allocator);
    }

    exportDoc.AddMember("faces", std::move(facesOut), allocator);
    Result result;
    result.ok = true;
    result.data.Swap(exportDoc);
    return result;
}

Result PackageManager::removePackage(const std::string& packageId) {
    const std::string packageRoot = paths::packagePath(dataRoot, packageId);
    if (!fs.exists(packageRoot)) {
        return Error("Package not found");
    }

    if (!removeTree(fs, packageRoot)) {
        return Error("Failed to remove package");
    }

    if (!buildLibraryIndex(fs, dataRoot)) {
        return Error("Failed to update library index");
    }
    return OkObject();
}

void PackageManager::rebuildLibrary() const {
    (void)buildLibraryIndex(fs, dataRoot);
}

} // namespace mg
