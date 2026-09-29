#include <multigauge/settings/Manager.h>

#include "../AppPaths.h"

#include <multigauge/io/FileSystem.h>
#include <multigauge/utils/Json.h>

#include <utility>

namespace mg::settings {

namespace {

bool readString(json::Reader object, const char* key, std::string& value) {
    return json::getStringMember(object, key, value);
}

} // namespace

Manager::Manager(io::FileSystem& fs, std::string dataRoot)
    : fs_(fs), dataRoot_(std::move(dataRoot)) {}

bool Manager::load(graphics::UserPalette& palette) {
    for (std::size_t slot = 0; slot < userColors_.size(); ++slot) {
        userColors_[slot] = palette.color(slot);
    }
    activeFace_.reset();

    const std::string path = paths::settingsPath(dataRoot_);
    if (!fs_.exists(path)) {
        dirty_ = false;
        return true;
    }

    json::Document document;
    return json::readJsonFile(fs_, path, document) && loadDocument(document.root(), palette);
}

bool Manager::save() {
    if (!dirty_) return true;

    json::Document document = json::object();
    json::Writer writer = document.writer();
    if (!writeDocument(writer)) return false;

    const std::string path = paths::settingsPath(dataRoot_);
    const std::string temporaryPath = path + ".tmp";
    if (!json::writeJsonFile(fs_, temporaryPath, document.root()) || !fs_.rename(temporaryPath, path)) {
        (void)fs_.remove(temporaryPath);
        return false;
    }

    dirty_ = false;
    return true;
}

const std::optional<ActiveFace>& Manager::activeFace() const noexcept {
    return activeFace_;
}

void Manager::setActiveFace(std::string packageId, std::string faceId) {
    activeFace_ = ActiveFace{std::move(packageId), std::move(faceId)};
    dirty_ = true;
}

void Manager::clearActiveFace() noexcept {
    if (!activeFace_) return;
    activeFace_.reset();
    dirty_ = true;
}

graphics::rgba Manager::userColor(std::size_t slot) const noexcept {
    return slot < userColors_.size() ? userColors_[slot] : graphics::rgba{0, 0, 0, 0};
}

bool Manager::setUserColor(std::size_t slot, graphics::rgba color) noexcept {
    if (slot >= userColors_.size()) return false;
    userColors_[slot] = color;
    dirty_ = true;
    return true;
}

bool Manager::loadDocument(json::Reader root, graphics::UserPalette& palette) {
    std::uint64_t version = 0;
    const json::Reader colors = json::getArrayMember(root, "userColors");
    if (!root.isObject() || root.size() < 2 || root.size() > 3 ||
        !root.member("version").read(version) || version != 1 ||
        !colors.valid() || !colors.isArray() || colors.size() != userColors_.size()) {
        return false;
    }

    std::array<graphics::rgba, graphics::UserPalette::Size> loadedColors{};
    for (std::size_t slot = 0; slot < loadedColors.size(); ++slot) {
        std::string_view color;
        if (!colors.element(slot).read(color) ||
            !graphics::rgba::fromString(std::string(color).c_str(), loadedColors[slot])) {
            return false;
        }
    }

    std::optional<ActiveFace> loadedActiveFace;
    const json::Reader activeFace = root.member("activeFace");
    if (activeFace.valid()) {
        ActiveFace selection;
        if (!activeFace.isObject() || activeFace.size() != 2 ||
            !readString(activeFace, "packageId", selection.packageId) || selection.packageId.empty() ||
            !readString(activeFace, "faceId", selection.faceId) || selection.faceId.empty()) {
            return false;
        }
        loadedActiveFace = std::move(selection);
    }

    userColors_ = loadedColors;
    activeFace_ = std::move(loadedActiveFace);
    for (std::size_t slot = 0; slot < userColors_.size(); ++slot) {
        (void)palette.setColor(slot, userColors_[slot]);
    }
    dirty_ = false;
    return true;
}

bool Manager::writeDocument(json::Writer& writer) const {
    return writer.writeObject([&](json::ObjectWriter& root) {
        if (!root.write("version", 1) ||
            !root.writeArray("userColors", [&](json::ArrayWriter& colors) {
                for (const graphics::rgba color : userColors_) {
                    if (!colors.write(color.toHex())) return false;
                }
                return true;
            })) {
            return false;
        }
        return !activeFace_ || root.writeObject("activeFace", [&](json::ObjectWriter& selection) {
            return selection.write("packageId", activeFace_->packageId) &&
                   selection.write("faceId", activeFace_->faceId);
        });
    });
}

} // namespace mg::settings
