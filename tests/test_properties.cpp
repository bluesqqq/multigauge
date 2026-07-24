#include <doctest/doctest.h>

#include <memory>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

#include <rapidjson/document.h>

#include <multigauge/properties/PropertyCodec.h>
#include <multigauge/properties/PropertyObject.h>

namespace property_tests {

enum class Choice { First, Second };

} // namespace property_tests

namespace mg {

template <>
struct EnumTraits<property_tests::Choice> {
    static constexpr EnumOption<property_tests::Choice> options[] = {
        {property_tests::Choice::First, "first", "First"},
        {property_tests::Choice::Second, "second", "Second"},
    };
};

CODEC_BEGIN(property_tests::Choice)
    DECODE() { return decodeEnum(v, out); }
    ENCODE() { return encodeEnum(out, a, v); }
CODEC_END()

} // namespace mg

namespace property_tests {

struct ChildProperties : mg::PropertyObject {
    int value = 7;

    MG_PROPS_BEGIN()
        MG_PROP(value, "value", "Value", "Test value.")
    MG_PROPS_END()
};

struct ParentProperties : mg::PropertyObject {
    ChildProperties child;
    std::optional<ChildProperties> optionalChild;

    MG_PROPS_BEGIN()
        MG_PROP(child, "child", "Child", "Test child.")
        MG_PROP(optionalChild, "optionalChild", "Optional Child", "Optional test child.")
    MG_PROPS_END()
};

struct BaseProperties : mg::PropertyObject {
    int inherited = 3;
    int shared = 4;

    MG_PROPS_BEGIN()
        MG_PROP(inherited, "inherited", "Inherited", "Inherited value.")
        MG_PROP(shared, "shared", "Base Shared", "Base shared value.")
    MG_PROPS_END()
};

struct DerivedProperties : BaseProperties {
    int sharedDerived = 8;

    MG_PROPS_PARENT(BaseProperties)
    MG_PROPS_BEGIN()
        MG_PROP(sharedDerived, "shared", "Derived Shared", "Derived shared value.")
    MG_PROPS_END()
};

struct EditableProperties : mg::PropertyObject {
    int number = 1;
    int custom = 2;
    int callbackCount = 0;
    Choice choice = Choice::First;

    void onNumberChanged() { ++callbackCount; }

    static bool setCustom(mg::PropertyObject* object, const rapidjson::Value& value) {
        auto* self = static_cast<EditableProperties*>(object);
        int decoded = 0;
        if (!mg::decodeAny(value, decoded)) return false;
        self->custom = decoded * 2;
        return true;
    }

    static bool getCustom(const mg::PropertyObject* object, rapidjson::Value& out, rapidjson::Document::AllocatorType&) {
        out.SetInt(static_cast<const EditableProperties*>(object)->custom);
        return true;
    }

    static rapidjson::Value visibleWhen(rapidjson::Document::AllocatorType& a) {
        return MG_UI_RULES(MG_UI_RULE("number", "eq", "1"));
    }

    MG_PROPS_BEGIN()
        MG_PROP_CALLBACK(number, "number", "Number", "Number value.", &EditableProperties::onNumberChanged)
        MG_PROP_CUSTOM("custom", "Custom", "Custom value.", &EditableProperties::setCustom, &EditableProperties::getCustom)
        MG_PROP_UI(choice, "choice", "Choice", "Choice value.", &EditableProperties::visibleWhen, nullptr)
        MG_PROP_HIDDEN(callbackCount, "callbackCount", "Callbacks", "Callback count.")
    MG_PROPS_END()
};

class Animal : public mg::PropertyObject {
public:
    using Registry = mg::MgPolymorphicRegistry<std::unique_ptr<Animal>>;
    static const Registry& registry();
    virtual ~Animal() = default;
};

class Cat final : public Animal {
public:
    MG_EDITOR_NAME("Cat")
    MG_TYPE_ID("cat")

    int age = 0;

    MG_PROPS_BEGIN()
        MG_PROP(age, "age", "Age", "Animal age.")
    MG_PROPS_END()
};

class Dog final : public Animal {
public:
    MG_EDITOR_NAME("Dog")
    MG_TYPE_ID("dog")

