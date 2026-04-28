#pragma once

#include <string>
#include <functional>
#include <vector>

namespace mg::editor {

struct Command {
    std::string name;
    std::function<bool()> execute;
    std::function<bool()> unexecute;
};

class History {
    private:
        std::vector<Command> historyStack;
        int head = -1;
    public:
        bool commit(Command cmd) {
            if (!cmd.execute()) return false;

            historyStack.erase(historyStack.begin() + head + 1, historyStack.end());
            historyStack.push_back(std::move(cmd));
            head = (int)historyStack.size() - 1;
            return true;
        }
        
        bool canUndo() const { return head >= 0; }
        bool canRedo() const { return head < (int)historyStack.size() - 1; }

        bool undo() {
            if (!canUndo()) return false;
            if (!historyStack[head].unexecute()) return false;
            --head;
            return true;
        }

        bool redo() {
            if (!canRedo()) return false;
            if (!historyStack[head + 1].execute()) return false;
            ++head;
            return true;
        }
};

}