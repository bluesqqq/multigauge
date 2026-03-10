#include <multigauge/editor/Property.h>

rapidjson::Value Property::getBaseMeta(rapidjson::Document::AllocatorType& a) const {
    rapidjson::Value meta(rapidjson::kObjectType);

    meta.AddMember("key", rapidjson::Value(key, a), a);
    meta.AddMember("name", rapidjson::Value(name ? name : key, a), a);
    meta.AddMember("description", rapidjson::Value(description ? description : "", a), a);
    meta.AddMember("widget", rapidjson::Value(widget ? widget : "json", a), a);

    return meta;
}