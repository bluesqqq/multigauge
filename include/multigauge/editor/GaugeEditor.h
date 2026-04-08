#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

#include <multigauge/gauge/Element.h>
#include <multigauge/gauge/GaugeFace.h>
struct EditorNode {
    std::uint32_t id = 0;
    std::uint32_t parentId = 0;
    std::uint32_t order = 0;
    std::string type;
};

struct EditorResult {
    bool ok = false;
    rapidjson::Document data;
    std::string error;

    std::string toJson() const {
        rapidjson::Document d;
        d.SetObject();
        auto& a = d.GetAllocator();

        d.AddMember("ok", ok, a);

        if (ok) {
            rapidjson::Value dataCopy;
            dataCopy.CopyFrom(data, a);
            d.AddMember("data", std::move(dataCopy), a);
        } else {
            d.AddMember("error", rapidjson::Value(error.c_str(), a), a);
        }

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        d.Accept(writer);
        return buffer.GetString();
    }

    static EditorResult OkObject() {
        EditorResult r;
        r.ok = true;
        r.data.SetObject();
        return r;
    }

    static EditorResult OkArray() {
        EditorResult r;
        r.ok = true;
        r.data.SetArray();
        return r;
    }

    static EditorResult Error(const std::string& error) {
        EditorResult r;
        r.ok = false;
        r.error = error;
        return r;
    }
};

inline EditorResult OkObject() {
    return EditorResult::OkObject();
}

inline EditorResult OkArray() {
    return EditorResult::OkArray();
}

inline EditorResult Error(const std::string& error) {
    return EditorResult::Error(error);
}

class GaugeEditor {
    private:
        using Id = std::uint32_t;
        static constexpr std::size_t kMaxHistoryEntries = 100;

        GaugeFace* face = nullptr;
        Id nextId = 1;
        std::string clipboardJson;
        std::vector<std::string> undoHistory;
        std::vector<std::string> redoHistory;

        std::unordered_map<Id, Element*> idToPtr;
        std::unordered_map<const Element*, Id> ptrToId;
        std::vector<EditorNode> nodes;

        static std::string toString(const rapidjson::Value& v);

        void indexElementRecursive(Element& e, Id parentId, std::uint32_t order);

        const EditorNode* findNode(Id id) const;
        const Element* find(Id id) const;
        Element* find(Id id);

        void clearHistory();
        void loadFaceInternal(const std::string& json, bool resetHistory);
        void pushHistorySnapshot(std::vector<std::string>& stack, const std::string& snapshot);
        void commitMutationSnapshot(const std::string& previousJson);
        void rebuildIndex();

    public:
        explicit GaugeEditor(GaugeFace& f) { setFace(f); }

        //----------[ DOCUMENT ]----------//

        void setFace(GaugeFace& f);
        GaugeFace* getFace() const { return face; }

        /// @brief Loads the gauge face from a JSON string, replacing all existing data.
        void loadFace(const std::string& json);

        /// @brief Returns the current gauge face serialized as JSON.
        std::string saveFace() const;
        EditorResult getHistoryState() const;
        EditorResult undo();
        EditorResult redo();

        //----------[ HIERARCHY QUERIES ]----------//

        const std::vector<EditorNode>& getNodes() const { return nodes; }

        /// @brief Lists the indexed element hierarchy as JSON.
        EditorResult listTreeJson() const;
        /// @brief Lists all exposed element types. 
        EditorResult listElements() const;
        /// @brief Lists all exposed values.
        EditorResult listValues() const;

        //----------[ HIERARCHY MUTATIONS ]----------//

        /// @brief Returns the full serialized JSON for the given element id.
        EditorResult getElementJson(Id id) const;

        /// @brief Adds an element from JSON.
        EditorResult addElementJson(Id parentId, const std::string& elementJsonText);
        /// @brief Inserts an element from JSON at the specified index. 
        EditorResult insertElementJson(Id parentId, const std::string& elementJsonText, int index);
        /// @brief Moves an element to a different (or same) parent at the specified index. 
        EditorResult moveElement(Id id, Id newParentId, int index);
        /// @brief Removes an element from the gauge face. 
        EditorResult removeElement(Id id);
        /// @brief Replaces an element with new JSON.
        EditorResult replaceElementJson(Id id, const std::string& json);

        //----------[ ORDERING ]----------//

        /// @brief Moves an element one index forward.
        EditorResult bringForward(Id id);
        /// @brief Moves an element to the very front.
        EditorResult bringToFront(Id id);
        /// @brief Moves an element one index backward.
        EditorResult sendBackward(Id id);
        /// @brief Moves an element to the very back.
        EditorResult sendToBack(Id id);
        /// @brief Moves an element to the specified index. 
        EditorResult reorderElement(Id id, int newIndex);

        //----------[ COPYING ]----------//

        EditorResult copyElement(Id id);
        EditorResult pasteIntoElement(Id id);
        EditorResult pasteToReplaceElement(Id id);
        EditorResult duplicateElement(Id id);

        //----------[ GEOMETRY ]----------//

        /// @brief Returns the bounds of the specified element. 
        EditorResult getElementBoundsJson(Id id) const;
        /// @brief Lists the bounds of all elements in the gauge face. 
        EditorResult listElementBoundsJson() const;

        /// @brief Returns the ID of the element at the specified location.
        Id hitTest(float x, float y, int index = 0) const;
        /// @brief Returns a list of all elements at the specified location. 
        EditorResult hitTestAll(float x, float y) const;

        //----------[ PROPERTIES ]----------//

        /// @brief Returns the top-level property inspector metadata for the given element id.
        EditorResult getPropertiesMetaJson(Id id) const;
        /// @brief Returns the inspector node metadata for a nested property path on the given element id.
        EditorResult getPropertiesMetaJson(Id id, const std::string& path) const;
        /// @brief Sets a property by path using a JSON value string.
        EditorResult setPropertyJson(Id id, const std::string& path, const std::string& jsonValueText);
};
