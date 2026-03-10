#include "Property.h"

rapidjson::Value Property::getMeta(rapidjson::Document::AllocatorType& a) const {
    rapidjson::Value meta(rapidjson::kObjectType);
    meta.AddMember("key", rapidjson::Value(key, a), a);
    meta.AddMember("name", rapidjson::Value(name ? name : key, a), a);
    meta.AddMember("description", rapidjson::Value(description ? description : "", a), a);
    return meta;
}