#pragma once

#include <string_view>
#include <string>
#include <vector>

#include <multigauge/json/Json.h>

#include <multigauge/Result.h>

namespace mg::io {
class FileSystem;
}

namespace mg {

struct FaceSummary {
    std::string id;
    std::string name;
};

struct PackageSummary {
    std::string id;
    std::string name;
    std::string author;
};

} // namespace mg

namespace mg::package {

/// Manages installed package storage under the configured data root.
class Manager {
public:
    /// Creates a package manager for `dataRoot`.
    /// @param fs File system used for package storage.
    /// @param dataRoot Base directory for package data.
    Manager(io::FileSystem& fs, std::string dataRoot);

    //----------[ QUERY ]----------//

    /// Copies cached package summaries into `out`.
    /// @param out Receives the cached package summaries. The vector is cleared first.
    /// @return True when the cache is ready and data was copied.
    bool listPackages(std::vector<PackageSummary>& out) const;

    /// Returns the stored manifest JSON for `packageId`.
    /// @param packageId Installed package id.
    /// @return Parsed `manifest.json` document, or an error result.
    Result getPackage(const std::string& packageId) const;

    /// Copies cached face summaries for `packageId` into `out`.
    /// @param packageId Installed package id.
    /// @param out Receives the cached face summaries. The vector is cleared first.
    /// @return True when the cache is ready and data was copied.
    bool listFaces(const std::string& packageId, std::vector<FaceSummary>& out) const;

    /// Returns the stored face JSON for `faceId` within `packageId`.
    /// @param packageId Installed package id.
    /// @param faceId Installed face id.
    /// @return Parsed `faces/<faceId>.json` document, or an error result.
    Result getFace(const std::string& packageId, const std::string& faceId) const;

    //----------[ MUTATION ]----------//

    /// Imports a package JSON payload into storage.
    /// @see docs/schemas/package.schema.json for the accepted document shape.
    /// @param json Package JSON to import.
    /// @return Imported package document, or an error result.
    Result importPackage(const std::string& json);

    /// Imports a package JSON payload into storage.
    /// @see docs/schemas/package.schema.json for the accepted document shape.
    /// @param package Package JSON to import.
    /// @return Imported package document, or an error result.
    Result importPackage(json::Reader package);

    /// Exports a package JSON payload from storage.
    /// @param packageId Installed package id.
    /// @return Exported package document, or an error result.
    Result exportPackage(const std::string& packageId) const;

    /// Removes the stored package for `packageId`.
    /// @param packageId Installed package id.
    /// @return Empty object on success, or an error result.
    Result removePackage(const std::string& packageId);

    //----------[ MAINTENANCE ]----------//
    /// Rebuilds `library.json` from installed packages.
    void rebuildLibrary() const;

private:
    struct PackageRecord {
        std::string name;
        PackageSummary summary;
        std::vector<FaceSummary> faces;
    };

    static void sortCache(std::vector<PackageRecord>& cache);
    static void rebuildDisplayNames(std::vector<PackageRecord>& cache);
    static std::vector<PackageRecord> readInstalledPackages(io::FileSystem& fs, std::string_view dataRoot);

    bool refreshCacheFromDisk() const;
    bool writeLibraryIndexFromCache() const;

    io::FileSystem& fs;
    std::string dataRoot;
    mutable bool cacheReady = false;
    mutable std::vector<PackageRecord> cache;
};

} // namespace mg::package
