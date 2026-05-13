#include "PackageManager.h"

#include "../AppPaths.h"

#include <multigauge/io/Base64.h>
#include <multigauge/io/FileSystem.h>
#include <multigauge/utils/Json.h>
#include <multigauge/utils/Text.h>

#include <rapidjson/document.h>

#include <algorithm>
#include <cctype>
#include <set>
#include <string_view>
#include <utility>
#include <vector>

namespace mg {

namespace {

using mg::json::getArrayMember;
using mg::json::getObjectMember;
using mg::json::getStringMember;

bool isSafePathComponent(const std::string& value) {
    if (value.empty()) return false;
    if (value == "." || value == "..") return false;

    for (char c : value) {
        if (mg::utils::isPathSeparator(c) || c == ':') return false;
    }

    return true;
}

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

Result readJsonFile(io::FileSystem& fs, const std::string& path) {
    if (!fs.exists(path)) {
        return Error("File not found");
    }

    rapidjson::Document json;
    if (!mg::json::readJsonFile(fs, path, json)) {
        return Error("Invalid JSON");
    }

    Result result = OkObject();
    result.data.CopyFrom(json, result.data.GetAllocator());
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

bool makeFacePayload(const rapidjson::Value& faceEntry, rapidjson::Document& out) {
    out.SetObject();

    const rapidjson::Value* payload = &faceEntry;

    if (faceEntry.IsObject()) {
        if (const auto* faceValue = getObjectMember(faceEntry, "face")) {
            payload = faceValue;
        } else if (const auto* documentValue = getObjectMember(faceEntry, "document")) {
            payload = documentValue;
        } else {
            std::string json;
            if (getStringMember(faceEntry, "json", json)) {
                out = mg::json::parseJson(json);
                return out.IsObject();
            }
            if (const auto* jsonObject = getObjectMember(faceEntry, "json")) {
                payload = jsonObject;
            }
        }
    }

    out.CopyFrom(*payload, out.GetAllocator());

    if (out.HasMember("id")) out.RemoveMember("id");
    if (out.HasMember("name")) out.RemoveMember("name");
    if (out.HasMember("path")) out.RemoveMember("path");
    if (out.HasMember("face")) out.RemoveMember("face");
    if (out.HasMember("document")) out.RemoveMember("document");
    if (out.HasMember("json")) out.RemoveMember("json");
    if (out.HasMember("assets")) out.RemoveMember("assets");

    return true;
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
        rebuildLibrary();
    }

    Result result = readJsonFile(fs, path);
    if (result.ok) return result;

    rebuildLibrary();
    return readJsonFile(fs, path);
}

Result PackageManager::getPackage(const std::string& packageId) const {
    return readJsonFile(fs, paths::manifestPath(dataRoot, packageId));
}

Result PackageManager::getFace(const std::string& packageId, const std::string& faceId) const {
    return readJsonFile(fs, paths::facePath(dataRoot, packageId, faceId));
}

Result PackageManager::importPackage(const std::string& json) {
    rapidjson::Document input = mg::json::parseJson(json);
    if (input.HasParseError() || !input.IsObject()) {
        return Error("Invalid package JSON");
    }

    const rapidjson::Value* manifest = &input;
    if (const auto* manifestObject = getObjectMember(input, "manifest")) {
        manifest = manifestObject;
    }

    std::string packageId;
    std::string packageName;
    std::string packageAuthor;
    std::string packageDescription;

    if (!getStringMember(*manifest, "id", packageId) ||
        !getStringMember(*manifest, "name", packageName) ||
        !getStringMember(*manifest, "author", packageAuthor) ||
        !getStringMember(*manifest, "description", packageDescription)) {
        return Error("Package manifest is missing required fields");
    }

    if (!isSafePathComponent(packageId)) {
        return Error("Invalid package id");
    }

    const rapidjson::Value* facesValue = getArrayMember(input, "faces");
    if (!facesValue) {
        facesValue = getArrayMember(*manifest, "faces");
    }
    if (!facesValue) {
        return Error("Package has no faces");
    }

    const rapidjson::Value* assetsValue = getArrayMember(input, "assets");
    if (!assetsValue && manifest != &input) {
        assetsValue = getArrayMember(*manifest, "assets");
    }

    const std::string packageRoot = paths::packagePath(dataRoot, packageId);
    if (fs.exists(packageRoot) && !fs.remove(packageRoot)) {
        return Error("Failed to replace existing package");
    }

    if (!makeDirectoryChain(fs, dataRoot) ||
        !makeDirectoryChain(fs, paths::packagesRootPath(dataRoot)) ||
        !makeDirectoryChain(fs, packageRoot) ||
        !makeDirectoryChain(fs, paths::facesRootPath(dataRoot, packageId)) ||
        !makeDirectoryChain(fs, paths::assetsPath(dataRoot, packageId))) {
        return Error("Failed to create package directories");
    }

    std::set<std::string> usedFaceIds;
    rapidjson::Document manifestOut;
    manifestOut.SetObject();
    auto& manifestAllocator = manifestOut.GetAllocator();

    manifestOut.AddMember("id", rapidjson::Value(packageId.c_str(), manifestAllocator), manifestAllocator);
    manifestOut.AddMember("name", rapidjson::Value(packageName.c_str(), manifestAllocator), manifestAllocator);
    manifestOut.AddMember("author", rapidjson::Value(packageAuthor.c_str(), manifestAllocator), manifestAllocator);
    manifestOut.AddMember("description", rapidjson::Value(packageDescription.c_str(), manifestAllocator), manifestAllocator);

    rapidjson::Value facesOut(rapidjson::kArrayType);
    for (const auto& faceValue : facesValue->GetArray()) {
        if (!faceValue.IsObject()) {
            fs.remove(packageRoot);
            return Error("Face entries must be objects");
        }

        std::string faceName;
        if (!getStringMember(faceValue, "name", faceName)) {
            fs.remove(packageRoot);
            return Error("Face entry is missing a name");
        }

        rapidjson::Document faceDoc;
        if (!makeFacePayload(faceValue, faceDoc) || !faceDoc.IsObject()) {
            fs.remove(packageRoot);
            return Error("Invalid face payload");
        }

        const std::string faceId = uniqueSlug(faceName, usedFaceIds);
        const std::string facePath = paths::facePath(dataRoot, packageId, faceId);

        if (!writeFaceFile(fs, facePath, faceDoc)) {
            fs.remove(packageRoot);
            return Error("Failed to write face file");
        }

        rapidjson::Value faceEntry(rapidjson::kObjectType);
        const std::string faceRelPath = paths::joinPath(paths::facesDir, faceId + ".json");
        faceEntry.AddMember("id", rapidjson::Value(faceId.c_str(), manifestAllocator), manifestAllocator);
        faceEntry.AddMember("name", rapidjson::Value(faceName.c_str(), manifestAllocator), manifestAllocator);
        faceEntry.AddMember("path", rapidjson::Value(faceRelPath.c_str(), manifestAllocator), manifestAllocator);
        facesOut.PushBack(faceEntry, manifestAllocator);
    }

    manifestOut.AddMember("faces", std::move(facesOut), manifestAllocator);

    if (assetsValue) {
        if (!assetsValue->IsArray()) {
            fs.remove(packageRoot);
            return Error("Package assets must be an array");
        }

        for (const auto& assetValue : assetsValue->GetArray()) {
            if (!assetValue.IsObject()) {
                fs.remove(packageRoot);
                return Error("Asset entries must be objects");
            }

            std::string assetPath;
            if (!getStringMember(assetValue, "path", assetPath) && !getStringMember(assetValue, "name", assetPath)) {
                fs.remove(packageRoot);
                return Error("Asset entry is missing a path");
            }

            if (!isSafeRelativePath(assetPath)) {
                fs.remove(packageRoot);
                return Error("Invalid asset path");
            }

            std::vector<uint8_t> bytes;
            if (!decodeAssetBytes(assetValue, bytes)) {
                fs.remove(packageRoot);
                return Error("Failed to decode asset data");
            }

            const std::string fullPath = paths::joinPath(paths::assetsPath(dataRoot, packageId), assetPath);
            if (!writeAssetFile(fs, fullPath, bytes)) {
                fs.remove(packageRoot);
                return Error("Failed to write asset file");
            }
        }
    }

    if (!mg::json::writeJsonFile(fs, paths::manifestPath(dataRoot, packageId), manifestOut)) {
        fs.remove(packageRoot);
        return Error("Failed to write package manifest");
    }

    rebuildLibrary();
    Result result = OkObject();
    result.data.CopyFrom(manifestOut, result.data.GetAllocator());
    return result;
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

    const char* keys[] = {"id", "name", "author", "description"};
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
        entry.AddMember("id", rapidjson::Value(faceId.c_str(), allocator), allocator);
        entry.AddMember("name", rapidjson::Value(faceName.c_str(), allocator), allocator);
        entry.AddMember("path", rapidjson::Value(facePath.c_str(), allocator), allocator);
        rapidjson::Value faceValue;
        faceValue.CopyFrom(faceDoc, allocator);
        entry.AddMember("face", faceValue, allocator);
        facesOut.PushBack(entry, allocator);
    }

