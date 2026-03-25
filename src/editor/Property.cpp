#include <multigauge/editor/Property.h>

rapidjson::Value Property::getBaseMeta(rapidjson::Document::AllocatorType& a) const {
    rapidjson::Value metaJson(rapidjson::kObjectType);

    metaJson.AddMember("key", rapidjson::Value(key, a), a);
    metaJson.AddMember("name", rapidjson::Value(meta.name ? meta.name : key, a), a);
    metaJson.AddMember("description", rapidjson::Value(meta.description ? meta.description : "", a), a);
    metaJson.AddMember("widget", rapidjson::Value(meta.widget ? meta.widget : "json", a), a);
    metaJson.AddMember("category", rapidjson::Value(meta.category ? meta.category : "General", a), a);
    metaJson.AddMember("group", rapidjson::Value(meta.group ? meta.group : "General", a), a);
    if (meta.getOptions) metaJson.AddMember("options", meta.getOptions(a), a);

    return metaJson;
}
