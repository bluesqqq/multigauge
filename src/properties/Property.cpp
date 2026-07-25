#include <multigauge/properties/Property.h>

namespace mg {

#if MG_ENABLE_EDITOR_REFLECTION
bool Property::writeBaseMeta(json::ObjectWriter& object) const {
    if (!object.write("key", key ? key : "") || !object.write("name", meta.name ? meta.name : key ? key : "") ||
        !object.write("description", meta.description ? meta.description : "") || !object.write("widget", meta.widget ? meta.widget : "json") ||
        !object.write("nullable", meta.nullable) || !object.write("inspectorVisible", meta.inspectorVisible)) return false;
    if (meta.getVisibleWhen && !object.writeValue("visibleWhen", meta.getVisibleWhen)) return false;
    if (meta.getInteractableWhen && !object.writeValue("interactableWhen", meta.getInteractableWhen)) return false;
    return !meta.getOptionsMeta || object.writeValue("options", meta.getOptionsMeta);
}
#endif

} // namespace mg