    exportDoc.AddMember("faces", std::move(facesOut), allocator);
    Result result = OkObject();
    result.data.CopyFrom(exportDoc, result.data.GetAllocator());
    return result;
}

Result PackageManager::removePackage(const std::string& packageId) {
    const std::string packageRoot = paths::packagePath(dataRoot, packageId);
    if (!fs.exists(packageRoot)) {
        return Error("Package not found");
    }

    if (!fs.remove(packageRoot)) {
        return Error("Failed to remove package");
    }

    rebuildLibrary();
    return OkObject();
}

void PackageManager::rebuildLibrary() const {
    const std::string packagesRoot = paths::packagesRootPath(dataRoot);

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

    rapidjson::Document library;
    library.SetObject();
    auto& allocator = library.GetAllocator();
    rapidjson::Value packages(rapidjson::kArrayType);

    std::vector<std::string> packageIds = readPackageDirectories(fs, packagesRoot);
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

        rapidjson::Value entry(rapidjson::kObjectType);
        entry.AddMember("id", rapidjson::Value(packageId.c_str(), allocator), allocator);
        entry.AddMember("name", rapidjson::Value(name.c_str(), allocator), allocator);
        entry.AddMember("author", rapidjson::Value(author.c_str(), allocator), allocator);
        entry.AddMember("description", rapidjson::Value(description.c_str(), allocator), allocator);
        entry.AddMember("favorite", favoriteIds.count(packageId) != 0, allocator);
        packages.PushBack(entry, allocator);
    }

    library.AddMember("packages", std::move(packages), allocator);
    mg::json::writeJsonFile(fs, paths::libraryPath(dataRoot), library);
}

} // namespace mg
