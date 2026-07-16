#include <multigauge/editor/History.h>

namespace mg::editor {

bool History::commit(Command cmd) {
    if (!cmd.execute()) return false;

    historyStack.erase(historyStack.begin() + head, historyStack.end());
    historyStack.push_back(std::move(cmd));
    head = historyStack.size();
    return true;
}

bool History::undo() {
    if (!canUndo()) return false;
    if (!historyStack[head - 1].unexecute()) return false;
    --head;
    return true;
}

bool History::redo() {
    if (!canRedo()) return false;
    if (!historyStack[head].execute()) return false;
    ++head;
    return true;
}

bool History::jumpTo(std::size_t index) {
    if (index > historyStack.size()) return false;
    if (head == index) return true;

    if (head > index) {
        while(head > index) {
            if (!undo()) return false;
        }
    } else {
        while(head < index) {
            if (!redo()) return false;
        }
    }

    return head == index;
}

std::vector<std::string> History::names() const {
    std::vector<std::string> result;
    result.reserve(historyStack.size() + 1);
    result.push_back("begin");

    for (const auto& command : historyStack)
        result.push_back(command.name);

    return result;
}

}
