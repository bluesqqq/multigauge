#include <doctest/doctest.h>

#include <multigauge/editor/Editor.h>
#include <multigauge/io/FileSystem.h>
#include <multigauge/json/Json.h>
#include <multigauge/runtime/AssetManager.h>
#include <multigauge/runtime/PackageManager.h>

#include <map>
#include <set>

namespace {

class MemoryFileSystem final : public mg::io::FileSystem {
public:
    bool init() override { return true; }
    bool readBytes(const std::string& path, std::vector<uint8_t>& out) override {
        const auto it = files.find(path);
        if (it == files.end()) return false;
        out = it->second;
        return true;
    }
    bool writeBytesImpl(const std::string& path, const uint8_t* data, size_t length) override {
        files[path] = std::vector<uint8_t>(data, data + length);
        return true;
    }
    bool exists(const std::string& path) override { return files.contains(path) || directories.contains(path); }
    bool size(const std::string& path, size_t& out) override {
        const auto it = files.find(path);
        if (it == files.end()) return false;
        out = it->second.size();
        return true;
    }
    bool remove(const std::string& path) override { return files.erase(path) != 0 || directories.erase(path) != 0; }
    bool rename(const std::string& from, const std::string& to) override {
        const auto it = files.find(from);
        if (it == files.end()) return false;
        files[to] = std::move(it->second);
        files.erase(it);
        return true;
    }
    bool makeDirectory(const std::string& path) override { directories.insert(path); return true; }
    bool listDirectories(const std::string& path, std::vector<std::string>& out) override {
        out.clear();
        const std::string prefix = path + "/";
        for (const auto& directory : directories) {
            if (directory.rfind(prefix, 0) != 0) continue;
            const std::string remainder = directory.substr(prefix.size());
            if (!remainder.empty() && remainder.find('/') == std::string::npos) out.push_back(remainder);
        }
        return true;
    }

    std::map<std::string, std::vector<uint8_t>> files;
    std::set<std::string> directories;
};

} // namespace

TEST_CASE("editor package assets survive save load and history") {
    mg::editor::Editor editor;
    REQUIRE(editor.setAsset({"logo.png", "image/png", "AQID"}));
    REQUIRE(editor.assets().size() == 1);
    const std::string package = editor.exportPackage();
    CHECK(package.find("\"assets\"") != std::string::npos);

    mg::editor::Editor restored;
    REQUIRE(restored.loadPackage(package));
    REQUIRE(restored.assets().size() == 1);
    CHECK(restored.assets().front().name == "logo.png");
    REQUIRE(editor.undo());
    CHECK(editor.assets().empty());
    REQUIRE(editor.redo());
    CHECK(editor.assets().front().data == "AQID");
}

TEST_CASE("editor refuses to remove an asset used by an image element") {
    mg::editor::Editor editor;
    REQUIRE(editor.loadPackage(
        R"({"name":"Package","author":"Author","description":"Test","assets":[{"name":"logo.png","mediaType":"image/png","data":"AQID"}],"faces":[{"name":"Face","face":{"children":[{"type":"image","path":"logo.png"}]}}]})"));
    CHECK_FALSE(editor.removeAsset("logo.png"));
    CHECK(editor.assets().size() == 1);
}

TEST_CASE("package manager installs and exports embedded assets") {
    MemoryFileSystem fs;
    mg::PackageManager packages(fs, "/data");
    const std::string input = R"({"name":"Package","author":"Author","description":"Test","assets":[{"name":"logo.png","mediaType":"image/png","data":"AQID"}],"faces":[{"name":"Face","face":{"children":[]}}]})";

    const mg::Result imported = packages.importPackage(input);
    REQUIRE(imported.ok);
    const auto stored = fs.files.find("/data/packages/package/assets/images/logo.png");
    REQUIRE(stored != fs.files.end());
    CHECK(stored->second == std::vector<uint8_t>{1, 2, 3});

    const mg::Result exported = packages.exportPackage("package");
    REQUIRE(exported.ok);
    const auto asset = exported.data.root().member("assets").element(0);
    std::string_view data;
    REQUIRE(asset.member("data").read(data));
    CHECK(data == "AQID");

    const mg::Result legacyImported = packages.importPackage(
        R"({"name":"Legacy","author":"Author","description":"Test","faces":[{"name":"Face","face":{"children":[]}}]})"
    );
    REQUIRE(legacyImported.ok);
    const mg::Result legacyExported = packages.exportPackage("legacy");
    REQUIRE(legacyExported.ok);
    CHECK(legacyExported.data.root().member("assets").isArray());
    CHECK(legacyExported.data.root().member("assets").size() == 0);
}

TEST_CASE("asset manager resolves assets from an explicit package id") {
    MemoryFileSystem fs;
    mg::AssetManager assets(fs, "/data");

    REQUIRE(assets.writeAsset("package", "logo.png", "AQID"));
    CHECK(fs.files.contains("/data/packages/package/assets/images/logo.png"));
    REQUIRE(assets.writeAsset({}, "preview.png", "AQID"));
    CHECK(fs.files.contains("/assets/images/preview.png"));
    REQUIRE(assets.removeAsset("package", "logo.png"));
    CHECK_FALSE(fs.files.contains("/data/packages/package/assets/images/logo.png"));
}
