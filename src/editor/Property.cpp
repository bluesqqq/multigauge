#include <multigauge/editor/Property.h>

rapidjson::Value Property::getMeta(const PropertyObject* self, rapidjson::Document::AllocatorType& a) const {
    rapidjson::Value meta(rapidjson::kObjectType);

    meta.AddMember("key", rapidjson::Value(key, a), a);
    meta.AddMember("name", rapidjson::Value(name ? name : key, a), a);
    meta.AddMember("description", rapidjson::Value(description ? description : "", a), a);

    rapidjson::Value value;
    if (get && self && get(self, value, a)) meta.AddMember("value", value, a);
    else meta.AddMember("value", rapidjson::Value().SetNull(), a);
    return meta;
}