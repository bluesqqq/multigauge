#include <doctest/doctest.h>

#include <multigauge/graphics/UserPalette.h>
#include "MemoryFileSystem.h"
#include <multigauge/settings/Manager.h>

#include <map>
#include <set>
#include <vector>

namespace {

using mg::test::MemoryFileSystem;

TEST_CASE("settings persist the active face and shared palette") {
    MemoryFileSystem fs;
    mg::graphics::UserPalette palette;
    mg::settings::Manager settings(fs, "/multigauge");

    REQUIRE(settings.load(palette));
    settings.setActiveFace("daily-driver", "tachometer");
    REQUIRE(settings.setUserColor(1, mg::graphics::rgba{1, 2, 3, 4}));
    REQUIRE(settings.save());

    mg::graphics::UserPalette restoredPalette;
    mg::settings::Manager restored(fs, "/multigauge");
    REQUIRE(restored.load(restoredPalette));
    REQUIRE(restored.activeFace());
    CHECK(restored.activeFace()->packageId == "daily-driver");
    CHECK(restored.activeFace()->faceId == "tachometer");
    const auto color = restoredPalette.color(1);
    CHECK(color.r == 1);
    CHECK(color.g == 2);
    CHECK(color.b == 3);
    CHECK(color.a == 4);
}

TEST_CASE("settings reject malformed documents without changing the palette") {
    MemoryFileSystem fs;
    const std::string malformed = R"({"version":1,"userColors":["#FFFFFFFF"]})";
    REQUIRE(fs.writeBytes("/multigauge/settings.json", reinterpret_cast<const std::uint8_t*>(malformed.data()), malformed.size()));

    mg::graphics::UserPalette palette;
    mg::settings::Manager settings(fs, "/multigauge");
    CHECK_FALSE(settings.load(palette));
    const auto color = palette.color(1);
    CHECK(color.r == 255);
    CHECK(color.g == 0);
    CHECK(color.b == 0);
    CHECK(color.a == 255);
}

} // namespace
