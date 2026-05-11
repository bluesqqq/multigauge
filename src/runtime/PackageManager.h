#pragma once

#include <string>

#include <multigauge/editor/Result.h>

namespace mg::io {
class FileSystem;
}

namespace mg {

class PackageManager {
public:
    PackageManager(io::FileSystem& fs, std::string dataRoot);

    Result listPackages() const;
    Result getPackage(const std::string& packageId) const;
    Result getFace(const std::string& packageId, const std::string& faceId) const;
    Result importPackage(const std::string& json);
    Result exportPackage(const std::string& packageId) const;
    Result removePackage(const std::string& packageId);

    void rebuildLibrary() const;

private:
    io::FileSystem& fs;
    std::string dataRoot;
};

} // namespace mg
