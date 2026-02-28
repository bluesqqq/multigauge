#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

#include <multigauge/gauge/Element.h>

struct EditorNode {
    std::uint32_t id = 0;
    std::uint32_t parentId = 0;
    std::uint32_t order = 0;
    std::string type;
};

class GaugeEditor {
    private:
        using Id = std::uint32_t;

        Element* root = nullptr;
        Id nextId = 1;

        std::unordered_map<Id, Element*> idToPtr;
        std::unordered_map<Element*, Id> ptrToId;
        std::vector<EditorNode> nodes;

        static Element* findRoot(Element*n) {
            if (!n) return nullptr;
            return n->getRoot();
        }

        void indexRecursive(Element& e, Id parentId, std::uint32_t order) {
            const Id id = nextId++;
            idToPtr[id] = &e;
            ptrToId[&e] = id;

            EditorNode node;
            node.id = id;
            node.parentId = parentId;
            node.order = order;
            node.type = "test";

            nodes.push_back(std::move(node));

            const std::size_t count = e.childCount();
            for (std::size_t i = 0; i < count; ++i) {
                Element* c = e.childAt(i);
                if (!c) continue;
                indexRecursive(*c, id, static_cast<std::uint32_t>(i));
            }
        }

        const Element* find(int id) const {
            auto it = idToPtr.find(id);
            return it == idToPtr.end() ? nullptr : it->second;
        }

        Element* findMutable(int id) {
            auto it = idToPtr.find(id);
            return it == idToPtr.end() ? nullptr : it->second;
        }

        static std::string toString(const rapidjson::Value& v) {
            rapidjson::StringBuffer sb;
            rapidjson::Writer<rapidjson::StringBuffer> w(sb);
            v.Accept(w);
            return std::string(sb.GetString(), sb.GetSize());
        }

    public:
        explicit GaugeEditor(Element& r) {
            setRoot(r);
        }

        void setRoot(Element& r) {
            root = findRoot(&r);
            rebuildIndex();
        }

        Element* getRoot() const { return root; }

        void rebuildIndex() {
            nextId = 1;
            idToPtr.clear();
            ptrToId.clear();
            nodes.clear();

            if (!root) return;

            // Ensure root is truly root
            root = findRoot(root);

            indexRecursive(*root, /*parentId*/0, /*order*/0);
        }

        // -----------------------------
        // Tree for hierarchy panel
        // -----------------------------

        const std::vector<EditorNode>& getNodes() const { return nodes; }

        std::string listTreeJson() const {
            rapidjson::Document d;
            d.SetArray();
            auto& a = d.GetAllocator();

            for (const auto& n : nodes) {
                rapidjson::Value obj(rapidjson::kObjectType);
                obj.AddMember("id", n.id, a);
                obj.AddMember("parentId", n.parentId, a);
                obj.AddMember("order", n.order, a);

                rapidjson::Value t;
                t.SetString(n.type.c_str(), static_cast<rapidjson::SizeType>(n.type.size()), a);
                obj.AddMember("type", t, a);

                d.PushBack(obj, a);
            }

            return toString(d);
        }

        // -----------------------------
        // Properties for inspector
        // -----------------------------

        std::string getPropertiesJson(Id id) const {
            const Element* e = find(id);
            if (!e) return R"({"ok":false,"error":"NotFound"})";

            rapidjson::Document d;
            d.SetObject();
            auto& a = d.GetAllocator();

            rapidjson::Value props;
            e->saveProperties(props, a);

            d.AddMember("ok", true, a);
            d.AddMember("properties", props, a);
            return toString(d);
        }

        std::string setPropertyJson(Id id, const std::string& propName, const std::string& jsonValueText) {
            Element* e = findMutable(id);
            if (!e) return R"({"ok":false,"error":"NotFound"})";

            const Editable::Property* p = e->findProperty(propName.c_str());
            if (!p || !p->set) return R"({"ok":false,"error":"UnknownProperty"})";

            rapidjson::Document v;
            if (v.Parse(jsonValueText.c_str()).HasParseError()) {
                return R"({"ok":false,"error":"BadJson"})";
            }

            if (!p->set(e, v)) {
                return R"({"ok":false,"error":"TypeMismatch"})";
            }

            return R"({"ok":true})";
        }

        std::string patchPropertyJson(Id id, const std::string& propName, const std::string& patchObjectText) {
            Element* e = findMutable(id);
            if (!e) return R"({"ok":false,"error":"NotFound"})";

            const Editable::Property* p = e->findProperty(propName.c_str());
            if (!p || !p->get || !p->set) return R"({"ok":false,"error":"UnknownProperty"})";

            rapidjson::Document d;
            d.SetObject();
            auto& a = d.GetAllocator();

            rapidjson::Value current;
            if (!p->get(e, current, a)) return R"({"ok":false,"error":"GetFailed"})";
            if (!current.IsObject()) return R"({"ok":false,"error":"NotObject"})";

            rapidjson::Document patch;
            if (patch.Parse(patchObjectText.c_str()).HasParseError()) {
                return R"({"ok":false,"error":"BadJson"})";
            }
            if (!patch.IsObject()) return R"({"ok":false,"error":"PatchNotObject"})";

            for (auto it = patch.MemberBegin(); it != patch.MemberEnd(); ++it) {
                rapidjson::Value name;
                name.SetString(it->name.GetString(), it->name.GetStringLength(), a);

                rapidjson::Value val;
                val.CopyFrom(it->value, a);

                auto existing = current.FindMember(it->name);
                if (existing != current.MemberEnd()) {
                    existing->value = std::move(val);
                } else {
                    current.AddMember(name, val, a);
                }
            }

            if (!p->set(e, current)) {
                return R"({"ok":false,"error":"TypeMismatch"})";
            }

            return R"({"ok":true})";
        }
};