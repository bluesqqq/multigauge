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

        /// @brief Returns a JSON string containing the current gauge face as a JSON string.
        std::string saveFace() const;

        //----------[ HIERARCHY ]----------//

        const std::vector<EditorNode>& getNodes() const { return nodes; }

        /// @brief Lists the indexed element hierarchy as a JSON array of editor node.
        std::string listTreeJson() const;

        //----------[ PROPERTIES ]----------//

        /// @brief Returns a JSON string containing an array of property metadata for the given node as JSON
        std::string getPropertiesMetaJson(Id id) const;

        /// @brief Sets a single property value from a JSON string. Returns a JSON string containing the new value of the property after setting, or an error message if failed.
        std::string loadPropertyJson(Id id, const std::string& propName, const std::string& jsonValueText);

        /// @brief Merges a JSON object patch into an object-valued property and writes it back.
        std::string patchPropertyJson(Id id, const std::string& propName, const std::string& patchObjectText);
};
