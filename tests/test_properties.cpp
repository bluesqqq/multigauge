#include <doctest/doctest.h>

#include <multigauge/properties/PropertyCodec.h>
#include <multigauge/properties/PropertyObject.h>

#include <optional>
#include <ostream>

namespace {

struct Child final : mg::PropertyObject {
    int value = 7;
    MG_PROPS_BEGIN()
        MG_PROP(value, "value", "Value", "Test value.")
    MG_PROPS_END()
};

struct Parent final : mg::PropertyObject {
    Child child;
    std::optional<Child> optionalChild;
    MG_PROPS_BEGIN()
        MG_PROP(child, "child", "Child", "Test child.")
        MG_PROP(optionalChild, "optionalChild", "Optional child", "Optional test child.")
    MG_PROPS_END()
};

struct TypedChild final : mg::PropertyObject {
    int value = 11;
    MG_TYPE_ID("test-child")
    MG_PROPS_BEGIN()
        MG_PROP(value, "value", "Value", "Test value.")
    MG_PROPS_END()
};

static_assert(mg::CodecFor<int>);
static_assert(mg::PropertyObjectValue<Child>);

mg::json::Document objectWithValue(std::string_view json) {
    return mg::json::parse(json);
}

}

TEST_CASE("properties reject invalid known values and preserve nested state") {
    Child child;
    const auto invalid = objectWithValue(R"({"value":"invalid","future":true})");
    CHECK_FALSE(child.loadProperties(invalid.root()));
    CHECK(child.value == 7);

    Parent parent;
    const auto invalidNested = objectWithValue(R"({"child":{"value":"invalid"}})");
    CHECK_FALSE(parent.loadProperties(invalidNested.root()));
    CHECK(parent.child.value == 7);
}

TEST_CASE("properties serialize, resolve paths, and represent nullable children") {
    Parent parent;
    mg::json::Document encoded = mg::json::object();
    mg::json::Writer writer = encoded.writer();
    REQUIRE(parent.saveProperties(writer));
    REQUIRE(encoded.root().member("child").isObject());
    std::int64_t value = 0;
    REQUIRE(encoded.root().member("child").member("value").read(value));
    CHECK(value == 7);

    mg::PropertyObject* owner = nullptr;
    const mg::Property* property = nullptr;
    REQUIRE(parent.resolvePath(".child..value.", owner, property));
    CHECK(owner == &parent.child);
    CHECK(property == parent.child.findProperty("value"));
    CHECK_FALSE(parent.resolvePath("optionalChild.value", owner, property));

    const Parent& constParent = parent;
    const mg::PropertyObject* constOwner = nullptr;
    REQUIRE(constParent.resolvePath("child.value", constOwner, property));
    CHECK(constOwner == &parent.child);
}

TEST_CASE("properties serialize members into an existing object") {
    TypedChild child;
    mg::json::Document encoded = mg::json::object();
    mg::json::Writer writer = encoded.writer();
    REQUIRE(writer.writeObject([&](mg::json::ObjectWriter& object) {
        return object.write("owner", "container") && child.savePropertyMembers(object);
    }));

    std::string_view type;
    std::string_view owner;
    std::int64_t value = 0;
    REQUIRE(encoded.root().member("type").read(type));
    REQUIRE(encoded.root().member("owner").read(owner));
    REQUIRE(encoded.root().member("value").read(value));
    CHECK(type == "test-child");
    CHECK(owner == "container");
    CHECK(value == 11);
}

TEST_CASE("property metadata respects the reflection configuration") {
    Child child;
    mg::json::Document metadata = mg::json::array();
    mg::json::Writer writer = metadata.writer();
    REQUIRE(child.writePropertiesMeta(writer));
#if MG_ENABLE_EDITOR_REFLECTION
    CHECK(metadata.root().size() == 1);
    std::string_view key;
    REQUIRE(metadata.root().element(0).member("key").read(key));
    CHECK(key == "value");
#else
    CHECK(metadata.root().size() == 0);
#endif
}

TEST_CASE("property metadata writes nested children directly into their group") {
    Parent parent;
    mg::json::Document metadata = mg::json::array();
    mg::json::Writer writer = metadata.writer();
    REQUIRE(parent.writePropertiesMeta(writer));

#if MG_ENABLE_EDITOR_REFLECTION
    const mg::json::Reader child = metadata.root().element(0);
    REQUIRE(child.isObject());
    REQUIRE(child.member("properties").isArray());
    CHECK(child.member("properties").size() == 1);
    CHECK(child.member("properties").element(0).isObject());

    std::string_view key;
    REQUIRE(child.member("properties").element(0).member("key").read(key));
    CHECK(key == "value");
#else
    CHECK(metadata.root().size() == 0);
#endif
}
