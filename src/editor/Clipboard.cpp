#include <multigauge/editor/Clipboard.h>

namespace mg::editor {

namespace {
ClipboardState g_clipboard;
}

ClipboardState& sharedClipboard() {
    return g_clipboard;
}

ClipboardSummary sharedClipboardSummary() {
    return { g_clipboard.kind };
}

void clearSharedClipboard() {
    g_clipboard.clear();
}

}
