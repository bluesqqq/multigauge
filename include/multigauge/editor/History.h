#pragma once

#include <cstddef>
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
        std::size_t head = 0;

    public:
        bool commit(Command cmd);
        
        bool canUndo() const { return head > 0; }
        bool canRedo() const { return head < historyStack.size(); }

        bool undo();
        bool redo();
        bool jumpTo(std::size_t index);
        std::size_t headIndex() const { return head; }

        std::vector<std::string> names() const;
};

}
