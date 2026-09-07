#include <multigauge/editor/Editor.h>


#include <algorithm>
#include <cctype>
#include <cstdint>
#include <limits>

#include <multigauge/utils/Json.h>
#include <multigauge/io/Base64.h>

namespace mg::editor {
bool Editor::Asset::valid() const {
    if (name.empty() || mediaType.empty() || data.empty() || data.size() > 64 * 1024) return false;
    if (name == "." || name == ".." ||
            !std::all_of(name.begin(), name.end(), [](unsigned char c) {
                return std::isalnum(c) || c == '.' || c == '_' || c == '-';
            }) ||
            (mediaType != "image/png" && mediaType != "image/jpeg" && mediaType != "image/bmp")) {
        return false;
    }
    std::vector<uint8_t> decoded(io::base64DecodedMaxSize(data));
    std::size_t decodedLength = 0;
    return io::base64Decode(data, decoded.data(), decoded.size(), decodedLength) && decodedLength != 0;
}

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

bool readAssets(json::Reader value, std::vector<Editor::Asset>& out) {
    if (!value.isArray() || value.size() > 16) return false;

    std::vector<Editor::Asset> assets;
    assets.reserve(value.size());
    std::size_t totalDataBytes = 0;
    for (std::size_t index = 0; index < value.size(); ++index) {
        const json::Reader item = value.element(index);
        Editor::Asset asset;
        if (!item.isObject() || item.size() != 3 ||
            !json::getStringMember(item, "name", asset.name) ||
            !json::getStringMember(item, "mediaType", asset.mediaType) ||
            !json::getStringMember(item, "data", asset.data) || !asset.valid()) {
            return false;
        }
        totalDataBytes += asset.data.size();
        if (totalDataBytes > 128 * 1024 || std::any_of(assets.begin(), assets.end(), [&](const Editor::Asset& existing) {
                return existing.name == asset.name;
            })) {
            return false;
        }
        assets.push_back(std::move(asset));
    }

    out = std::move(assets);
    return true;
}

bool writeAssets(json::ArrayWriter& output, const std::vector<Editor::Asset>& assets) {
    for (const auto& asset : assets) {
        if (!output.writeObject([&](json::ObjectWriter& item) {
                return item.write("name", asset.name) && item.write("mediaType", asset.mediaType) &&
                       item.write("data", asset.data);
            }))
            return false;
    }
    return true;
}

std::size_t countAssetReferences(const GaugeFace& face, NodeHandle handle, const std::string& name) {
    const Element* element = face.get(handle);
    if (!element) return 0;

    std::size_t count = 0;
    if (std::string_view(element->typeId() ? element->typeId() : "") == "image") {
        json::Document path = json::object();
        json::Writer writer = path.writer();
        std::string_view value;
        if (element->getProperty("path", writer) && path.root().read(value) && value == name) ++count;
    }
    face.forEachChild(handle, [&](NodeHandle child, const Element&) {
        count += countAssetReferences(face, child, name);
    });
    return count;
}

} // namespace

void Editor::clear() {
    faces_.clear();
    package_ = {};
    assets_.clear();
    recordAssetChange(AssetChange::Kind::Reset);
    ++revision_;
    nextFaceId_ = 1;
    history_ = {};
}

bool Editor::setPackageInfo(const PackageInfo& info) {
    return commit("set package info", [this, info]() {
        package_ = info;
        return true;
    });
}

bool Editor::setAsset(const Asset& asset) {
    if (!asset.valid()) return false;
    return commit("upsert package asset", [this, asset]() {
        std::size_t totalDataBytes = asset.data.size();
        auto existing = std::find_if(assets_.begin(), assets_.end(), [&](const Asset& value) {
            return value.name == asset.name;
        });
        for (const auto& value : assets_) {
            if (value.name != asset.name) totalDataBytes += value.data.size();
        }
        if ((existing == assets_.end() && assets_.size() >= 16) || totalDataBytes > 128 * 1024) return false;
        if (existing == assets_.end()) assets_.push_back(asset);
        else *existing = asset;
        recordAssetChange(AssetChange::Kind::Upsert, asset.name);
        return true;
    });
}

bool Editor::removeAsset(const std::string& name) {
    if (assetUseCount(name) != 0) return false;
    return commit("remove package asset", [this, name]() {
        const auto existing = std::find_if(assets_.begin(), assets_.end(), [&](const Asset& value) {
            return value.name == name;
        });
        if (existing == assets_.end()) return false;
        assets_.erase(existing);
        recordAssetChange(AssetChange::Kind::Remove, name);
        return true;
    });
}

std::size_t Editor::assetUseCount(const std::string& name) const {
    std::size_t count = 0;
    for (const auto& entry : faces_) {
        entry.face->forEachRoot([&](NodeHandle root, const Element&) {
            count += countAssetReferences(*entry.face, root, name);
        });
    }
    return count;
}

bool Editor::assetChangesSince(std::size_t revision, std::vector<AssetChange>& out) const {
    out.clear();
    if (revision == assetRevision_) return true;
    if (assetChanges_.empty() || revision + 1 < assetChanges_.front().revision) return false;
    for (const auto& change : assetChanges_) {
        if (change.revision > revision) out.push_back(change);
    }
    return !out.empty();
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

    std::vector<Asset> assets;
    const json::Reader assetsValue = document.root().member("assets");
    if (assetsValue.valid() && !readAssets(assetsValue, assets)) return false;

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
    assets_ = std::move(assets);
    recordAssetChange(AssetChange::Kind::Reset);
    nextFaceId_ = nextId;
    ++revision_;
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
                   object.writeArray("assets", [&](json::ArrayWriter& assets) {
                       return writeAssets(assets, assets_);
                   }) &&
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
            ++revision_;
            *after = exportPackage();
            return !after->empty();
        },
        [this, before]() { return restorePackage(before); },
    });
}

void Editor::recordAssetChange(AssetChange::Kind kind, std::string name) {
    constexpr std::size_t MaxChanges = 32;
    AssetChange change;
    change.revision = ++assetRevision_;
    change.kind = kind;
    change.name = std::move(name);
    assetChanges_.push_back(std::move(change));
    if (assetChanges_.size() > MaxChanges) assetChanges_.erase(assetChanges_.begin());
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

Result Editor::getFaceInspector(FaceId id) const {
    const GaugeFace* value = face(id);
    if (!value) return Error("Invalid face id");

    Result result = OkObject();
    json::Writer writer = result.data.writer();
    return writer.writeObject([&](json::ObjectWriter& object) {
        if (!object.write("id", static_cast<std::uint64_t>(id))) return false;
        return object.writeValue("inspector", [&](json::Writer& inspector) {
            return value->writeInspectorMeta(inspector);
        });
    })
               ? std::move(result) : Error("Failed to get face inspector");
}

Result Editor::getElementInspector(ElementRef reference) const {
    const Element* value = element(reference);
    if (!value) return Error("Invalid element");

    Result result = OkObject();
    json::Writer writer = result.data.writer();
    return writer.writeObject([&](json::ObjectWriter& object) {
        if (!object.writeValue("element", [&](json::Writer& referenceWriter) {
                return writeHandle(referenceWriter, reference.faceId, reference.handle);
            }))
            return false;
        return object.writeValue("inspector", [&](json::Writer& inspector) {
            return value->writeInspectorMeta(inspector);
        });
    })
               ? std::move(result) : Error("Failed to get element inspector");
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
