#include <multigauge/properties/meta/Inspector.h>

#include <multigauge/properties/PropertyObject.h>

#if MG_BUILD_EDITOR
namespace mg::inspector {

bool Builder::claim(const char* path) {
    if (!path || !*path || bindings_.contains(path)) return false;

    const PropertyObject* owner = nullptr;
    const Property* property = nullptr;
    if (!object_.resolvePath(path, owner, property) || !owner || !property) return false;

    bindings_.emplace(path);
    return true;
}

bool Builder::property(
    const char* path, 
    PropertyMetadata::RuleListGetter visibleWhen
) {
    if (!claim(path)) return false;

    return nodes_.writeObject([&](json::ObjectWriter& object) {
        if (!object.write("type", "property") || !object.write("path", path)) return false;
        return !visibleWhen || object.writeValue("visibleWhen", visibleWhen);
    });
}

bool Builder::control(
    const char* widget,
    std::initializer_list<Binding> bindings,
    PropertyMetadata::RuleListGetter visibleWhen,
    std::initializer_list<Option> options
) {
    if (!widget || !*widget || bindings.size() == 0) return false;

    for (const Binding& binding : bindings)
        if (!binding.name || !*binding.name || !claim(binding.path)) return false;
    
    return nodes_.writeObject([&](json::ObjectWriter& object) {
        if (
            !object.write("type", "control") ||
            !object.write("widget", widget) ||
            !object.writeObject("bindings", [&](json::ObjectWriter& output) {
                for (const Binding& binding : bindings)
                    if (!output.write(binding.name, binding.path)) return false;

                return true;
            })
        ) return false;

        if (visibleWhen && !object.writeValue("visibleWhen", visibleWhen)) return false;

        if (options.size() == 0) return true;

        return object.writeObject("options", [&](json::ObjectWriter& output) {
            for (const Option& option : options)
                if (!option.key || !option.value || !output.write(option.key, option.value)) return false;

            return true;
        });
    });
}

} // namespace mg::inspector
#endif