    int age = 0;

    MG_PROPS_BEGIN()
        MG_PROP(age, "age", "Age", "Animal age.")
    MG_PROPS_END()
};

using OwnedAnimal = std::unique_ptr<Animal>;

OwnedAnimal createCat() { return std::make_unique<Cat>(); }
OwnedAnimal createDog() { return std::make_unique<Dog>(); }

const Animal::Registry& Animal::registry() {
    static const Registry::Descriptor descriptors[] = {
        mg::makePolymorphicTypeDescriptor<Cat, OwnedAnimal>(&createCat),
        mg::makePolymorphicTypeDescriptor<Dog, OwnedAnimal>(&createDog),
    };
    static const Registry registry(descriptors, sizeof(descriptors) / sizeof(descriptors[0]), &createDog);
    return registry;
}

} // namespace property_tests

namespace mg {

template <>
struct PolymorphicOwnedTraits<property_tests::OwnedAnimal> {
    static constexpr bool supported = true;
    using Base = property_tests::Animal;
};

} // namespace mg

namespace {

using property_tests::Animal;
using property_tests::Cat;
using property_tests::Choice;
using property_tests::ChildProperties;
using property_tests::DerivedProperties;
using property_tests::Dog;
using property_tests::EditableProperties;
using property_tests::OwnedAnimal;
using property_tests::ParentProperties;

} // namespace

TEST_CASE("property lookup and direct set/get use bounded keys") {
    ChildProperties properties;
    rapidjson::Value value(12);
    rapidjson::Document output;
    output.SetObject();
    rapidjson::Value encoded;

    CHECK(properties.findProperty("value") != nullptr);
    CHECK(properties.findProperty("missing") == nullptr);
    CHECK(properties.findProperty(std::string_view("value\0extra", 11)) == nullptr);
    CHECK(properties.setProperty("value", value));
    CHECK(properties.value == 12);
    CHECK_FALSE(properties.setProperty("missing", value));
    CHECK(properties.getProperty("value", encoded, output.GetAllocator()));
    CHECK(encoded.IsInt());
    CHECK(encoded.GetInt() == 12);
    CHECK_FALSE(properties.getProperty("missing", encoded, output.GetAllocator()));
}

