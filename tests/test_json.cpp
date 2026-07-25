#include <doctest/doctest.h>

#include <multigauge/json/Json.h>

#include <ostream>
#include <string>

TEST_CASE("JSON document writer builds and copies nested values") {
    mg::json::Document source = mg::json::parse(R"({"name":"source","values":[1,true,null]})");
    REQUIRE(source.valid());

    mg::json::Document output = mg::json::object();
    mg::json::Writer writer = output.writer();
    REQUIRE(writer.writeObject([&](mg::json::ObjectWriter& object) {
        return object.write("copy", source.root()) && object.writeArray("nested", [](mg::json::ArrayWriter& array) {
            return array.writeObject([](mg::json::ObjectWriter& value) { return value.write("id", 7); });
        });
    }));

    const mg::json::Reader root = output.root();
    REQUIRE(root.isObject());
    std::string_view name;
    REQUIRE(root.member("copy").member("name").read(name));
    CHECK(name == "source");
    std::int64_t id = 0;
    REQUIRE(root.member("nested").element(0).member("id").read(id));
    CHECK(id == 7);
}

TEST_CASE("JSON readers have strict types and invalid parses have no root") {
    mg::json::Document document = mg::json::parse(R"({"integer":7,"decimal":2.5,"text":"ok"})");
    REQUIRE(document.valid());
    std::int64_t integer = 0;
    double decimal = 0;
    std::string_view text;
    CHECK(document.root().member("integer").read(integer));
    CHECK_FALSE(document.root().member("integer").read(text));
    CHECK(document.root().member("decimal").read(decimal));
    CHECK(document.root().member("text").read(text));
    CHECK(text == "ok");
    CHECK_FALSE(mg::json::parse("{").valid());
}
