#include <multigauge/runtime/PackageManager.h>

#include "../AppPaths.h"

#include <multigauge/io/FileSystem.h>
#include <multigauge/io/Base64.h>
#include <multigauge/io/Log.h>
#include <multigauge/utils/Json.h>
#include <multigauge/utils/Text.h>

#include <algorithm>
#include <cctype>
#include <string_view>
#include <utility>
#include <vector>

namespace mg {

namespace {

constexpr const char* TAG = "PackageManager";
constexpr std::size_t maxAssetCount = 16;
constexpr std::size_t maxAssetEncodedBytes = 64 * 1024;
constexpr std::size_t maxPackageAssetBytes = 128 * 1024;

using mg::json::getStringMember;

struct LibraryIndexPackage {
    std::string id;
    std::string name;
    std::string author;
};

bool writePackageEntry(json::ArrayWriter& writer, const LibraryIndexPackage& package) {
    return writer.writeObject([&](json::ObjectWriter& object) {
        return object.write("id", package.id) && object.write("name", package.name) && object.write("author", package.author);
    });
}

bool isSupportedAssetMediaType(std::string_view value) {
    return value == "image/png" || value == "image/jpeg" || value == "image/bmp";
}

struct AssetMetadata {
    std::string name;
    std::string mediaType;
};

bool readAssetMetadata(json::Reader assets, std::vector<AssetMetadata>& out, std::string& error) {
    if (!assets.valid()) return true;
    if (!assets.isArray() || assets.size() > maxAssetCount) {
        error = "Package assets must be an array with at most " + std::to_string(maxAssetCount) + " entries";
        return false;
    }

    std::vector<AssetMetadata> metadata;
    metadata.reserve(assets.size());
    std::size_t totalEncodedBytes = 0;
    for (std::size_t index = 0; index < assets.size(); ++index) {
        const json::Reader asset = assets.element(index);
        AssetMetadata entry;
        std::string_view data;
        if (!asset.isObject() || asset.size() != 3 ||
            !getStringMember(asset, "name", entry.name) || !utils::isSafeFileName(entry.name) ||
            !getStringMember(asset, "mediaType", entry.mediaType) || !isSupportedAssetMediaType(entry.mediaType) ||
            !asset.member("data").read(data) || data.empty() || data.size() > maxAssetEncodedBytes) {
            error = "Package contains an invalid embedded asset";
            return false;
        }
        if (std::any_of(metadata.begin(), metadata.end(), [&](const AssetMetadata& other) {
                return other.name == entry.name;
            })) {
            error = "Package contains duplicate asset names";
            return false;
        }
        totalEncodedBytes += data.size();
        if (totalEncodedBytes > maxPackageAssetBytes) {
            error = "Package embedded assets exceed the size limit";
            return false;
        }
        std::vector<uint8_t> decoded;
        if (!io::base64Decode(data, decoded) || decoded.empty()) {
            error = "Package contains invalid embedded asset data";
            return false;
        }
        metadata.push_back(std::move(entry));
    }
    out = std::move(metadata);
    return true;
}

bool readInstalledAssetMetadata(json::Reader assets, std::vector<AssetMetadata>& out) {
    if (!assets.valid()) return true;
    if (!assets.isArray() || assets.size() > maxAssetCount) return false;
    std::vector<AssetMetadata> metadata;
    metadata.reserve(assets.size());
    for (std::size_t index = 0; index < assets.size(); ++index) {
        const json::Reader asset = assets.element(index);
        AssetMetadata entry;
        if (!asset.isObject() || asset.size() != 2 ||
            !getStringMember(asset, "name", entry.name) || !utils::isSafeFileName(entry.name) ||
            !getStringMember(asset, "mediaType", entry.mediaType) || !isSupportedAssetMediaType(entry.mediaType)) {
            return false;
        }
        metadata.push_back(std::move(entry));
    }
    out = std::move(metadata);
    return true;
}

bool encodeAsset(const std::vector<uint8_t>& bytes, std::string& out) {
    out.resize(io::base64EncodedSize(bytes.size()));
    std::size_t encodedLength = 0;
    if (!io::base64Encode(bytes.data(), bytes.size(), out.data(), out.size(), encodedLength)) return false;
    out.resize(encodedLength);
    return true;
}

bool containsString(const std::vector<std::string>& values, const std::string& value) {
    return std::find(values.begin(), values.end(), value) != values.end();
}

std::string uniqueDisplayName(const std::string& name, const std::vector<std::string>& usedNames) {
    if (!containsString(usedNames, name)) {
        return name;
    }

    std::size_t suffix = 2;
    while (true) {
        const std::string candidate = name + " (" + std::to_string(suffix++) + ")";
        if (!containsString(usedNames, candidate)) {
            return candidate;
        }
    }
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
        LOG_WARN(TAG, "readJsonFile: missing %s", path.c_str());
        return Error("File not found");
    }

