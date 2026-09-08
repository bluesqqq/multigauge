#include <multigauge/properties/Property.h>

namespace mg {

#if MG_BUILD_EDITOR
bool Property::writeBaseMeta(json::ObjectWriter& object) const {
    if (!object.write("key", key ? key : "") || !object.write("nullable", meta.nullable)) return false;
    return !meta.getOptions || object.writeValue("options", meta.getOptions);
}
#endif

} // namespace mg
