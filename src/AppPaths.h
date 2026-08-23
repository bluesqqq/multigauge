#pragma once

#include <string>
#include <string_view>

namespace mg::paths {

inline constexpr std::string_view libraryIndex = "library.json";
inline constexpr std::string_view state = "state.json";
inline constexpr std::string_view packagesDir = "packages";
inline constexpr std::string_view facesDir = "faces";
inline constexpr std::string_view assetsDir = "assets";
inline constexpr std::string_view imagesDir = "images";

inline std::string joinPath(std::string_view left, std::string_view right) {
    if (left.empty()) return std::string(right);
    if (right.empty()) return std::string(left);

    std::string out(left);
    if (out.back() != '/' && out.back() != '\\') {
        out.push_back('/');
    }

    if (!right.empty() && (right.front() == '/' || right.front() == '\\')) {
        right.remove_prefix(1);
    }

    out.append(right.data(), right.size());
    return out;
}

inline std::string appPath(std::string_view root, std::string_view relativePath) {
    return joinPath(root, relativePath);
}

inline std::string libraryPath(std::string_view root) {
    return appPath(root, libraryIndex);
}

inline std::string statePath(std::string_view root) {
    return appPath(root, state);
}

inline std::string packagesRootPath(std::string_view root) {
    return appPath(root, packagesDir);
}

inline std::string packagePath(std::string_view root, std::string_view packageId) {
    return joinPath(packagesRootPath(root), packageId);
}

inline std::string manifestPath(std::string_view root, std::string_view packageId) {
    return joinPath(packagePath(root, packageId), "manifest.json");
}

inline std::string facesRootPath(std::string_view root, std::string_view packageId) {
    return joinPath(packagePath(root, packageId), facesDir);
}

inline std::string facePath(std::string_view root, std::string_view packageId, std::string_view faceId) {
    return joinPath(facesRootPath(root, packageId), std::string(faceId) + ".json");
}

inline std::string assetsPath(std::string_view root, std::string_view packageId) {
    return joinPath(packagePath(root, packageId), assetsDir);
}

/// Returns the directory for one asset type. An empty package ID denotes raw assets.
inline std::string assetDirectory(
    std::string_view root,
    std::string_view packageId,
    std::string_view assetType
) {
    return packageId.empty() ? joinPath("/assets", assetType) : joinPath(assetsPath(root, packageId), assetType);
}

/// Returns the path for one named asset within its typed asset directory.
inline std::string assetPath(
    std::string_view root,
    std::string_view packageId,
    std::string_view assetType,
    std::string_view name
) {
    return joinPath(assetDirectory(root, packageId, assetType), name);
}

} // namespace mg::paths
