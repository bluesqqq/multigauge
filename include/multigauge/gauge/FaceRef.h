#pragma once

#include <string>

namespace mg::gauge {

/// @brief Identifies one installed face within a package.
/// @details Face IDs are scoped to a package, so both IDs are required to
/// unambiguously select an installed face.
struct FaceRef {
    std::string packageId;
    std::string faceId;

    [[nodiscard]] bool valid() const noexcept {
        return !packageId.empty() && !faceId.empty();
    }
};

} // namespace mg::gauge
