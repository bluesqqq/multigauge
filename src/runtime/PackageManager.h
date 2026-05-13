#pragma once

#include <string_view>
#include <string>
#include <vector>

#include <multigauge/editor/Result.h>

namespace mg::io {
class FileSystem;
}

namespace mg {

/// Manages package storage under the configured data root.
	class PackageManager {
	    public:
	        PackageManager(io::FileSystem& fs, std::string dataRoot);

	        //----------[ QUERY ]----------//
            
	        /// Returns the library index JSON.
	        /// @return Parsed `library.json` document.
	        Result listPackages() const;

	        /// Returns the package manifest JSON for `packageId`.
	        /// @param packageId Installed package id.
	        /// @return Parsed `manifest.json` document.
	        Result getPackage(const std::string& packageId) const;

	        /// Returns the face JSON for `faceId` within `packageId`.
	        /// @param packageId Installed package id.
	        /// @param faceId Installed face id.
	        /// @return Parsed `faces/<faceId>.json` document.
	        Result getFace(const std::string& packageId, const std::string& faceId) const;

	        //----------[ MUTATION ]----------//

	        /// Imports a package JSON payload into storage.
	        /// @param json Package JSON to import.
	        /// @return Parsed normalized manifest document.
	        Result importPackage(const std::string& json);
	            
	        /// Exports a package JSON payload from storage.
	        /// @param packageId Installed package id.
	        /// @return Parsed exported package document.
	        Result exportPackage(const std::string& packageId) const;

	        /// Removes the stored package for `packageId`.
	        /// @param packageId Installed package id.
	        Result removePackage(const std::string& packageId);

	        //----------[ MAINTENANCE ]----------//
	        /// Rebuilds `library.json` from installed manifests.
	        void rebuildLibrary() const;

    private:
        io::FileSystem& fs;
        std::string dataRoot;
};

} // namespace mg
