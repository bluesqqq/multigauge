#pragma once

#define MG_EDITOR_NAME(name_literal) \
    public: static constexpr const char* staticTypeName() { return name_literal; } \
    public: const char* typeName() const override { return staticTypeName(); }

#define MG_TYPE_ID(str_literal) \
    public: static constexpr const char* staticTypeId() { return (str_literal); } \
    public: const char* typeId() const override { return staticTypeId(); }

#define MG_PROPS_PARENT(parent_type) \
    public: \
    static ::PropertyObject::PropertyList __mg_parent_property_list(const ::PropertyObject* __mg_obj) { \
        return static_cast<const parent_type*>(__mg_obj)->parent_type::propertyList(); \
    }

#define MG_PROPS_BEGIN() \
public: \
    ::PropertyObject::PropertyList propertyList() const override { \
        using Self = std::remove_cv_t<std::remove_reference_t<decltype(*this)>>; \
        static const ::Property props[] = {

#define MG_PROP(member, key, display_name, description) \
    ::PropertyObject::makeProperty<&Self::member, nullptr>( \
        key, \
        display_name, \
        description, \
        nullptr, \
        nullptr, \
        true \
    ),

#define MG_PROP_CALLBACK(member, key, display_name, description, callback) \
    ::PropertyObject::makeProperty<&Self::member, callback>( \
        key, \
        display_name, \
        description, \
        nullptr, \
        nullptr, \
        true \
    ),

#define MG_PROP_UI(member, key, display_name, description, visible_when, interactable_when) \
    ::PropertyObject::makeProperty<&Self::member, nullptr>( \
        key, \
        display_name, \
        description, \
        visible_when, \
        interactable_when, \
        true \
    ),

#define MG_PROP_CALLBACK_UI(member, key, display_name, description, callback, visible_when, interactable_when) \
    ::PropertyObject::makeProperty<&Self::member, callback>( \
        key, \
        display_name, \
        description, \
        visible_when, \
        interactable_when, \
        true \
    ),

#define MG_PROP_HIDDEN(member, key, display_name, description) \
    ::PropertyObject::makeProperty<&Self::member, nullptr>( \
        key, \
        display_name, \
        description, \
        nullptr, \
        nullptr, \
        false \
    ),

#define MG_PROP_CALLBACK_HIDDEN(member, key, display_name, description, callback) \
    ::PropertyObject::makeProperty<&Self::member, callback>( \
        key, \
        display_name, \
        description, \
        nullptr, \
        nullptr, \
        false \
    ),

#define MG_PROP_UI_HIDDEN(member, key, display_name, description, visible_when, interactable_when) \
    ::PropertyObject::makeProperty<&Self::member, nullptr>( \
        key, \
        display_name, \
        description, \
        visible_when, \
        interactable_when, \
        false \
    ),

#define MG_PROP_CALLBACK_UI_HIDDEN(member, key, display_name, description, callback, visible_when, interactable_when) \
    ::PropertyObject::makeProperty<&Self::member, callback>( \
        key, \
        display_name, \
        description, \
        visible_when, \
        interactable_when, \
        false \
    ),

#define MG_PROP_CUSTOM(key, display_name, description, set_fn, get_fn) \
    ::PropertyObject::makeCustomProperty( \
        key, \
        display_name, \
        description, \
        nullptr, \
        nullptr, \
        true, \
        set_fn, \
        get_fn \
    ),

#define MG_PROP_CUSTOM_UI(key, display_name, description, visible_when, interactable_when, set_fn, get_fn) \
    ::PropertyObject::makeCustomProperty( \
        key, \
        display_name, \
        description, \
        visible_when, \
        interactable_when, \
        true, \
        set_fn, \
        get_fn \
    ),

#define MG_PROPS_END() \
        }; \
        const auto parentGetter = ::MgPropsParentGetter<Self>::value; \
        return { props, sizeof(props) / sizeof(props[0]), parentGetter }; \
    }
