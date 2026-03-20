#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

#include <multigauge/gauge/Element.h>
#include <multigauge/gauge/GaugeFace.h>
#include <multigauge/editor/PropertyObject.h>

struct EditorNode {
    std::uint32_t id = 0;
    std::uint32_t parentId = 0;
    std::uint32_t order = 0;
    std::string type;
};

class GaugeEditor {
    private:
        using Id = std::uint32_t;

        GaugeFace* face = nullptr;
        Id nextId = 1;

        std::unordered_map<Id, PropertyObject*> idToPtr;
        std::unordered_map<PropertyObject*, Id> ptrToId;
        std::vector<EditorNode> nodes;

        static std::string toString(const rapidjson::Value& v);

        void indexElementRecursive(Element& e, Id parentId, std::uint32_t order);

        const EditorNode* findNode(Id id) const;
        const PropertyObject* find(Id id) const;
        PropertyObject* find(Id id);

        void rebuildIndex();

    public:
        explicit GaugeEditor(GaugeFace& f) { setFace(f); }

        //----------[ GAUGE FACE ]----------//

        void setFace(GaugeFace& f);
        GaugeFace* getFace() const { return face; }

        /// @brief Loads the gauge face from a JSON string, replacing all existing data.
        void loadFace(const std::string& json);

        /// @brief Returns the current gauge face serialized as JSON.
        std::string saveFace() const;

        //----------[ HIERARCHY ]----------//

        const std::vector<EditorNode>& getNodes() const { return nodes; }

        /// @brief Lists the indexed element hierarchy as JSON.
        std::string listTreeJson() const;

        std::string listElements() const;

        std::string addElement(Id parentId, const std::string& type);
        std::string insertElement(Id parentId, const std::string& type, int index);
        std::string moveElement(Id id, Id newParentId, int index);
        std::string removeElement(Id id);

        //----------[ PROPERTIES ]----------//

        /// @brief Returns the top-level property inspector metadata for the given element id.
        std::string getPropertiesMetaJson(Id id) const;

        /// @brief Returns the inspector node metadata for a nested property path on the given element id.
        std::string getPropertiesMetaJson(Id id, const std::string& path) const;

        /// @brief Sets a property by path using a JSON value string.
        std::string setPropertyJson(Id id, const std::string& path, const std::string& jsonValueText);
};
