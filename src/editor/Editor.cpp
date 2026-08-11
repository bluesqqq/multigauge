#include <multigauge/editor/Editor.h>


#include <algorithm>
#include <cstdint>
#include <limits>

#include <multigauge/utils/Json.h>

namespace mg::editor {
namespace {

bool writeHandle(json::Writer& writer, Editor::FaceId faceId, NodeHandle handle) {
    return writer.writeObject([&](json::ObjectWriter& object) {
        return object.write("faceId", static_cast<std::uint64_t>(faceId)) &&
               object.write("slot", static_cast<std::uint64_t>(handle.slot())) &&
               object.write("generation", static_cast<std::uint64_t>(handle.generation()));
    });
}

bool writeElementHierarchy(json::ObjectWriter& object,
                           const GaugeFace& face,
                           Editor::FaceId faceId,
                           NodeHandle handle) {
    const Element* element = face.get(handle);
    if (!element) return false;

    const char* type = element->typeId();
    return object.writeValue(
               "element",
               [&](json::Writer& value) { return writeHandle(value, faceId, handle); }) &&
           object.write("type", type ? type : "") &&
           object.writeArray("children", [&](json::ArrayWriter& children) {
               bool success = true;
               face.forEachChild(handle, [&](NodeHandle child, const Element&) {
                   if (success)
                       success = children.writeObject([&](json::ObjectWriter& childObject) {
                           return writeElementHierarchy(childObject, face, faceId, child);
                       });
               });
               return success;
           });
}

Element::OwnedElement decodeElementDefinition(json::Reader value) {
    if (!value.isObject()) return {};

    std::string_view type;
    if (!value.member(TYPE_KEY).read(type) || type.empty()) return {};

    Element::OwnedElement element = Element::registry().create(type);
    if (!element || !element->loadProperties(value)) return {};
    return element;
}

bool setPropertyPath(::mg::PropertyObject& object, const std::string& path, json::Reader value) {
    ::mg::PropertyObject* owner = nullptr;
    const ::mg::Property* property = nullptr;
    return object.resolvePath(path, owner, property) && owner && property &&
           owner->setProperty(property->key, value);
}

} // namespace

void Editor::clear() {
    faces_.clear();
    package_ = {};
    nextFaceId_ = 1;
    history_ = {};
}

bool Editor::setPackageInfo(const PackageInfo& info) {
    return commit("set package info", [this, info]() {
        package_ = info;
        return true;
    });
}

bool Editor::setFaceName(FaceId id, const std::string& name) {
    if (faceIndex(id) == Append) return false;
    return commit("set face name", [this, id, name]() {
        const std::size_t index = faceIndex(id);
        if (index == Append) return false;
        faces_[index].meta.name = name;
        return true;
    });
}

std::string Editor::getFaceName(FaceId id) const {
    const std::size_t index = faceIndex(id);
    return index == Append ? std::string{} : faces_[index].meta.name;
}

GaugeFace* Editor::face(FaceId id) noexcept {
    const std::size_t index = faceIndex(id);
    return index == Append ? nullptr : faces_[index].face.get();
}

const GaugeFace* Editor::face(FaceId id) const noexcept {
    const std::size_t index = faceIndex(id);
    return index == Append ? nullptr : faces_[index].face.get();
}

Element* Editor::element(ElementRef reference) noexcept {
    GaugeFace* owner = face(reference.faceId);
    return owner ? owner->get(reference.handle) : nullptr;
}

const Element* Editor::element(ElementRef reference) const noexcept {
    const GaugeFace* owner = face(reference.faceId);
    return owner ? owner->get(reference.handle) : nullptr;
}

std::size_t Editor::faceIndex(FaceId id) const noexcept {
    for (std::size_t index = 0; index < faces_.size(); ++index) {
        if (faces_[index].id == id) return index;
    }
    return Append;
}

std::size_t Editor::clampIndex(std::size_t index, std::size_t size) noexcept {
    return index == Append || index > size ? size : index;
}

bool Editor::restorePackage(const std::string& text) {
    const json::Document document = json::parse(text);
    if (!document.valid() || !document.root().isObject()) return false;

    PackageInfo package;
    if (!json::getStringMember(document.root(), "name", package.name) ||
        !json::getStringMember(document.root(), "author", package.author) ||
        !json::getStringMember(document.root(), "description", package.description)) {
        return false;
    }

    const json::Reader entries = json::getArrayMember(document.root(), "faces");
    if (!entries.valid()) return false;

    std::vector<FaceEntry> loaded;
    loaded.reserve(entries.size());
    FaceId nextId = 1;
    for (std::size_t index = 0; index < entries.size(); ++index) {
        const json::Reader entry = entries.element(index);
        std::uint64_t rawId = 0;
        std::string name;
        if (!entry.isObject() || !json::getStringMember(entry, "name", name)) {
            return false;
        }

        if (entry.member("id").valid() && (!entry.member("id").read(rawId) || rawId == 0 ||
                                           rawId >= std::numeric_limits<FaceId>::max()))
            return false;
        const FaceId id = rawId ? static_cast<FaceId>(rawId) : nextId;
        if (std::any_of(loaded.begin(), loaded.end(), [id](const FaceEntry& face) {
                return face.id == id;
            })) {
            return false;
        }

        auto value = std::make_unique<GaugeFace>();
        if (!value->load(json::getObjectMember(entry, "face"))) return false;
        loaded.push_back({id, {std::move(name)}, std::move(value)});
        nextId = std::max(nextId, static_cast<FaceId>(id + 1));
    }

    faces_ = std::move(loaded);
    package_ = std::move(package);
    nextFaceId_ = nextId;
    return true;
}

bool Editor::loadPackage(const std::string& text) {
    if (!restorePackage(text)) return false;
    history_ = {};
    return true;
}

std::string Editor::exportPackage() const {
    json::Document document = json::object();
    json::Writer writer = document.writer();
    if (!writer.writeObject([&](json::ObjectWriter& object) {
            return object.write("name", package_.name) && object.write("author", package_.author) &&
                   object.write("description", package_.description) &&
                   object.writeArray("faces", [&](json::ArrayWriter& entries) {
                       for (const FaceEntry& entry : faces_) {
                           if (!entries.writeObject([&](json::ObjectWriter& item) {
                                   return item.write("name", entry.meta.name) &&
                                          item.writeObject(
                                              "face",
                                              [&](json::ObjectWriter& faceObject) {
                                                  return entry.face->savePropertyMembers(
                                                      faceObject);
                                              });
                               }))
                               return false;
                       }
                       return true;
                   });
        }))
        return {};
    return document.toString();
}

bool Editor::commit(const std::string& name, const std::function<bool()>& mutation) {
    const std::string before = exportPackage();
    if (before.empty()) return false;

    auto after = std::make_shared<std::string>();
    return history_.commit({
        name,
        [this, mutation, after]() {
            if (!after->empty()) return restorePackage(*after);
            if (!mutation()) return false;
            *after = exportPackage();
            return !after->empty();
        },
        [this, before]() { return restorePackage(before); },
    });
}

Result Editor::createFace(const std::string& text, FacePlacement where) {
    const FaceId id = nextFaceId_;
    if (!commit("create face", [this, text, where, id]() {
            const json::Document document = json::parse(text);
            if (!document.valid() || !document.root().isObject()) return false;

            auto value = std::make_unique<GaugeFace>();
            if (!value->load(document.root())) return false;

            faces_.insert(faces_.begin() + clampIndex(where.index, faces_.size()),
                          {
                              id,
                              {"Face"},
                              std::move(value),
                          });
            nextFaceId_ = std::max(nextFaceId_, id + 1);
            return true;
        }))
        return Error("Invalid face JSON");

    Result result = OkObject();
    json::Writer writer = result.data.writer();
    return writer.writeObject([&](json::ObjectWriter& object) {
        return object.write("id", static_cast<std::uint64_t>(id));
    })
               ? std::move(result)
               : Error("Failed to create face result");
}

Result Editor::removeFace(FaceId id) {
    if (!face(id)) return Error("Invalid face id");
    return commit("remove face",
                  [this, id]() {
                      faces_.erase(faces_.begin() + faceIndex(id));
                      return true;
                  })
               ? OkObject()
               : Error("Failed to remove face");
}

Result Editor::reorderFace(FaceId id, std::size_t index) {
    const std::size_t from = faceIndex(id);
    if (from == Append) return Error("Invalid face id");
    return commit("reorder face",
                  [this, id, index]() {
                      const std::size_t current = faceIndex(id);
                      FaceEntry entry = std::move(faces_[current]);
                      faces_.erase(faces_.begin() + current);
                      faces_.insert(faces_.begin() + clampIndex(index, faces_.size()),
                                    std::move(entry));
                      return true;
                  })
               ? OkObject()
               : Error("Failed to reorder face");
}

Result Editor::serializeFace(FaceId id) const {
    const GaugeFace* value = face(id);
    if (!value) return Error("Invalid face id");

    json::Document document = json::object();
    json::Writer faceWriter = document.writer();
    if (!value->save(faceWriter)) return Error("Failed to serialize face");

    Result result = OkObject();
    json::Writer writer = result.data.writer();
    return writer.writeObject([&](json::ObjectWriter& object) {
        return object.write("json", document.toString());
    })
               ? std::move(result)
               : Error("Failed to serialize face result");
}

Result Editor::createElement(const ElementPlacement& where, const std::string& text) {
    NodeHandle created;
    if (!commit("create element", [this, where, text, &created]() {
            GaugeFace* owner = face(where.faceId);
            const json::Document document = json::parse(text);
            if (!owner || !document.valid()) return false;

            Element::OwnedElement element = decodeElementDefinition(document.root());
            if (!element) return false;

            created = owner->addElement(std::move(element));
            if (!created.valid()) return false;
            const bool inserted = owner->moveElement(created, where.parent, where.index);
            if (inserted) return true;
            owner->deleteElement(created);
            return false;
        }))
        return Error("Invalid element JSON or parent");

    Result result = OkObject();
    json::Writer writer = result.data.writer();
    return writer.writeObject([&](json::ObjectWriter& object) {
        return object.writeValue("element", [&](json::Writer& value) {
            return writeHandle(value, where.faceId, created);
        });
    })
               ? std::move(result)
               : Error("Failed to create element result");
}

Result Editor::removeElement(ElementRef reference) {
    if (!element(reference)) return Error("Invalid element");
    return commit("remove element",
                  [this, reference]() {
                      GaugeFace* owner = face(reference.faceId);
                      return owner && owner->deleteElement(reference.handle);
                  })
               ? OkObject()
               : Error("Failed to remove element");
}

Result Editor::reorderElement(ElementRef reference, std::size_t index) {
    GaugeFace* owner = face(reference.faceId);
    if (!owner || !owner->get(reference.handle)) return Error("Invalid element");
    const NodeHandle parent = owner->parentOf(reference.handle);
    return commit("reorder element",
                  [this, reference, parent, index]() {
                      GaugeFace* faceValue = face(reference.faceId);
                      if (!faceValue) return false;
                      return faceValue->moveElement(reference.handle, parent, index);
                  })
               ? OkObject()
               : Error("Failed to reorder element");
}

Result Editor::moveElement(ElementRef reference, const ElementPlacement& where) {
    if (reference.faceId != where.faceId) return Error("Cross-face moves are not supported");
    if (!element(reference)) return Error("Invalid element");
    return commit("move element",
                  [this, reference, where]() {
                      GaugeFace* owner = face(reference.faceId);
                      if (!owner) return false;
                      return owner->moveElement(reference.handle, where.parent, where.index);
                  })
               ? OkObject()
               : Error("Failed to move element");
}

Result Editor::replaceElement(ElementRef reference, const std::string& text) {
    if (!element(reference)) return Error("Invalid element");
    return commit("replace element",
                  [this, reference, text]() {
                      GaugeFace* owner = face(reference.faceId);
                      const json::Document document = json::parse(text);
                      if (!owner || !document.valid()) return false;
                      Element::OwnedElement replacement = decodeElementDefinition(document.root());
                      return replacement &&
                             owner->replaceElement(reference.handle, std::move(replacement));
                  })
               ? OkObject()
               : Error("Invalid replacement element JSON");
}

Result Editor::serializeElement(ElementRef reference) const {
    const Element* value = element(reference);
    if (!value) return Error("Invalid element");

    json::Document document = json::object();
    json::Writer elementWriter = document.writer();
    if (!value->saveProperties(elementWriter)) return Error("Failed to serialize element");

    Result result = OkObject();
    json::Writer writer = result.data.writer();
    return writer.writeObject([&](json::ObjectWriter& object) {
        return object.write("json", document.toString());
    })
               ? std::move(result)
               : Error("Failed to serialize element result");
}

Result Editor::setFaceProperty(FaceId id, const std::string& path, const std::string& text) {
    if (!face(id)) return Error("Invalid face id");
    return commit("set face property",
                  [this, id, path, text]() {
                      GaugeFace* value = face(id);
                      const json::Document document = json::parse(text);
                      return value && document.valid() &&
                             setPropertyPath(*value, path, document.root());
                  })
               ? OkObject()
               : Error("Failed to set face property");
}

Result
Editor::setElementProperty(ElementRef reference, const std::string& path, const std::string& text) {
    if (!element(reference)) return Error("Invalid element");
    return commit("set element property",
                  [this, reference, path, text]() {
                      Element* value = element(reference);
                      const json::Document document = json::parse(text);
                      return value && document.valid() &&
                             setPropertyPath(*value, path, document.root());
                  })
               ? OkObject()
               : Error("Failed to set element property");
}

Result Editor::getFaceProperty(FaceId id, const std::string& path) const {
    const GaugeFace* value = face(id);
    if (!value || path.empty()) return Error("Invalid face property");

    const ::mg::PropertyObject* owner = nullptr;
    const ::mg::Property* property = nullptr;
    if (!value->resolvePath(path, owner, property) || !owner || !property)
        return Error("Invalid property path");

    Result result = OkObject();
    json::Writer writer = result.data.writer();
    return writer.writeObject([&](json::ObjectWriter& object) {
        return object.write("id", static_cast<std::uint64_t>(id)) && object.write("path", path) &&
               object.writeValue("value", [&](json::Writer& propertyWriter) {
                   return owner->getProperty(property->key, propertyWriter);
               });
    })
               ? std::move(result)
               : Error("Failed to get face property");
}

Result Editor::getElementProperty(ElementRef reference, const std::string& path) const {
    const Element* value = element(reference);
    if (!value || path.empty()) return Error("Invalid element property");

    const ::mg::PropertyObject* owner = nullptr;
    const ::mg::Property* property = nullptr;
    if (!value->resolvePath(path, owner, property) || !owner || !property)
        return Error("Invalid property path");

    Result result = OkObject();
    json::Writer writer = result.data.writer();
    return writer.writeObject([&](json::ObjectWriter& object) {
        return object.writeValue(
                   "element",
                   [&](json::Writer& referenceWriter) {
                       return writeHandle(referenceWriter, reference.faceId, reference.handle);
                   }) &&
               object.write("path", path) &&
               object.writeValue("value", [&](json::Writer& propertyWriter) {
                   return owner->getProperty(property->key, propertyWriter);
               });
    })
               ? std::move(result)
               : Error("Failed to get element property");
}

Result Editor::getFacePropertiesMeta(FaceId id, const std::string& path) const {
    const GaugeFace* value = face(id);
    if (!value) return Error("Invalid face id");

    const ::mg::PropertyObject* owner = value;
    const ::mg::Property* property = nullptr;
    if (!path.empty() && (!value->resolvePath(path, owner, property) || !owner || !property))
        return Error("Invalid property path");

    Result result = OkObject();
    json::Writer writer = result.data.writer();
    return writer.writeObject([&](json::ObjectWriter& object) {
        if (!object.write("id", static_cast<std::uint64_t>(id))) return false;
        if (!path.empty() && !object.write("path", path)) return false;
        return object.writeValue("meta", [&](json::Writer& metaWriter) {
            return property ? owner->writePropertyMeta(metaWriter, *property)
                            : value->writePropertiesMeta(metaWriter);
        });
    })
               ? std::move(result)
               : Error("Failed to get face property metadata");
}

Result Editor::getElementPropertiesMeta(ElementRef reference, const std::string& path) const {
    const Element* value = element(reference);
    if (!value) return Error("Invalid element");

    const ::mg::PropertyObject* owner = value;
    const ::mg::Property* property = nullptr;
    if (!path.empty() && (!value->resolvePath(path, owner, property) || !owner || !property))
        return Error("Invalid property path");

    Result result = OkObject();
    json::Writer writer = result.data.writer();
    return writer.writeObject([&](json::ObjectWriter& object) {
        if (!object.writeValue("element", [&](json::Writer& referenceWriter) {
                return writeHandle(referenceWriter, reference.faceId, reference.handle);
            }))
            return false;
        if (!path.empty() && !object.write("path", path)) return false;
        return object.writeValue("meta", [&](json::Writer& metaWriter) {
            return property ? owner->writePropertyMeta(metaWriter, *property)
                            : value->writePropertiesMeta(metaWriter);
        });
    })
               ? std::move(result)
               : Error("Failed to get element property metadata");
}

Result Editor::getHierarchy() const {
    Result result = OkObject();
    json::Writer writer = result.data.writer();
    return writer.writeObject([&](json::ObjectWriter& object) {
        return object.writeArray("faces", [&](json::ArrayWriter& faces) {
            for (const FaceEntry& entry : faces_) {
                if (!faces.writeObject([&](json::ObjectWriter& faceObject) {
                        return faceObject.write("id", static_cast<std::uint64_t>(entry.id)) &&
                               faceObject.write("name", entry.meta.name) &&
                               faceObject.writeArray("elements", [&](json::ArrayWriter& roots) {
                                   bool success = true;
                                   entry.face->forEachRoot([&](NodeHandle root, const Element&) {
                                       if (success)
                                           success = roots.writeObject(
                                               [&](json::ObjectWriter& rootObject) {
                                                   return writeElementHierarchy(rootObject,
                                                                                *entry.face,
                                                                                entry.id,
                                                                                root);
                                               });
                                   });
                                   return success;
                               });
                    }))
                    return false;
            }
            return true;
        });
    })
               ? std::move(result)
               : Error("Failed to build hierarchy");
}

Result Editor::listElementTypes() const {
    Result result = OkArray();
    json::Writer writer = result.data.writer();
    return writer.writeArray([&](json::ArrayWriter& entries) {
        for (const auto& descriptor : Element::registry()) {
            if (!entries.writeObject([&](json::ObjectWriter& entry) {
                    return entry.write("name", descriptor.name ? descriptor.name : "") &&
                           entry.write("type", descriptor.id ? descriptor.id : "");
                }))
                return false;
        }
        return true;
    })
               ? std::move(result)
               : Error("Failed to list element types");
}

Result Editor::getHistory() const {
    Result result = OkArray();
    json::Writer writer = result.data.writer();
    return writer.writeArray([&](json::ArrayWriter& entries) {
        for (const std::string& name : history_.names()) {
            if (!entries.write(name)) return false;
        }
        return true;
    })
               ? std::move(result)
               : Error("Failed to get history");
}

} // namespace mg::editor
