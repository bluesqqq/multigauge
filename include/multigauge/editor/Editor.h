#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <multigauge/editor/History.h>
#include <multigauge/Result.h>
#include <multigauge/editor/Types.h>
#include <multigauge/gauge/GaugeFace.h>

namespace mg::editor {

using gauge::Element;
using gauge::GaugeFace;
using gauge::NodeHandle;

class EditorPreview;

/// @brief Editor model for gauge faces.

class Editor {
public:
    /// @brief Stable ID for an editor-owned face.
    using FaceId = NodeId;

    /// @brief Sentinel index that appends to a sibling list.
    static constexpr std::size_t Append = static_cast<std::size_t>(-1);

    struct PackageInfo {
        std::string name;
        std::string author;
        std::string description;
    };

    /// @brief An asset embedded in an editor package document.
    /// @details `data` contains standard Base64 without a data-URL prefix. The editor
    /// owns all fields; callers must not retain references across edits or history changes.
    struct Asset {
        std::string name;
        std::string mediaType;
        std::string data;

        /// @brief Returns whether this asset has a supported image type and valid embedded data.
        /// @details Valid assets have a safe logical file name, a supported image media type,
        /// and non-empty standard Base64 data within the editor's size limit.
        bool valid() const;
    };

    struct FaceMeta {
        std::string name;
    };

    /// @brief Constructs an empty editor.
    Editor() = default;

    //----------[ LIFETIME ]----------//

    /// @brief Clears faces, package metadata, and history.
    void clear();

    //----------[ PACKAGE ]----------//

    /// @brief Updates package metadata.
    bool setPackageInfo(const PackageInfo& info);

    /// @brief Returns package metadata.
    const PackageInfo& packageInfo() const { return package_; }

    /// @brief Adds or replaces one embedded asset without replacing the asset collection.
    bool setAsset(const Asset& asset);

    /// @brief Removes one embedded asset by its logical name.
    /// @return False when the asset is unknown or still referenced by an image element.
    bool removeAsset(const std::string& name);

    /// @brief Returns the package's embedded assets.
    const std::vector<Asset>& assets() const { return assets_; }

    //----------[ FACES ]----------//

    /// @brief Sets a face display name.
    bool setFaceName(FaceId faceId, const std::string& name);

    /// @brief Returns a face display name.
    std::string getFaceName(FaceId faceId) const;

    /// @brief Returns the number of faces.
    std::size_t faceCount() const { return faces_.size(); }

    /// @brief Returns the ID of the face at an index.
    FaceId faceIdAt(std::size_t index) const { return faces_.at(index).id; }

    /// @brief Returns a borrowed face by ID.
    /// @details The pointer becomes invalid when the face is removed, the package is loaded or cleared,
    /// or an undo/redo operation restores a package snapshot.
    GaugeFace* getFace(FaceId id) noexcept { return face(id); }

    /// @brief Returns a borrowed read-only face by ID.
    /// @details The pointer becomes invalid when the face is removed, the package is loaded or cleared,
    /// or an undo/redo operation restores a package snapshot.
    const GaugeFace* getFace(FaceId id) const noexcept { return face(id); }

    //----------[ SERIALIZATION ]----------//

    /// @brief Loads a package document containing gauge faces.
    bool loadPackage(const std::string& json);

    /// @brief Exports the current package document.
    std::string exportPackage() const;

    //----------[ INSPECTION ]----------//

    /// @brief Serializes hierarchy data using face IDs and NodeHandle tokens.
    Result getHierarchy() const;

    /// @brief Lists registered gauge element types.
    Result listElementTypes() const;

    //----------[ FACE EDITING ]----------//

    /// @brief Creates a face from serialized face JSON.
    Result createFace(const std::string& json, FacePlacement where = FacePlacement{});

    /// @brief Removes one face.
    Result removeFace(FaceId faceId);

    /// @brief Reorders one face.
    Result reorderFace(FaceId faceId, std::size_t index);

    /// @brief Serializes one face.
    Result serializeFace(FaceId faceId) const;

