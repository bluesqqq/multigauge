#include <multigauge/editor/Manager.h>

#include <cstdint>

#include <multigauge/editor/Clipboard.h>
#include <multigauge/editor/Editor.h>
#include <multigauge/io/Log.h>
#include <multigauge/value/ValueRegistry.h>

namespace mg::editor {
namespace {
Editor* editor(Manager& manager, EditorId id) {
    return manager.find(id);
}

Result invalidEditor(EditorId id) {
    constexpr const char* TAG = "EditorManager";

    LOG_WARN(
        TAG,
        "Invalid editor ID: slot=%u generation=%u",
        id.slot(),
        id.generation()
    );

    return Error("Invalid editor ID");
}

Result jsonResult(const std::string& json) {
    Result result = OkObject();
    json::Writer writer = result.data.writer();
    return writer.writeObject(
               [&](json::ObjectWriter& object) { return object.write("json", json); })
               ? std::move(result)
               : Error("Failed to build JSON result");
}

Result packageInfoResult(const Editor::PackageInfo& info) {
    Result result = OkObject();
    json::Writer writer = result.data.writer();
    return writer.writeObject([&](json::ObjectWriter& object) {
        return object.write("name", info.name) && object.write("author", info.author) &&
               object.write("description", info.description);
    })
               ? std::move(result)
               : Error("Failed to build package result");
}
} // namespace

namespace {
namespace detail {

Result setPackageInfo(
    Manager& manager, EditorId id,
    const std::string& name,
    const std::string& author,
    const std::string& description
) {
    auto* value = editor(manager, id);
    if (!value) return invalidEditor(id);
    const Editor::PackageInfo info{name, author, description};
    return value->setPackageInfo(info)
               ? packageInfoResult(value->packageInfo())
               : Error("Failed to set package info");
}

Result getPackageInfo(Manager& manager, EditorId id) {
    auto* value = editor(manager, id);
    return value ? packageInfoResult(value->packageInfo()) : invalidEditor(id);
}

Result getAssets(Manager& manager, EditorId id) {
    auto* value = editor(manager, id);
    if (!value) return invalidEditor(id);

    Result result = OkArray();
    json::Writer writer = result.data.writer();
    return writer.writeArray([&](json::ArrayWriter& output) {
        for (const auto& asset : value->assets()) {
            if (!output.writeObject([&](json::ObjectWriter& item) {
                    return item.write("name", asset.name) && item.write("mediaType", asset.mediaType) &&
                           item.write("data", asset.data);
                }))
                return false;
        }
        return true;
    }) ? std::move(result) : Error("Failed to build asset result");
}

Result setAsset(Manager& manager, EditorId id, const std::string& name, const std::string& mediaType, const std::string& data) {
    auto* value = editor(manager, id);
    if (!value) return invalidEditor(id);
    return value->setAsset({name, mediaType, data}) ? OkObject() : Error("Invalid package asset");
}

Result removeAsset(Manager& manager, EditorId id, const std::string& name) {
    auto* value = editor(manager, id);
    if (!value) return invalidEditor(id);
    return value->removeAsset(name) ? OkObject() : Error("Asset is unknown or still in use");
}

Result setFaceName(
    Manager& manager, EditorId id,
    NodeId faceId,
    const std::string& name
) {
    auto* value = editor(manager, id);
    if (!value) return invalidEditor(id);
    return value->setFaceName(faceId, name) ? OkObject() : Error("Invalid face ID");
}

Result getFaceName(
    Manager& manager, EditorId id,
    NodeId faceId
) {
    auto* value = editor(manager, id);
    if (!value) return invalidEditor(id);
    if (!value->getFace(faceId)) return Error("Invalid face ID");
    Result result = OkObject();
    json::Writer writer = result.data.writer();
    return writer.writeObject([&](json::ObjectWriter& object) {
        return object.write("id", static_cast<std::uint64_t>(faceId)) &&
               object.write("name", value->getFaceName(faceId));
    })
               ? std::move(result)
               : Error("Failed to build face result");
}

Result loadPackage(
    Manager& manager, EditorId id,
    const std::string& text
) {
    auto* value = editor(manager, id);
    return !value ? invalidEditor(id)
                  : (value->loadPackage(text) ? OkObject() : Error("Invalid package JSON"));
}

Result exportPackage(Manager& manager, EditorId id) {
    auto* value = editor(manager, id);
    return value ? jsonResult(value->exportPackage()) : invalidEditor(id);
}

Result getHierarchy(Manager& manager, EditorId id) {
    auto* value = editor(manager, id);
    return value ? value->getHierarchy() : invalidEditor(id);
}

Result listElementTypes(Manager& manager, EditorId id) {
    auto* value = editor(manager, id);
    return value ? value->listElementTypes() : invalidEditor(id);
}

Result listValueIDs(Manager& manager, EditorId id) {
    if (!editor(manager, id)) return invalidEditor(id);

    Result result = OkArray();
    json::Writer writer = result.data.writer();
    return writer.writeArray([&](json::ArrayWriter& entries) {
        bool written = true;
        ValueRegistry::forEach([&](ValueHandle handle) {
            written = written && entries.write(ValueRegistry::id(handle));
        });
        return written;
    })
               ? std::move(result)
               : Error("Failed to list value IDs");
}

Result createFace(
    Manager& manager, EditorId id,
    const std::string& text,
    FacePlacement where
) {
    auto* value = editor(manager, id);
    return value ? value->createFace(text, where) : invalidEditor(id);
}

Result removeFace(
    Manager& manager, EditorId id,
    NodeId faceId
) {
    auto* value = editor(manager, id);
    return value ? value->removeFace(faceId) : invalidEditor(id);
}

Result reorderFace(
    Manager& manager, EditorId id,
    NodeId faceId,
    std::size_t index
) {
    auto* value = editor(manager, id);
    return value ? value->reorderFace(faceId, index) : invalidEditor(id);
}

Result createElement(
    Manager& manager, EditorId id,
    const ElementPlacement& where,
    const std::string& text
) {
    auto* value = editor(manager, id);
    return value ? value->createElement(where, text) : invalidEditor(id);
}

Result removeElement(
    Manager& manager, EditorId id,
    ElementRef ref
) {
    auto* value = editor(manager, id);
    return value ? value->removeElement(ref) : invalidEditor(id);
}

Result reorderElement(
    Manager& manager, EditorId id,
    ElementRef ref,
    std::size_t index
) {
    auto* value = editor(manager, id);
    return value ? value->reorderElement(ref, index) : invalidEditor(id);
}

Result moveElement(
    Manager& manager, EditorId id,
    ElementRef ref,
    const ElementPlacement& where
) {
    auto* value = editor(manager, id);
    return value ? value->moveElement(ref, where) : invalidEditor(id);
}

Result replaceElement(
    Manager& manager, EditorId id,
    ElementRef ref,
    const std::string& text
) {
    auto* value = editor(manager, id);
    return value ? value->replaceElement(ref, text) : invalidEditor(id);
}

Result setFaceProperty(
    Manager& manager, EditorId id,
    NodeId faceId,
    const std::string& path,
    const std::string& text
) {
    auto* value = editor(manager, id);
    return value ? value->setFaceProperty(faceId, path, text) : invalidEditor(id);
}

Result getFaceProperty(
    Manager& manager, EditorId id,
    NodeId faceId,
    const std::string& path
) {
    auto* value = editor(manager, id);
    return value ? value->getFaceProperty(faceId, path) : invalidEditor(id);
}

Result getFacePropertiesMeta(
    Manager& manager, EditorId id,
    NodeId faceId,
    const std::string& path
) {
    auto* value = editor(manager, id);
    return value ? value->getFacePropertiesMeta(faceId, path) : invalidEditor(id);
}

Result setElementProperty(
    Manager& manager, EditorId id,
    ElementRef ref,
    const std::string& path,
    const std::string& text
) {
    auto* value = editor(manager, id);
    return value ? value->setElementProperty(ref, path, text) : invalidEditor(id);
}

Result getElementProperty(
    Manager& manager, EditorId id,
    ElementRef ref,
    const std::string& path
) {
    auto* value = editor(manager, id);
    return value ? value->getElementProperty(ref, path) : invalidEditor(id);
}

Result getElementPropertiesMeta(
    Manager& manager, EditorId id,
    ElementRef ref,
    const std::string& path
) {
    auto* value = editor(manager, id);
    return value ? value->getElementPropertiesMeta(ref, path) : invalidEditor(id);
}

ClipboardState::Kind clipboardKind(const Manager& manager) {
    return manager.clipboard().kind;
}

void clearClipboard(Manager& manager) {
    manager.clipboard().clear();
}

Result copyFace(
    Manager& manager, EditorId id,
    NodeId faceId
) {
    auto* value = editor(manager, id);
    if (!value) return invalidEditor(id);
    Result result = value->serializeFace(faceId);
    if (!result.ok) return result;
    std::string_view text;
    if (!result.data.root().member("json").read(text)) return Error("Invalid serialized face");
    manager.clipboard() = {ClipboardState::Kind::Face, std::string(text)};
    return OkObject();
}

Result cutFace(
    Manager& manager, EditorId id,
    NodeId faceId
) {
    Result result = copyFace(manager, id, faceId);
    if (!result.ok) return std::move(result);
    return removeFace(manager, id, faceId);
}

Result pasteFace(
    Manager& manager, EditorId id,
    FacePlacement where
) {
    auto* value = editor(manager, id);
    if (!value) return invalidEditor(id);
    return manager.clipboard().kind == ClipboardState::Kind::Face
               ? value->createFace(manager.clipboard().json, where)
               : Error("Clipboard does not contain a face");
}

Result copyElement(
    Manager& manager, EditorId id,
    ElementRef ref
) {
    auto* value = editor(manager, id);
    if (!value) return invalidEditor(id);
    Result result = value->serializeElement(ref);
    if (!result.ok) return result;
    std::string_view text;
    if (!result.data.root().member("json").read(text)) return Error("Invalid serialized element");
    manager.clipboard() = {ClipboardState::Kind::Element, std::string(text)};
    return OkObject();
}

Result cutElement(
    Manager& manager, EditorId id,
    ElementRef ref
) {
    Result result = copyElement(manager, id, ref);
    if (!result.ok) return std::move(result);
    return removeElement(manager, id, ref);
}

Result pasteElement(
    Manager& manager, EditorId id,
    const ElementPlacement& where
) {
    auto* value = editor(manager, id);
    if (!value) return invalidEditor(id);
    return manager.clipboard().kind == ClipboardState::Kind::Element
               ? value->createElement(where, manager.clipboard().json)
               : Error("Clipboard does not contain an element");
}

Result pasteToReplaceElement(
    Manager& manager, EditorId id,
    ElementRef ref
) {
    auto* value = editor(manager, id);
    if (!value) return invalidEditor(id);
    return manager.clipboard().kind == ClipboardState::Kind::Element
               ? value->replaceElement(ref, manager.clipboard().json)
               : Error("Clipboard does not contain an element");
}

Result getHistory(Manager& manager, EditorId id) {
    auto* value = editor(manager, id);
    return value ? value->getHistory() : invalidEditor(id);
}

} // namespace detail
} // namespace

} // namespace mg::editor

