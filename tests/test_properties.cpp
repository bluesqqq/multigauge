#include <doctest/doctest.h>

#include <rapidjson/document.h>

#include <multigauge/properties/PropertyObject.h>

namespace {

struct ChildProperties : mg::PropertyObject {
    int value = 7;

    MG_PROPS_BEGIN()
        MG_PROP(value, "value", "Value", "Test value.")
    MG_PROPS_END()
};

struct ParentProperties : mg::PropertyObject {
    ChildProperties child;

    MG_PROPS_BEGIN()
        MG_PROP(child, "child", "Child", "Test child.")
    MG_PROPS_END()
};

} // namespace

TEST_CASE("property loads report rejected known values and ignore unknown values") {
    rapidjson::Document document;
    document.SetObject();
    auto& allocator = document.GetAllocator();
    document.AddMember("value", "invalid", allocator);
    document.AddMember("future", true, allocator);

    ChildProperties properties;
    CHECK_FALSE(properties.loadProperties(static_cast<const rapidjson::Document&>(document).GetObject()));
    CHECK(properties.value == 7);
}

TEST_CASE("nested property assignment preserves the previous value when decoding fails") {
    rapidjson::Document document;
    document.SetObject();
    auto& allocator = document.GetAllocator();

    rapidjson::Value child(rapidjson::kObjectType);
    child.AddMember("value", "invalid", allocator);
    document.AddMember("child", std::move(child), allocator);

    ParentProperties properties;
    CHECK_FALSE(properties.loadProperties(static_cast<const rapidjson::Document&>(document).GetObject()));
    CHECK(properties.child.value == 7);
}

TEST_CASE("integer codec rejects non-integer JSON numbers") {
    rapidjson::Value number(1.5);
    int value = 0;

    CHECK_FALSE(mg::Codec<int>::decode(number, value));
    CHECK(value == 0);
}

TEST_CASE("property paths preserve empty-segment compatibility without allocating") {
    ParentProperties properties;
    mg::PropertyObject* owner = nullptr;
    const mg::Property* property = nullptr;

    CHECK(properties.resolvePath(".child..value.", owner, property));
    CHECK(owner == &properties.child);
    CHECK(property == properties.child.findProperty("value"));
}