    //----------[ ELEMENT EDITING ]----------//

    /// @brief Creates an element from flat element JSON.
    Result createElement(
        const ElementPlacement& where,
        const std::string& json
    );

    /// @brief Removes an element subtree.
    Result removeElement(ElementRef element);

    /// @brief Reorders an element within its current parent.
    Result reorderElement(
        ElementRef element,
        std::size_t index
    );

    /// @brief Moves an element within its owning face.
    Result moveElement(
        ElementRef element,
        const ElementPlacement& where
    );

    /// @brief Replaces an element's type/properties while preserving its position.
    Result replaceElement(
        ElementRef element,
        const std::string& json
    );

    /// @brief Serializes one element's type/properties.
    Result serializeElement(ElementRef element) const;

    //----------[ PROPERTIES ]----------//

    /// @brief Sets one face or element property from JSON.
    Result setFaceProperty(
        FaceId faceId,
        const std::string& path,
        const std::string& json
    );

    /// @brief Sets one element property from JSON.
    Result setElementProperty(
        ElementRef element,
        const std::string& path,
        const std::string& json
    );

    /// @brief Gets one face property.
    Result getFaceProperty(
        FaceId faceId,
        const std::string& path
    ) const;

    /// @brief Gets one element property.
    Result getElementProperty(
        ElementRef element,
        const std::string& path
    ) const;

    /// @brief Gets face property metadata.
    Result getFacePropertiesMeta(
        FaceId faceId,
        const std::string& path = ""
    ) const;

    /// @brief Gets element property metadata.
    Result getElementPropertiesMeta(
        ElementRef element,
        const std::string& path = ""
    ) const;

    //----------[ HISTORY ]----------//

    /// @brief Returns whether undo is available.
    bool canUndo() const { return history_.canUndo(); }

    /// @brief Returns whether redo is available.
    bool canRedo() const { return history_.canRedo(); }

    /// @brief Restores the preceding package snapshot.
    bool undo() { return history_.undo(); }

    /// @brief Restores the next package snapshot.
    bool redo() { return history_.redo(); }

    /// @brief Moves the history cursor to an index.
    bool jumpTo(std::size_t index) { return history_.jumpTo(index); }

    /// @brief Returns the current history index.
    std::size_t historyIndex() const { return history_.headIndex(); }

    /// @brief Returns history command names.
    Result getHistory() const;

private:
    friend class EditorPreview;

    struct FaceEntry {
        FaceId id = 0;
        FaceMeta meta;
        std::unique_ptr<GaugeFace> face;
    };

    struct AssetChange {
        enum class Kind : std::uint8_t { Upsert, Remove, Reset };

        std::size_t revision = 0;
        Kind kind = Kind::Reset;
        std::string name;
    };

    [[nodiscard]] GaugeFace* face(FaceId id) noexcept;
    [[nodiscard]] const GaugeFace* face(FaceId id) const noexcept;
    [[nodiscard]] Element* element(ElementRef element) noexcept;
    [[nodiscard]] const Element* element(ElementRef element) const noexcept;
    [[nodiscard]] std::size_t faceIndex(FaceId id) const noexcept;
    [[nodiscard]] static std::size_t clampIndex(std::size_t index, std::size_t size) noexcept;
    std::size_t assetUseCount(const std::string& name) const;
    std::size_t assetRevision() const { return assetRevision_; }
    bool assetChangesSince(std::size_t revision, std::vector<AssetChange>& out) const;
    std::size_t revision() const { return revision_; }
    bool restorePackage(const std::string& json);
    bool commit(const std::string& name, const std::function<bool()>& mutation);
    void recordAssetChange(AssetChange::Kind kind, std::string name = {});

    std::vector<FaceEntry> faces_;
    PackageInfo package_;
    std::vector<Asset> assets_;
    std::vector<AssetChange> assetChanges_;
    std::size_t assetRevision_ = 0;
    std::size_t revision_ = 0;
    FaceId nextFaceId_ = 1;
    History history_;
};

} // namespace mg::editor