TEST_CASE("property loads reject invalid known values and ignore unknown values") {
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

TEST_CASE("property inheritance gives derived properties precedence during lookup and serialization") {
    DerivedProperties properties;
    rapidjson::Document document;
    document.SetObject();

    rapidjson::Value derivedValue(9);
    CHECK(properties.setProperty("shared", derivedValue));
    CHECK(properties.shared == 4);
    CHECK(properties.sharedDerived == 9);

    properties.saveProperties(document, document.GetAllocator());
    CHECK(document.MemberCount() == 2);
    CHECK(document["inherited"].GetInt() == 3);
    CHECK(document["shared"].GetInt() == 9);
}

TEST_CASE("property callbacks and custom accessors run only after successful decoding") {
    EditableProperties properties;
    rapidjson::Value number(5);
    rapidjson::Value invalid("invalid");
    rapidjson::Value custom(4);
    rapidjson::Document document;
    document.SetObject();
    rapidjson::Value encoded;

    CHECK(properties.setProperty("number", number));
    CHECK(properties.number == 5);
    CHECK(properties.callbackCount == 1);
    CHECK_FALSE(properties.setProperty("number", invalid));
    CHECK(properties.number == 5);
    CHECK(properties.callbackCount == 1);
    CHECK(properties.setProperty("custom", custom));
    CHECK(properties.custom == 8);
    CHECK(properties.getProperty("custom", encoded, document.GetAllocator()));
    CHECK(encoded.GetInt() == 8);
}

TEST_CASE("property paths resolve direct and optional children") {
    ParentProperties properties;
    mg::PropertyObject* owner = nullptr;
    const mg::Property* property = nullptr;
    const mg::PropertyObject* constOwner = nullptr;

    CHECK(properties.resolvePath(".child..value.", owner, property));
    CHECK(owner == &properties.child);
    CHECK(property == properties.child.findProperty("value"));
    CHECK(properties.resolvePath("child.value", constOwner, property));
    CHECK(constOwner == &properties.child);
    CHECK_FALSE(properties.resolvePath("optionalChild.value", owner, property));
    CHECK_FALSE(properties.resolvePath("child.missing", owner, property));
    CHECK_FALSE(properties.resolvePath("value.next", owner, property));
}

TEST_CASE("primitive and container codecs preserve values and reject invalid forms") {
    rapidjson::Document document;
    document.SetObject();
    auto& allocator = document.GetAllocator();
    rapidjson::Value encoded;

    int integer = 0;
    rapidjson::Value decimal(1.5);
    CHECK_FALSE(mg::Codec<int>::decode(decimal, integer));
    rapidjson::Value integral(42);
    CHECK(mg::Codec<int>::decode(integral, integer));
    CHECK(integer == 42);

    bool boolean = false;
    rapidjson::Value trueValue(true);
    rapidjson::Value invalidBoolean(1);
    CHECK(mg::Codec<bool>::decode(trueValue, boolean));
    CHECK(boolean);
    CHECK_FALSE(mg::Codec<bool>::decode(invalidBoolean, boolean));

    float floating = 0.0f;
    rapidjson::Value floatingValue(2.5);
    rapidjson::Value invalidFloat("invalid", allocator);
    CHECK(mg::Codec<float>::decode(floatingValue, floating));
    CHECK(floating == doctest::Approx(2.5f));
    CHECK_FALSE(mg::Codec<float>::decode(invalidFloat, floating));

    std::int8_t small = 0;
    rapidjson::Value tooLarge(128);
    CHECK_FALSE(mg::Codec<std::int8_t>::decode(tooLarge, small));

    std::optional<int> optional = 3;
    rapidjson::Value nullValue(rapidjson::kNullType);
    CHECK(mg::Codec<std::optional<int>>::decode(nullValue, optional));
    CHECK_FALSE(optional.has_value());

    rapidjson::Value array(rapidjson::kArrayType);
    array.PushBack(1, allocator).PushBack(2, allocator);
    std::vector<int> values;
    CHECK(mg::Codec<std::vector<int>>::decode(array, values));
    CHECK(values == std::vector<int>{1, 2});
    rapidjson::Value invalidArray(rapidjson::kArrayType);
    invalidArray.PushBack(3, allocator).PushBack("invalid", allocator);
    CHECK_FALSE(mg::Codec<std::vector<int>>::decode(invalidArray, values));
    CHECK(values == std::vector<int>{1, 2});
    CHECK(mg::Codec<std::vector<int>>::encode(encoded, allocator, values));
    CHECK(encoded.IsArray());
    CHECK(encoded.Size() == 2);

    rapidjson::Value text("hello", allocator);
    std::string decoded;
    CHECK(mg::Codec<std::string>::decode(text, decoded));
    CHECK(decoded == "hello");
}

TEST_CASE("enum codecs use JSON string lengths and publish options metadata") {
    rapidjson::Document document;
    document.SetObject();
    auto& allocator = document.GetAllocator();
    Choice choice = Choice::First;
    rapidjson::Value encoded;
    rapidjson::Value valid("second", allocator);
    rapidjson::Value invalid("third", allocator);

    CHECK(mg::decodeEnum(valid, choice));
    CHECK(choice == Choice::Second);
    CHECK_FALSE(mg::decodeEnum(invalid, choice));
    CHECK(mg::encodeEnum(encoded, allocator, Choice::First));
    CHECK(std::string_view(encoded.GetString(), encoded.GetStringLength()) == "first");

    rapidjson::Value options = mg::enumOptionsMeta<Choice>(allocator);
    CHECK(options.IsArray());
    REQUIRE(options.Size() == 2);
    CHECK(std::string_view(options[1]["value"].GetString(), options[1]["value"].GetStringLength()) == "second");
}

TEST_CASE("polymorphic registry finds descriptors, uses fallback, and emits types metadata") {
    const Animal::Registry& registry = Animal::registry();
    rapidjson::Document document;
    document.SetObject();

    CHECK(registry.size() == 2);
    CHECK(registry.find("cat") != nullptr);
    CHECK(registry.find("missing") == nullptr);
    CHECK(std::string_view(registry.create("cat")->typeId()) == "cat");
    CHECK(std::string_view(registry.create("missing")->typeId()) == "dog");

    rapidjson::Value types = registry.getTypesMeta(document.GetAllocator());
    REQUIRE(types.Size() == 2);
    CHECK(std::string_view(types[0]["id"].GetString(), types[0]["id"].GetStringLength()) == "cat");
}

TEST_CASE("polymorphic property decoding stages values and retains schema fallback behavior") {
    rapidjson::Document document;
    document.SetObject();
    auto& allocator = document.GetAllocator();

    rapidjson::Value cat(rapidjson::kObjectType);
    cat.AddMember("type", "cat", allocator);
    cat.AddMember("age", 6, allocator);
    OwnedAnimal animal;
    CHECK(mg::decodeAny(cat, animal));
    REQUIRE(std::string_view(animal->typeId()) == "cat");
    CHECK(static_cast<Cat*>(animal.get())->age == 6);

    rapidjson::Value encoded;
    CHECK(mg::encodeAny(encoded, allocator, animal));
    CHECK(std::string_view(encoded["type"].GetString(), encoded["type"].GetStringLength()) == "cat");
    CHECK(encoded["age"].GetInt() == 6);

    rapidjson::Value unknown(rapidjson::kObjectType);
    unknown.AddMember("type", "future", allocator);
    unknown.AddMember("age", 4, allocator);
    CHECK(mg::decodeAny(unknown, animal));
    REQUIRE(std::string_view(animal->typeId()) == "dog");
    CHECK(static_cast<Dog*>(animal.get())->age == 4);

    OwnedAnimal previous = std::make_unique<Cat>();
    static_cast<Cat*>(previous.get())->age = 9;
    rapidjson::Value invalid(rapidjson::kObjectType);
    invalid.AddMember("type", "cat", allocator);
    invalid.AddMember("age", "invalid", allocator);
    CHECK_FALSE(mg::decodeAny(invalid, previous));
    REQUIRE(std::string_view(previous->typeId()) == "cat");
    CHECK(static_cast<Cat*>(previous.get())->age == 9);
}

TEST_CASE("property metadata and rules describe visible editor properties") {
#if MG_ENABLE_EDITOR_REFLECTION
    EditableProperties properties;
    rapidjson::Document document;
    document.SetObject();

    rapidjson::Value metadata = properties.getPropertiesMeta(document.GetAllocator());
    REQUIRE(metadata.IsArray());
    CHECK(metadata.Size() == 3);
    CHECK(std::string_view(metadata[0]["key"].GetString(), metadata[0]["key"].GetStringLength()) == "number");
    CHECK(metadata[2].HasMember("options"));
    CHECK(metadata[2].HasMember("visibleWhen"));
    CHECK(metadata[2]["visibleWhen"].IsArray());

    ParentProperties parent;
    rapidjson::Value parentMetadata = parent.getPropertiesMeta(document.GetAllocator());
    REQUIRE(parentMetadata[0]["properties"].IsArray());
    CHECK(parentMetadata[0]["properties"].Size() == 1);
#else
    CHECK(true);
#endif
}

TEST_CASE("rule helpers construct scalar, set, and grouped rules") {
    rapidjson::Document document;
    document.SetObject();
    auto& a = document.GetAllocator();

    rapidjson::Value scalar = mg::rules::makeRule(a, "mode", "eq", "manual");
    CHECK(scalar["path"] == "mode");
    CHECK(scalar["op"] == "eq");
    CHECK(scalar["value"] == "manual");

    rapidjson::Value set = mg::rules::makeRule(a, "mode", "in", {"a", "b"});
    REQUIRE(set["value"].IsArray());
    CHECK(set["value"].Size() == 2);

    rapidjson::Value all = mg::rules::makeAllRule(a, {std::move(scalar), std::move(set)});
    REQUIRE(all["all"].IsArray());
    CHECK(all["all"].Size() == 2);
}
