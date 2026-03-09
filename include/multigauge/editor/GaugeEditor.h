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
        Id faceId = 0;

        std::unordered_map<Id, PropertyObject*> idToPtr;
        std::unordered_map<PropertyObject*, Id> ptrToId;
        std::vector<EditorNode> nodes;

        static std::string toString(const rapidjson::Value& v);

        void indexPropertyObject(PropertyObject& e, Id parentId, std::uint32_t order, const std::string& typeName);

        void indexElementRecursive(Element& e, Id parentId, std::uint32_t order);

        const PropertyObject* find(Id id) const;

        PropertyObject* find(Id id);

        void rebuildIndex();

    public:
        explicit GaugeEditor(GaugeFace& f) { setFace(f); }

        //----------[ GAUGE FACE ]----------//

        void setFace(GaugeFace& f);

        GaugeFace* getFace() const { return face; }

        void loadFace(const std::string& json);

        std::string saveFace(const std::string& json) const;

        //----------[ HIERARCHY ]----------//

        const std::vector<EditorNode>& getNodes() const { return nodes; }

        std::string listTreeJson() const;

        //----------[ PROPERTIES ]----------//

        std::string savePropertiesJson(Id id) const;

        std::string loadPropertyJson(Id id, const std::string& propName, const std::string& jsonValueText);

        std::string patchPropertyJson(Id id, const std::string& propName, const std::string& patchObjectText);
};