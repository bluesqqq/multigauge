#include <doctest/doctest.h>

#include <multigauge/graphics/UserPalette.h>
#include <multigauge/io/FileSystem.h>
#include <multigauge/settings/Manager.h>

#include <map>
#include <set>
#include <vector>

namespace {

class MemoryFileSystem final : public mg::io::FileSystem {
public:
    bool init() override { return true; }
    bool readBytes(const std::string& path, std::vector<std::uint8_t>& out) override {
        const auto found = files_.find(path);
        if (found == files_.end()) return false;
        out = found->second;
        return true;
    }
    bool exists(const std::string& path) override { return files_.contains(path) || directories_.contains(path); }
    bool size(const std::string& path, size_t& out) override {
        const auto found = files_.find(path);
        if (found == files_.end()) return false;
        out = found->second.size();
        return true;
    }
    bool remove(const std::string& path) override { return files_.erase(path) != 0 || directories_.erase(path) != 0; }
    bool rename(const std::string& from, const std::string& to) override {
        const auto found = files_.find(from);
        if (found == files_.end()) return false;
        files_[to] = std::move(found->second);
        files_.erase(found);
        return true;
    }
    bool makeDirectory(const std::string& path) override { directories_.insert(path); return true; }
    bool listDirectories(const std::string&, std::vector<std::string>& out) override { out.clear(); return true; }

protected:
    bool writeBytesImpl(const std::string& path, const std::uint8_t* data, size_t length) override {
        files_[path] = std::vector<std::uint8_t>(data, data + length);
        return true;
    }

private:
    std::map<std::string, std::vector<std::uint8_t>> files_;
    std::set<std::string> directories_;
};

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
