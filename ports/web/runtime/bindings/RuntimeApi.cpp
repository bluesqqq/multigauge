#include "ApiBindings.h"

#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <utility>

#include <emscripten.h>

#include <multigauge/App.h>
#include <multigauge/HandlePool.h>

#include "platform/GraphicsContextCanvas.h"

namespace {

constexpr mg::ContextId kInvalidContextId = std::numeric_limits<mg::ContextId>::max();

struct RuntimeBinding {
    mg::ContextId bindingId = kInvalidContextId;
    std::string canvasId;
    std::unique_ptr<GraphicsContextCanvas> context;
    mg::ContextId runtimeContextId = kInvalidContextId;

    explicit RuntimeBinding(std::string canvas)
        : canvasId(std::move(canvas)), context(std::make_unique<GraphicsContextCanvas>(canvasId)) {}
};

mg::HandlePool<RuntimeBinding> g_contexts;

RuntimeBinding* findBinding(mg::ContextId id) {
    return g_contexts.get(id);
}

RuntimeBinding* findBindingByCanvasId(const char* canvasId) {
    if (!canvasId || !canvasId[0]) return nullptr;

    for (auto& binding : g_contexts) {
        if (binding.canvasId == canvasId) {
            return &binding;
        }
    }

    return nullptr;
}

}

extern "C" {

EMSCRIPTEN_KEEPALIVE
mg::ContextId mg_runtime_create_context(const char* canvasId) {
    try {
        if (!canvasId || !canvasId[0]) return kInvalidContextId;

        if (auto* existing = findBindingByCanvasId(canvasId)) {
            return existing->bindingId;
        }

        const mg::ContextId bindingId = g_contexts.emplace(canvasId);
        auto* binding = findBinding(bindingId);
        if (!binding || !binding->context || !binding->context->init()) {
            g_contexts.remove(bindingId);
            return kInvalidContextId;
        }

        binding->bindingId = bindingId;
        binding->context->resize(binding->context->width(), binding->context->height());
        binding->runtimeContextId = mg::addContext(*binding->context);
        return bindingId;
    } catch (...) {
        return kInvalidContextId;
    }
}

EMSCRIPTEN_KEEPALIVE
bool mg_runtime_remove_context(mg::ContextId id) {
    try {
        auto* binding = findBinding(id);
        if (!binding) return false;

        if (binding->runtimeContextId != kInvalidContextId) {
            mg::removeContext(binding->runtimeContextId);
        }

        return g_contexts.remove(id);
    } catch (...) {
        return false;
    }
}

EMSCRIPTEN_KEEPALIVE
bool mg_runtime_clear_screen(mg::ContextId id) {
    try {
        auto* binding = findBinding(id);
        return binding ? mg::clearScreen(binding->runtimeContextId) : false;
    } catch (...) {
        return false;
    }
}

EMSCRIPTEN_KEEPALIVE
bool mg_runtime_set_gauge_screen(mg::ContextId id, const char* json) {
    try {
        auto* binding = findBinding(id);
        return binding ? mg::setGaugeScreen(binding->runtimeContextId, json ? std::string(json) : std::string()) : false;
    } catch (...) {
        return false;
    }
}

EMSCRIPTEN_KEEPALIVE
bool mg_runtime_set_render_mode(mg::ContextId id, int mode, int width, int height) {
    try {
        auto* binding = findBinding(id);
        if (!binding || !binding->context) return false;

        if (width > 0 && height > 0) {
            binding->context->resize(width, height);
        }

        binding->context->setRenderMode(static_cast<GraphicsContextCanvas::RenderMode>(mode));
        return true;
    } catch (...) {
        return false;
    }
}

EMSCRIPTEN_KEEPALIVE
bool mg_runtime_set_editor_screen(mg::ContextId id, mg::editor::EditorId editorId, mg::editor::NodeId faceId) {
    try {
        auto* binding = findBinding(id);
        return binding ? mg::setEditorScreen(binding->runtimeContextId, editorId, faceId) : false;
    } catch (...) {
        return false;
    }
}

EMSCRIPTEN_KEEPALIVE
void mg_runtime_frame() {
    mg::frame();
}

}
