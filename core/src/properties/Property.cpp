#include <multigauge/properties/Property.h>

namespace mg {

rapidjson::Value Property::getBaseMeta(rapidjson::Document::AllocatorType& a) const {
    rapidjson::Value metaJson(rapidjson::kObjectType);

    metaJson.AddMember("key", rapidjson::Value(key, a), a);
    metaJson.AddMember("name", rapidjson::Value(meta.name ? meta.name : key, a), a);
    metaJson.AddMember("description", rapidjson::Value(meta.description ? meta.description : "", a), a);
    metaJson.AddMember("widget", rapidjson::Value(meta.widget ? meta.widget : "json", a), a);
    metaJson.AddMember("nullable", meta.nullable, a);
    metaJson.AddMember("inspectorVisible", meta.inspectorVisible, a);
    if (meta.getVisibleWhen) metaJson.AddMember("visibleWhen", meta.getVisibleWhen(a), a);
    if (meta.getInteractableWhen) metaJson.AddMember("interactableWhen", meta.getInteractableWhen(a), a);
    if (meta.getOptionsMeta) metaJson.AddMember("options", meta.getOptionsMeta(a), a);

    return metaJson;
}

} // namespace mg