    LOG_DEBUG(TAG, "readJsonFile: reading %s", path.c_str());

    json::Document json;
    if (!mg::json::readJsonFile(fs, path, json)) {
        LOG_WARN(TAG, "readJsonFile: invalid JSON in %s", path.c_str());
        return Error("Invalid JSON");
    }

    LOG_DEBUG(TAG, "readJsonFile: parsed %s", path.c_str());
    Result result = OkObject();
    result.data = std::move(json);
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

std::string uniqueSlug(const std::string& name, std::vector<std::string>& usedIds) {
    const std::string base = slugify(name);
    std::string candidate = base;
    int suffix = 2;

    while (containsString(usedIds, candidate)) {
        candidate = base + "-" + std::to_string(suffix++);
    }

    usedIds.push_back(candidate);
    return candidate;
}

std::vector<std::string> readPackageDirectories(io::FileSystem& fs, const std::string& root) {
    std::vector<std::string> out;
    fs.listDirectories(root, out);
    return out;
}

} // namespace

PackageManager::PackageManager(io::FileSystem& fs, std::string dataRoot) : fs(fs), dataRoot(std::move(dataRoot)) {}

std::vector<PackageManager::PackageRecord> PackageManager::readInstalledPackages(io::FileSystem& fs, std::string_view dataRoot) {
    std::vector<PackageRecord> installedPackages;
    for (const auto& packageId : readPackageDirectories(fs, paths::packagesRootPath(dataRoot))) {
        if (!utils::isSafeId(packageId)) {
            LOG_WARN(TAG, "rebuildLibrary: skipping unsafe package id %s", packageId.c_str());
            continue;
        }

        json::Document manifest;
        if (!mg::json::readJsonFile(fs, paths::manifestPath(dataRoot, packageId), manifest) || !manifest.root().isObject()) {
            LOG_WARN(TAG, "rebuildLibrary: skipping invalid manifest for %s", packageId.c_str());
            continue;
        }

        PackageRecord record;
        record.summary.id = packageId;
        if (!getStringMember(manifest.root(), "name", record.name) ||
            !getStringMember(manifest.root(), "author", record.summary.author)) {
            LOG_WARN(TAG, "rebuildLibrary: skipping manifest missing package fields for %s", packageId.c_str());
            continue;
        }

        record.summary.name = record.name;

        const json::Reader facesValue = json::getArrayMember(manifest.root(), "faces");
        if (!facesValue.valid()) {
            LOG_WARN(TAG, "rebuildLibrary: skipping manifest missing faces for %s", packageId.c_str());
            continue;
        }

        record.faces.reserve(facesValue.size());
        for (std::size_t index = 0; index < facesValue.size(); ++index) {
            const json::Reader faceEntry = facesValue.element(index);
            if (!faceEntry.isObject()) {
                LOG_WARN(TAG, "rebuildLibrary: skipping non-object face entry for %s", packageId.c_str());
                continue;
            }

            FaceSummary face;
            if (!getStringMember(faceEntry, "id", face.id) ||
                !getStringMember(faceEntry, "name", face.name)) {
                LOG_WARN(TAG, "rebuildLibrary: skipping invalid face entry for %s", packageId.c_str());
                continue;
            }

            record.faces.push_back(std::move(face));
        }

        installedPackages.push_back(std::move(record));
    }

    sortCache(installedPackages);
    return installedPackages;
}

void PackageManager::sortCache(std::vector<PackageRecord>& cache) {
    std::sort(cache.begin(), cache.end(), [](const auto& a, const auto& b) {
        return a.summary.id < b.summary.id;
    });
}

void PackageManager::rebuildDisplayNames(std::vector<PackageRecord>& cache) {
    std::vector<std::string> usedNames;
    usedNames.reserve(cache.size());

    for (auto& package : cache) {
        package.summary.name = uniqueDisplayName(package.name, usedNames);
        usedNames.push_back(package.summary.name);
    }
}

bool PackageManager::refreshCacheFromDisk() const {
    LOG_INFO(TAG, "refreshCacheFromDisk: scanning installed manifests");
    cache = readInstalledPackages(fs, dataRoot);
    rebuildDisplayNames(cache);
    cacheReady = true;
    LOG_INFO(TAG, "refreshCacheFromDisk: cached %u packages", static_cast<unsigned>(cache.size()));
    return true;
}

bool PackageManager::writeLibraryIndexFromCache() const {
    if (!cacheReady) {
        LOG_WARN(TAG, "writeLibraryIndexFromCache: cache not ready");
        return false;
    }

    json::Document document = json::object();
    json::Writer writer = document.writer();
    if (!writer.writeObject([&](json::ObjectWriter& object) { return object.writeArray("packages", [&](json::ArrayWriter& packages) {
        for (const auto& entry : cache) {
            if (!writePackageEntry(packages, {entry.summary.id, entry.summary.name, entry.summary.author})) return false;
        }
        return true;
    }); })) return false;
    return fs.writeText(paths::libraryPath(dataRoot), document.toString());
}

bool PackageManager::listPackages(std::vector<PackageSummary>& out) const {
    if (!cacheReady) {
        LOG_WARN(TAG, "listPackages: cache not ready");
        return false;
    }

    out.clear();
    out.reserve(cache.size());
    for (const auto& entry : cache) {
        out.push_back(entry.summary);
    }

    LOG_INFO(TAG, "listPackages: packages=%u", static_cast<unsigned>(out.size()));
    return true;
}

Result PackageManager::getPackage(const std::string& packageId) const {
    if (!utils::isSafeId(packageId)) {
        return Error("Invalid package id");
    }

    LOG_INFO(TAG, "getPackage: packageId=%s", packageId.c_str());
    return readJsonFile(fs, paths::manifestPath(dataRoot, packageId));
}

bool PackageManager::listFaces(const std::string& packageId, std::vector<FaceSummary>& out) const {
    if (!utils::isSafeId(packageId)) {
        LOG_WARN(TAG, "listFaces: invalid package id %s", packageId.c_str());
        return false;
    }

    if (!cacheReady) {
        LOG_WARN(TAG, "listFaces: cache not ready for %s", packageId.c_str());
        return false;
    }

    const auto it = std::lower_bound(cache.begin(), cache.end(), packageId, [](const PackageRecord& record, const std::string& id) {
        return record.summary.id < id;
    });
    if (it == cache.end() || it->summary.id != packageId) {
        LOG_WARN(TAG, "listFaces: cache miss for %s", packageId.c_str());
        return false;
    }

    out.clear();
    out.reserve(it->faces.size());
    for (const auto& face : it->faces) {
        out.push_back(face);
    }

    LOG_INFO(TAG, "listFaces: packageId=%s faces=%u", packageId.c_str(), static_cast<unsigned>(it->faces.size()));
    return true;
}

Result PackageManager::getFace(const std::string& packageId, const std::string& faceId) const {
    if (!utils::isSafeId(packageId) || !utils::isSafeId(faceId)) {
        return Error("Invalid face id");
    }

    const std::string path = paths::facePath(dataRoot, packageId, faceId);
    LOG_INFO(TAG, "getFace: packageId=%s faceId=%s path=%s", packageId.c_str(), faceId.c_str(), path.c_str());
    return readJsonFile(fs, path);
}

bool validatePackageDocument(json::Reader input, std::vector<AssetMetadata>& assets, std::string& error) {
    if (!input.isObject()) {
        error = "Invalid package JSON";
        return false;
    }

    const char* requiredKeys[] = {"name", "author", "description", "faces"};
    for (const char* key : requiredKeys) {
        if (!input.member(key).valid()) {
            error = std::string("Package JSON is missing required field: ") + key;
            return false;
        }
    }

    if (input.size() != 4 && input.size() != 5) {
        error = "Package JSON contains unsupported fields";
        return false;
    }

    std::string_view name;
    std::string_view author;
    std::string_view description;
    if (!input.member("name").read(name) || name.empty()) {
        error = "Package name must be a non-empty string";
        return false;
    }

    if (!input.member("author").read(author) || author.empty()) {
        error = "Package author must be a non-empty string";
        return false;
    }

    if (!input.member("description").read(description)) {
        error = "Package description must be a string";
        return false;
    }

    if (!readAssetMetadata(input.member("assets"), assets, error)) {
        return false;
    }

    const json::Reader faces = input.member("faces");
    if (!faces.isArray() || faces.size() == 0) {
        error = "Package must contain at least one face";
        return false;
    }

    for (std::size_t index = 0; index < faces.size(); ++index) {
        const json::Reader faceEntry = faces.element(index);
        if (!faceEntry.isObject()) {
            error = "Face entries must be objects";
            return false;
        }

        if (!faceEntry.member("name").valid() || !faceEntry.member("face").valid()) {
            error = "Face entries must contain name and face";
            return false;
        }

        if (faceEntry.size() != 2) {
            error = "Face entries contain unsupported fields";
            return false;
        }

        std::string_view faceName;
        if (!faceEntry.member("name").read(faceName) || faceName.empty()) {
            error = "Face name must be a non-empty string";
            return false;
        }

        if (!faceEntry.member("face").isObject()) {
            error = "Face payload must be an object";
            return false;
        }
    }

    return true;
}

Result PackageManager::importPackage(const std::string& json) {
    LOG_INFO(TAG, "importPackage: input bytes=%u", static_cast<unsigned>(json.size()));

    json::Document input = json::parse(json);
    if (!input.valid() || !input.root().isObject()) {
        LOG_ERROR(TAG, "importPackage: invalid package JSON");
        return Error("Invalid package JSON");
    }

    return importPackage(input.root());
}

Result PackageManager::importPackage(json::Reader input) {
    std::string validationError;
    std::vector<AssetMetadata> assetMetadata;
    if (!validatePackageDocument(input, assetMetadata, validationError)) {
        LOG_ERROR(TAG, "importPackage: %s", validationError.c_str());
        return Error(validationError);
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

    if (!cacheReady) {
        (void)refreshCacheFromDisk();
    }

    std::vector<std::string> usedPackageIds;
    usedPackageIds.reserve(cache.size());
    for (const auto& package : cache) {
        usedPackageIds.push_back(package.summary.id);
    }

    const std::string packageId = uniqueSlug(packageName, usedPackageIds);
    LOG_INFO(TAG,
             "importPackage: packageId=%s name=%s faces=%u",
             packageId.c_str(),
             packageName.c_str(),
             static_cast<unsigned>(input.member("faces").size()));

    const std::string packageRoot = paths::packagePath(dataRoot, packageId);
    const json::Reader inputAssets = input.member("assets");
    for (std::size_t index = 0; index < assetMetadata.size(); ++index) {
        std::string_view encoded;
        if (!inputAssets.element(index).member("data").read(encoded)) {
            removeTree(fs, packageRoot);
            return Error("Invalid embedded asset");
        }

        std::vector<uint8_t> decoded;
        if (!io::base64Decode(encoded, decoded)) {
            removeTree(fs, packageRoot);
            return Error("Invalid embedded asset data");
        }

        const std::string path = paths::assetPath(dataRoot, packageId, paths::imagesDir, assetMetadata[index].name);
        if (!fs.writeBytes(path, decoded.data(), decoded.size())) {
            removeTree(fs, packageRoot);
            return Error("Failed to write embedded asset");
        }
    }

    std::vector<std::string> usedFaceIds;
    std::vector<FaceSummary> faceSummaries;
    const json::Reader inputFaces = input.member("faces");
    faceSummaries.reserve(inputFaces.size());
    json::Document manifest = json::object();
    json::Writer manifestWriter = manifest.writer();
    std::vector<std::pair<std::string, std::string>> manifestFaces;
    unsigned faceIndex = 0;
    for (std::size_t inputIndex = 0; inputIndex < inputFaces.size(); ++inputIndex) {
        const json::Reader faceValue = inputFaces.element(inputIndex);
        ++faceIndex;
        std::string faceName;
        (void)getStringMember(faceValue, "name", faceName);
        const json::Reader faceDocValue = json::getObjectMember(faceValue, "face");

        const std::string faceId = uniqueSlug(faceName, usedFaceIds);
        const std::string faceStoragePath = paths::facePath(dataRoot, packageId, faceId);
        json::Document faceDocument = json::object();
        json::Writer faceWriter = faceDocument.writer();
        if (!faceWriter.write(faceDocValue)) return Error("Invalid face payload");
        const std::string faceJson = faceDocument.toString();
        faceSummaries.push_back(FaceSummary{faceId, faceName});
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

        manifestFaces.emplace_back(faceId, faceName);
    }

    if (!manifestWriter.writeObject([&](json::ObjectWriter& object) {
            return object.write("name", packageName) && object.write("author", packageAuthor) &&
                   object.write("description", packageDescription) &&
                   object.writeArray("assets", [&](json::ArrayWriter& assets) {
                       for (const auto& asset : assetMetadata) {
                           if (!assets.writeObject([&](json::ObjectWriter& entry) {
                                   return entry.write("name", asset.name) &&
                                          entry.write("mediaType", asset.mediaType);
                               }))
                               return false;
                       }
                       return true;
                   }) &&
                   object.writeArray("faces", [&](json::ArrayWriter& faces) {
                       for (const auto& face : manifestFaces) {
                           if (!faces.writeObject([&](json::ObjectWriter& entry) {
                                   return entry.write("id", face.first) && entry.write("name", face.second);
                               }))
                               return false;
                       }
                       return true;
                   });
        }))
        return Error("Failed to create package manifest");
    const std::string manifestJson = manifest.toString();
    LOG_INFO(TAG, "importPackage: writing manifest %s (%u bytes)", paths::manifestPath(dataRoot, packageId).c_str(), static_cast<unsigned>(manifestJson.size()));
    if (!fs.writeText(paths::manifestPath(dataRoot, packageId), manifestJson)) {
        LOG_ERROR(TAG, "importPackage: failed to write manifest");
        removeTree(fs, packageRoot);
        return Error("Failed to write package manifest");
    }

    PackageRecord newRecord;
    newRecord.name = packageName;
    newRecord.summary.id = packageId;
    newRecord.summary.name = packageName;
    newRecord.summary.author = packageAuthor;
    newRecord.faces = std::move(faceSummaries);

    auto insertIt = std::lower_bound(cache.begin(), cache.end(), packageId, [](const PackageRecord& record, const std::string& id) {
        return record.summary.id < id;
    });
    insertIt = cache.insert(insertIt, std::move(newRecord));
    rebuildDisplayNames(cache);

    LOG_INFO(TAG, "importPackage: writing library index");
    if (!writeLibraryIndexFromCache()) {
        LOG_ERROR(TAG, "importPackage: failed to write library index");
        cache.erase(insertIt);
        rebuildDisplayNames(cache);
        removeTree(fs, packageRoot);
        return Error("Failed to update library index");
    }

    LOG_INFO(TAG, "importPackage: done");
    return OkObject();
}

Result PackageManager::exportPackage(const std::string& packageId) const {
    if (!utils::isSafeId(packageId)) {
        return Error("Invalid package id");
    }

    LOG_INFO(TAG, "exportPackage: packageId=%s", packageId.c_str());
    json::Document manifest;
    if (!mg::json::readJsonFile(fs, paths::manifestPath(dataRoot, packageId), manifest) || !manifest.root().isObject()) {
        LOG_WARN(TAG, "exportPackage: manifest load failed for %s", packageId.c_str());
        return Error("Package not found");
    }

    const json::Reader facesValue = json::getArrayMember(manifest.root(), "faces");
    if (!facesValue.valid()) {
        return Error("Package manifest is invalid");
    }

    std::vector<AssetMetadata> assets;
    if (!readInstalledAssetMetadata(manifest.root().member("assets"), assets)) {
        return Error("Package manifest has invalid assets");
    }

    Result result = OkObject();
    std::string name, author, description;
    if (!getStringMember(manifest.root(), "name", name) || !getStringMember(manifest.root(), "author", author) || !getStringMember(manifest.root(), "description", description)) return Error("Package manifest is invalid");
    json::Writer writer = result.data.writer();
    if (!writer.writeObject([&](json::ObjectWriter& object) {
        return object.write("name", name) && object.write("author", author) &&
               object.write("description", description) &&
               object.writeArray("assets", [&](json::ArrayWriter& outputAssets) {
                   for (const auto& asset : assets) {
                       std::vector<uint8_t> bytes;
                       const std::string path = paths::assetPath(dataRoot, packageId, paths::imagesDir, asset.name);
                       if (!fs.readBytes(path, bytes) || bytes.empty()) return false;
                       std::string encoded;
                       if (!encodeAsset(bytes, encoded)) return false;
                       if (!outputAssets.writeObject([&](json::ObjectWriter& output) {
                               return output.write("name", asset.name) &&
                                      output.write("mediaType", asset.mediaType) && output.write("data", encoded);
                           }))
                           return false;
                   }
                   return true;
               }) &&
               object.writeArray("faces", [&](json::ArrayWriter& outputFaces) {
    for (std::size_t index = 0; index < facesValue.size(); ++index) {
        const json::Reader faceEntry = facesValue.element(index);
        if (!faceEntry.isObject()) return false;

        std::string faceId;
        std::string faceName;
        if (!getStringMember(faceEntry, "id", faceId) ||
            !getStringMember(faceEntry, "name", faceName)) {
            return false;
        }

        json::Document faceDoc;
        const std::string facePath = paths::facePath(dataRoot, packageId, faceId);
        LOG_DEBUG(TAG, "exportPackage: reading face %s", facePath.c_str());
        if (!mg::json::readJsonFile(fs, facePath, faceDoc) || !faceDoc.root().isObject()) {
            LOG_WARN(TAG, "exportPackage: face load failed for %s/%s", packageId.c_str(), faceId.c_str());
            return false;
        }

        if (!outputFaces.writeObject([&](json::ObjectWriter& faceOut) { return faceOut.write("name", faceName) && faceOut.write("face", faceDoc.root()); })) return false;
    }
                   return true;
               });
    })) return Error("Failed to export package");
    return result;
}

Result PackageManager::removePackage(const std::string& packageId) {
    if (!utils::isSafeId(packageId)) {
        return Error("Invalid package id");
    }

    LOG_INFO(TAG, "removePackage: packageId=%s", packageId.c_str());
    const std::string packageRoot = paths::packagePath(dataRoot, packageId);
    if (!fs.exists(packageRoot)) {
        return Error("Package not found");
    }

    if (!cacheReady) {
        (void)refreshCacheFromDisk();
    }

    if (!removeTree(fs, packageRoot)) {
        return Error("Failed to remove package");
    }

    const auto it = std::lower_bound(cache.begin(), cache.end(), packageId, [](const PackageRecord& record, const std::string& id) {
        return record.summary.id < id;
    });
    if (it != cache.end() && it->summary.id == packageId) {
        cache.erase(it);
        rebuildDisplayNames(cache);
    }

    if (!writeLibraryIndexFromCache()) {
        return Error("Failed to update library index");
    }
    return OkObject();
}

void PackageManager::rebuildLibrary() const {
    LOG_INFO(TAG, "rebuildLibrary: full rescan");
    (void)refreshCacheFromDisk();
    (void)writeLibraryIndexFromCache();
}

} // namespace mg
