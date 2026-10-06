#include <multigauge/control/Manager.h>

#include "../AppPaths.h"

#include <multigauge/io/FileSystem.h>
#include <multigauge/navigation/Manager.h>
#include <multigauge/utils/Json.h>

#include <cstdint>
#include <utility>

namespace mg::control {

namespace {

const char* actionName(Action action) {
    switch (action) {
        case Action::Next: return "next";
        case Action::Select: return "select";
        case Action::Previous: return "previous";
    }
    return nullptr;
}

bool readAction(std::string_view value, Action& out) {
    if (value == "next") out = Action::Next;
    else if (value == "select") out = Action::Select;
    else if (value == "previous") out = Action::Previous;
    else return false;
    return true;
}

} // namespace

Manager::Manager(
    io::FileSystem& fs,
    std::string dataRoot,
    navigation::Manager& navigation
) : fs_(fs),
    dataRoot_(std::move(dataRoot)),
    navigation_(navigation) {}

std::size_t Manager::portCount() const noexcept { return MaxPorts; }

bool Manager::load() {
    const std::string path = paths::controlsPath(dataRoot_);
    if (!fs_.exists(path)) {
        for (Slot& slot : slots_) slot.bound = false;
        dirty_ = false;
        return true;
    }

    json::Document document;
    return json::readJsonFile(fs_, path, document) && loadDocument(document.root());
}

bool Manager::save() {
    if (!dirty_) return true;

    json::Document document = json::object();
    json::Writer writer = document.writer();
    if (!writeDocument(writer)) return false;

    const std::string path = paths::controlsPath(dataRoot_);
    const std::string temporaryPath = path + ".tmp";
    if (!json::writeJsonFile(fs_, temporaryPath, document.root()) || !fs_.rename(temporaryPath, path)) {
        (void)fs_.remove(temporaryPath);
        return false;
    }

    dirty_ = false;
    return true;
}

bool Manager::bind(std::size_t portIndex, Action action) {
    Slot* slot = slotAt(portIndex);
    if (!slot) return false;

    slot->action = action;
    slot->bound = true;
    dirty_ = true;
    return true;
}

bool Manager::unbind(std::size_t portIndex) {
    Slot* slot = slotAt(portIndex);
    if (!slot || !slot->bound) return false;
    slot->bound = false;
    dirty_ = true;
    return true;
}

void Manager::listBindings(std::vector<Binding>& out) const {
    out.clear();
    for (std::size_t index = 0; index < MaxPorts; ++index) {
        const Slot& slot = slots_[index];
        if (slot.bound) out.push_back({index, slot.action});
    }
}

Result Manager::trigger(ContextId contextId, std::size_t portIndex) {
    const Slot* slot = slotAt(portIndex);
    if (!slot || !slot->bound) return Result::ignored();
    return navigation_.handleControl(contextId, slot->action);
}

Manager::Slot* Manager::slotAt(std::size_t portIndex) noexcept {
    return portIndex < MaxPorts ? &slots_[portIndex] : nullptr;
}

const Manager::Slot* Manager::slotAt(std::size_t portIndex) const noexcept {
    return portIndex < MaxPorts ? &slots_[portIndex] : nullptr;
}

bool Manager::loadDocument(json::Reader root) {
    std::uint64_t version = 0;
    const json::Reader bindings = json::getArrayMember(root, "bindings");
    if (!root.isObject() || root.size() != 2 || !root.member("version").read(version) ||
        version != 1 || !bindings.valid() || !bindings.isArray()) {
        return false;
    }

    std::array<Slot, MaxPorts> loaded = slots_;
    for (Slot& slot : loaded) slot.bound = false;
    for (std::size_t index = 0; index < bindings.size(); ++index) {
        const json::Reader entry = bindings.element(index);
        std::uint64_t portIndex = 0;
        std::string actionType;
        Action action;
        if (!entry.isObject() || !entry.member("port").read(portIndex) ||
            !json::getStringMember(entry, "action", actionType) || !readAction(actionType, action)) {
            return false;
        }

        if (entry.size() != 2) {
            return false;
        }

        if (portIndex >= MaxPorts) continue;
        Slot* slot = &loaded[static_cast<std::size_t>(portIndex)];
        if (slot->bound) return false;
        slot->action = action;
        slot->bound = true;
    }

    slots_ = std::move(loaded);
    dirty_ = false;
    return true;
}

bool Manager::writeDocument(json::Writer& writer) const {
    return writer.writeObject([&](json::ObjectWriter& root) {
        return root.write("version", 1) && root.writeArray("bindings", [&](json::ArrayWriter& bindings) {
            for (std::size_t index = 0; index < MaxPorts; ++index) {
                const Slot& slot = slots_[index];
                if (!slot.bound) continue;
                const char* type = actionName(slot.action);
                if (!type || !bindings.writeObject([&](json::ObjectWriter& entry) {
                    if (!entry.write("port", index) || !entry.write("action", type)) return false;
                    return true;
                })) {
                    return false;
                }
            }
            return true;
        });
    });
}

} // namespace mg::control
