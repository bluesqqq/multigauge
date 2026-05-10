#include "runtime/PackageManager.h"

#include "AppPaths.h"

#include <multigauge/io/FileSystem.h>

#include <rapidjson/document.h>

namespace mg {

namespace {
Result jsonResult(const std::string& json) {
    Result result = OkObject();
    auto& allocator = result.data.GetAllocator();
    rapidjson::Value jsonValue;
    jsonValue.SetString(json.c_str(), allocator);
    result.data.AddMember("json", std::move(jsonValue), allocator);
    return result;
}

Result notImplemented() {
    return Error("Not implemented yet");
}

Result readTextFile(io::FileSystem& fs, const std::string& path) {
    std::string json;
    if (!fs.readText(path, json)) {
        return Error("File not found");
    }
    return jsonResult(json);
}
}

PackageManager::PackageManager(io::FileSystem& fs, std::string dataRoot)
    : fs(fs), dataRoot(std::move(dataRoot)) {
}

Result PackageManager::listPackages() const {
    return readTextFile(fs, paths::libraryPath(dataRoot));
}

Result PackageManager::getPackage(const std::string& packageId) const {
    return readTextFile(fs, paths::manifestPath(dataRoot, packageId));
}

Result PackageManager::getFace(const std::string& packageId, const std::string& faceId) const {
    return readTextFile(fs, paths::facePath(dataRoot, packageId, faceId));
}

Result PackageManager::importPackage(const std::string&) {
    return notImplemented();
}

Result PackageManager::exportPackage(const std::string&) const {
    return notImplemented();
}

Result PackageManager::removePackage(const std::string&) {
    return notImplemented();
}

} // namespace mg