namespace mg::editor {

Result Manager::setPackageInfo(EditorId id, const std::string& name, const std::string& author, const std::string& description) { return detail::setPackageInfo(*this, id, name, author, description); }
Result Manager::getPackageInfo(EditorId id) { return detail::getPackageInfo(*this, id); }
Result Manager::getAssets(EditorId id) { return detail::getAssets(*this, id); }
Result Manager::setAsset(EditorId id, const std::string& name, const std::string& mediaType, const std::string& data) { return detail::setAsset(*this, id, name, mediaType, data); }
Result Manager::removeAsset(EditorId id, const std::string& name) { return detail::removeAsset(*this, id, name); }
Result Manager::setFaceName(EditorId id, NodeId faceId, const std::string& name) { return detail::setFaceName(*this, id, faceId, name); }
Result Manager::getFaceName(EditorId id, NodeId faceId) { return detail::getFaceName(*this, id, faceId); }
Result Manager::loadPackage(EditorId id, const std::string& json) { return detail::loadPackage(*this, id, json); }
Result Manager::exportPackage(EditorId id) { return detail::exportPackage(*this, id); }
Result Manager::getHierarchy(EditorId id) { return detail::getHierarchy(*this, id); }
Result Manager::listElementTypes(EditorId id) { return detail::listElementTypes(*this, id); }
Result Manager::listValueIDs(EditorId id) { return detail::listValueIDs(*this, id); }
Result Manager::createFace(EditorId id, const std::string& json, FacePlacement where) { return detail::createFace(*this, id, json, where); }
Result Manager::removeFace(EditorId id, NodeId faceId) { return detail::removeFace(*this, id, faceId); }
Result Manager::reorderFace(EditorId id, NodeId faceId, std::size_t index) { return detail::reorderFace(*this, id, faceId, index); }
Result Manager::createElement(EditorId id, const ElementPlacement& where, const std::string& json) { return detail::createElement(*this, id, where, json); }
Result Manager::removeElement(EditorId id, ElementRef element) { return detail::removeElement(*this, id, element); }
Result Manager::reorderElement(EditorId id, ElementRef element, std::size_t index) { return detail::reorderElement(*this, id, element, index); }
Result Manager::moveElement(EditorId id, ElementRef element, const ElementPlacement& where) { return detail::moveElement(*this, id, element, where); }
Result Manager::replaceElement(EditorId id, ElementRef element, const std::string& json) { return detail::replaceElement(*this, id, element, json); }
Result Manager::setFaceProperty(EditorId id, NodeId faceId, const std::string& path, const std::string& json) { return detail::setFaceProperty(*this, id, faceId, path, json); }
Result Manager::getFaceProperty(EditorId id, NodeId faceId, const std::string& path) { return detail::getFaceProperty(*this, id, faceId, path); }
Result Manager::getFacePropertiesMeta(EditorId id, NodeId faceId, const std::string& path) { return detail::getFacePropertiesMeta(*this, id, faceId, path); }
Result Manager::setElementProperty(EditorId id, ElementRef element, const std::string& path, const std::string& json) { return detail::setElementProperty(*this, id, element, path, json); }
Result Manager::getElementProperty(EditorId id, ElementRef element, const std::string& path) { return detail::getElementProperty(*this, id, element, path); }
Result Manager::getElementPropertiesMeta(EditorId id, ElementRef element, const std::string& path) { return detail::getElementPropertiesMeta(*this, id, element, path); }
ClipboardState::Kind Manager::clipboardKind() const { return detail::clipboardKind(*this); }
void Manager::clearClipboard() { detail::clearClipboard(*this); }
Result Manager::copyFace(EditorId id, NodeId faceId) { return detail::copyFace(*this, id, faceId); }
Result Manager::cutFace(EditorId id, NodeId faceId) { return detail::cutFace(*this, id, faceId); }
Result Manager::pasteFace(EditorId id, FacePlacement where) { return detail::pasteFace(*this, id, where); }
Result Manager::copyElement(EditorId id, ElementRef element) { return detail::copyElement(*this, id, element); }
Result Manager::cutElement(EditorId id, ElementRef element) { return detail::cutElement(*this, id, element); }
Result Manager::pasteElement(EditorId id, const ElementPlacement& where) { return detail::pasteElement(*this, id, where); }
Result Manager::pasteToReplaceElement(EditorId id, ElementRef element) { return detail::pasteToReplaceElement(*this, id, element); }
Result Manager::getHistory(EditorId id) { return detail::getHistory(*this, id); }

} // namespace mg::editor
