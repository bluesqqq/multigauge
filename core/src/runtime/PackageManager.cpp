#include <multigauge/runtime/PackageManager.h>

#include "../AppPaths.h"

#include <multigauge/io/FileSystem.h>
#include <multigauge/io/Log.h>
#include <multigauge/utils/Json.h>
#include <multigauge/utils/Text.h>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <algorithm>
#include <cctype>
#include <string_view>
#include <utility>
#include <vector>

namespace mg {

namespace {

constexpr const char* TAG = "PackageManager";

using mg::json::getArrayMember;
using mg::json::getObjectMember;
using mg::json::getStringMember;

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

struct LibraryIndexPackage {
    std::string id;
    std::string name;
    std::string author;
};

bool writePackageEntry(rapidjson::Writer<rapidjson::StringBuffer>& writer, const LibraryIndexPackage& package) {
    writer.StartObject();
    writer.Key("id");
    writer.String(package.id.c_str());
    writer.Key("name");
    writer.String(package.name.c_str());
    writer.Key("author");
    writer.String(package.author.c_str());

    writer.EndObject();
    return true;
}

bool isSafeId(std::string_view value) {
    if (value.empty()) return false;

    for (unsigned char c : value) {
        if (!(std::isalnum(c) || c == '-')) {
            return false;
        }
    }

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

    rapidjson::Document json;
    if (!mg::json::readJsonFile(fs, path, json)) {
        LOG_WARN(TAG, "readJsonFile: invalid JSON in %s", path.c_str());
        return Error("Invalid JSON");
    }

    LOG_DEBUG(TAG, "readJsonFile: parsed %s", path.c_str());
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
        if (!isSafeId(packageId)) {
            LOG_WARN(TAG, "rebuildLibrary: skipping unsafe package id %s", packageId.c_str());
            continue;
        }

        rapidjson::Document manifest;
        if (!mg::json::readJsonFile(fs, paths::manifestPath(dataRoot, packageId), manifest) || !manifest.IsObject()) {
            LOG_WARN(TAG, "rebuildLibrary: skipping invalid manifest for %s", packageId.c_str());
            continue;
        }

        PackageRecord record;
        record.summary.id = packageId;
        if (!getStringMember(manifest, "name", record.name) ||
            !getStringMember(manifest, "author", record.summary.author)) {
            LOG_WARN(TAG, "rebuildLibrary: skipping manifest missing package fields for %s", packageId.c_str());
            continue;
        }

        record.summary.name = record.name;

        const rapidjson::Value* facesValue = getArrayMember(manifest, "faces");
        if (!facesValue) {
            LOG_WARN(TAG, "rebuildLibrary: skipping manifest missing faces for %s", packageId.c_str());
            continue;
        }

        record.faces.reserve(facesValue->Size());
        for (const auto& faceEntry : facesValue->GetArray()) {
            if (!faceEntry.IsObject()) {
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

    if (!makeDirectoryChain(fs, std::string(dataRoot)) ||
        !makeDirectoryChain(fs, paths::packagesRootPath(dataRoot))) {
        return false;
    }

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    writer.StartObject();
    writer.Key("packages");
    writer.StartArray();
    for (const auto& entry : cache) {
        LibraryIndexPackage packageEntry;
        packageEntry.id = entry.summary.id;
        packageEntry.name = entry.summary.name;
        packageEntry.author = entry.summary.author;
        writePackageEntry(writer, packageEntry);
    }
    writer.EndArray();
    writer.EndObject();

    return fs.writeText(paths::libraryPath(dataRoot), buffer.GetString());
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
    if (!isSafeId(packageId)) {
        return Error("Invalid package id");
    }

    LOG_INFO(TAG, "getPackage: packageId=%s", packageId.c_str());
    return readJsonFile(fs, paths::manifestPath(dataRoot, packageId));
}

bool PackageManager::listFaces(const std::string& packageId, std::vector<FaceSummary>& out) const {
    if (!isSafeId(packageId)) {
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
    if (!isSafeId(packageId) || !isSafeId(faceId)) {
        return Error("Invalid face id");
    }

    const std::string path = paths::facePath(dataRoot, packageId, faceId);
    LOG_INFO(TAG, "getFace: packageId=%s faceId=%s path=%s", packageId.c_str(), faceId.c_str(), path.c_str());
    return readJsonFile(fs, path);
}

bool validatePackageDocument(const rapidjson::Value& input, std::string& error) {
    if (!input.IsObject()) {
        error = "Invalid package JSON";
        return false;
    }

    const char* requiredKeys[] = {"name", "author", "description", "faces"};
    for (const char* key : requiredKeys) {
        if (!input.HasMember(key)) {
            error = std::string("Package JSON is missing required field: ") + key;
            return false;
        }
    }

    if (input.MemberCount() != 4) {
        error = "Package JSON contains unsupported fields";
        return false;
    }

    if (!input["name"].IsString() || input["name"].GetStringLength() == 0) {
        error = "Package name must be a non-empty string";
        return false;
    }

    if (!input["author"].IsString() || input["author"].GetStringLength() == 0) {
        error = "Package author must be a non-empty string";
        return false;
    }

    if (!input["description"].IsString()) {
        error = "Package description must be a string";
        return false;
    }

    if (!input["faces"].IsArray() || input["faces"].Empty()) {
        error = "Package must contain at least one face";
        return false;
    }

    for (const auto& faceEntry : input["faces"].GetArray()) {
        if (!faceEntry.IsObject()) {
            error = "Face entries must be objects";
            return false;
        }

        if (!faceEntry.HasMember("name") || !faceEntry.HasMember("face")) {
            error = "Face entries must contain name and face";
            return false;
        }

        if (faceEntry.MemberCount() != 2) {
            error = "Face entries contain unsupported fields";
            return false;
        }

        if (!faceEntry["name"].IsString() || faceEntry["name"].GetStringLength() == 0) {
            error = "Face name must be a non-empty string";
            return false;
        }

        if (!faceEntry["face"].IsObject()) {
            error = "Face payload must be an object";
            return false;
        }
    }

    return true;
}

Result PackageManager::importPackage(const std::string& json) {
    LOG_INFO(TAG, "importPackage: input bytes=%u", static_cast<unsigned>(json.size()));

    rapidjson::Document input;
    input.Parse(json.c_str());
    if (input.HasParseError() || !input.IsObject()) {
        LOG_ERROR(TAG, "importPackage: invalid package JSON");
        return Error("Invalid package JSON");
    }

    return importPackage(input);
}

Result PackageManager::importPackage(const rapidjson::Value& input) {
    std::string validationError;
    if (!validatePackageDocument(input, validationError)) {
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
             static_cast<unsigned>(input["faces"].Size()));

    const std::string packageRoot = paths::packagePath(dataRoot, packageId);
    if (!makeDirectoryChain(fs, dataRoot) ||
        !makeDirectoryChain(fs, paths::packagesRootPath(dataRoot)) ||
        !makeDirectoryChain(fs, packageRoot) ||
        !makeDirectoryChain(fs, paths::facesRootPath(dataRoot, packageId))) {
        LOG_ERROR(TAG, "importPackage: failed to create package directories for %s", packageId.c_str());
        return Error("Failed to create package directories");
    }

    std::vector<std::string> usedFaceIds;
    std::vector<FaceSummary> faceSummaries;
    faceSummaries.reserve(input["faces"].Size());
    rapidjson::StringBuffer manifestBuffer;
    rapidjson::Writer<rapidjson::StringBuffer> manifestWriter(manifestBuffer);
    manifestWriter.StartObject();
    manifestWriter.Key("name");
    manifestWriter.String(packageName.c_str());
    manifestWriter.Key("author");
    manifestWriter.String(packageAuthor.c_str());
    manifestWriter.Key("description");
    manifestWriter.String(packageDescription.c_str());
    manifestWriter.Key("faces");
    manifestWriter.StartArray();
    unsigned faceIndex = 0;
    for (const auto& faceValue : input["faces"].GetArray()) {
        ++faceIndex;
        std::string faceName;
        (void)getStringMember(faceValue, "name", faceName);
        const rapidjson::Value* faceDocValue = getObjectMember(faceValue, "face");

        const std::string faceId = uniqueSlug(faceName, usedFaceIds);
        const std::string faceStoragePath = paths::facePath(dataRoot, packageId, faceId);
        const std::string faceJson = mg::json::toString(*faceDocValue);
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

        manifestWriter.StartObject();
        manifestWriter.Key("id");
        manifestWriter.String(faceId.c_str());
        manifestWriter.Key("name");
        manifestWriter.String(faceName.c_str());
        manifestWriter.EndObject();
    }

    manifestWriter.EndArray();
    manifestWriter.EndObject();

    const std::string manifestJson = manifestBuffer.GetString();
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
    if (!isSafeId(packageId)) {
        return Error("Invalid package id");
    }

    LOG_INFO(TAG, "exportPackage: packageId=%s", packageId.c_str());
    rapidjson::Document manifest;
    if (!mg::json::readJsonFile(fs, paths::manifestPath(dataRoot, packageId), manifest) || !manifest.IsObject()) {
        LOG_WARN(TAG, "exportPackage: manifest load failed for %s", packageId.c_str());
        return Error("Package not found");
    }

    const rapidjson::Value* facesValue = getArrayMember(manifest, "faces");
    if (!facesValue) {
        return Error("Package manifest is invalid");
    }

    Result result;
    result.ok = true;
    result.data.SetObject();
    auto& allocator = result.data.GetAllocator();

    const char* keys[] = {"name", "author", "description"};
    for (const char* key : keys) {
        if (!manifest.HasMember(key) || !manifest[key].IsString()) {
            return Error("Package manifest is invalid");
        }

        rapidjson::Value keyValue;
        keyValue.SetString(manifest[key].GetString(), manifest[key].GetStringLength(), allocator);
        result.data.AddMember(rapidjson::StringRef(key), std::move(keyValue), allocator);
    }

    rapidjson::Value facesValueOut(rapidjson::kArrayType);
    facesValueOut.Reserve(static_cast<rapidjson::SizeType>(facesValue->Size()), allocator);
    for (const auto& faceEntry : facesValue->GetArray()) {
        if (!faceEntry.IsObject()) {
            return Error("Package manifest is invalid");
        }

        std::string faceId;
        std::string faceName;
        if (!getStringMember(faceEntry, "id", faceId) ||
            !getStringMember(faceEntry, "name", faceName)) {
            return Error("Package manifest is invalid");
        }

        rapidjson::Document faceDoc;
        const std::string facePath = paths::facePath(dataRoot, packageId, faceId);
        LOG_DEBUG(TAG, "exportPackage: reading face %s", facePath.c_str());
        if (!mg::json::readJsonFile(fs, facePath, faceDoc) || !faceDoc.IsObject()) {
            LOG_WARN(TAG, "exportPackage: face load failed for %s/%s", packageId.c_str(), faceId.c_str());
            return Error("Face file not found");
        }

        rapidjson::Value faceOut(rapidjson::kObjectType);

        rapidjson::Value faceNameValue;
        faceNameValue.SetString(faceName.c_str(), static_cast<rapidjson::SizeType>(faceName.size()), allocator);
        faceOut.AddMember("name", std::move(faceNameValue), allocator);

        rapidjson::Value faceDocValue;
        faceDocValue.CopyFrom(faceDoc, allocator);
        faceOut.AddMember("face", std::move(faceDocValue), allocator);

        facesValueOut.PushBack(std::move(faceOut), allocator);
    }
    result.data.AddMember("faces", std::move(facesValueOut), allocator);
    return result;
}

Result PackageManager::removePackage(const std::string& packageId) {
    if (!isSafeId(packageId)) {
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
