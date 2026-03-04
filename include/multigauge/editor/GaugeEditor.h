#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

#include <multigauge/gauge/Element.h>
#include <multigauge/gauge/GaugeFace.h>
#include <multigauge/editor/Editable.h>

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

        std::unordered_map<Id, Editable*> idToPtr;
        std::unordered_map<Editable*, Id> ptrToId;
        std::vector<EditorNode> nodes;

        static std::string toString(const rapidjson::Value& v);

        void indexEditable(Editable& e, Id parentId, std::uint32_t order, const std::string& typeName);

        void indexElementRecursive(Element& e, Id parentId, std::uint32_t order);

        const Editable* find(Id id) const;

        Editable* find(Id id);

        rapidjson::Value exportEditableProps(const Editable& e, rapidjson::Document::AllocatorType& a) const;

        rapidjson::Value exportElementRecursive(const Element& e, rapidjson::Document::AllocatorType& a) const;

    public:
        explicit GaugeEditor(GaugeFace& f) { setFace(f); }

        void setFace(GaugeFace& f);

        GaugeFace* getFace() const { return face; }

        void rebuildIndex();

        // -----------------------------
        // Tree for hierarchy panel
        // -----------------------------

        const std::vector<EditorNode>& getNodes() const { return nodes; }

        std::string listTreeJson() const;

        // -----------------------------
        // Properties for inspector
        // -----------------------------

        std::string savePropertiesJson(Id id) const;

        std::string loadPropertyJson(Id id, const std::string& propName, const std::string& jsonValueText);

        std::string patchPropertyJson(Id id, const std::string& propName, const std::string& patchObjectText);

        std::string exportFaceJson() const;
};