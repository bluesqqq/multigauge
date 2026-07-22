#include <doctest/doctest.h>

#include <array>
#include <ostream>
#include <rapidjson/document.h>

#include <multigauge/text/EmbeddedText.h>

TEST_CASE("EmbeddedText preserves source and renders into fixed storage") {
    mg::text::EmbeddedText text("RPM: {engineRPM.0}");
    CHECK(text.hasEmbeds());
    CHECK(text.sourceView() == "RPM: {engineRPM.0}");

    std::array<char, 64> storage{};
    mg::text::TextBuffer output(storage.data(), storage.size());
    REQUIRE(text.render(output));
    CHECK(output.view() == "RPM: 0rpm");
}

TEST_CASE("EmbeddedText leaves ordinary strings unchanged") {
    mg::text::EmbeddedText text("static text");
    CHECK_FALSE(text.hasEmbeds());

    std::array<char, 32> storage{};
    mg::text::TextBuffer output(storage.data(), storage.size());
    REQUIRE(text.render(output));
    CHECK(output.view() == "static text");
}

TEST_CASE("EmbeddedText codec remains a JSON string codec") {
    rapidjson::Document document;
    document.SetObject();
    auto& allocator = document.GetAllocator();

    mg::text::EmbeddedText text;
    rapidjson::Value source("Value: {engineRPM}", allocator);
    REQUIRE(mg::Codec<mg::text::EmbeddedText>::decode(source, text));
    CHECK(text.source() == "Value: {engineRPM}");

    rapidjson::Value encoded;
    REQUIRE(mg::Codec<mg::text::EmbeddedText>::encode(encoded, allocator, text));
    REQUIRE(encoded.IsString());
    CHECK(std::string(encoded.GetString(), encoded.GetStringLength()) == "Value: {engineRPM}");
}

TEST_CASE("EmbeddedText reports fixed-buffer overflow without partial output") {
    mg::text::EmbeddedText text("prefix {engineRPM} suffix");
    std::array<char, 8> storage{};
    mg::text::TextBuffer output(storage.data(), storage.size());

    CHECK_FALSE(text.render(output));
    CHECK_FALSE(output.ok());
}
