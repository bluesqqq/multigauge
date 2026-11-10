#include <doctest/doctest.h>
#include "MemoryFileSystem.h"

namespace {
class ReportedSizeFileSystem : public mg::test::MemoryFileSystem {
public:
    size_t reportedSize = 0;
    int opens = 0;
    bool failSize = false;
    bool size(const std::string&, size_t& out) override {
        if (failSize) return false;
        out = reportedSize;
        return true;
    }
protected:
    std::unique_ptr<mg::io::File> openImpl(const std::string& path, mg::io::FileMode mode) override {
        ++opens;
        return MemoryFileSystem::openImpl(path, mode);
    }
};
}

TEST_CASE("whole-file limits reject oversized input before opening and preserve output") {
    ReportedSizeFileSystem fs;
    fs.files["data"] = {1, 2, 3};
    fs.reportedSize = 3;
    std::vector<uint8_t> bytes{99};
    std::string text = "unchanged";
    CHECK_FALSE(fs.readBytes("data", bytes, 2));
    CHECK_FALSE(fs.readText("data", text, 0));
    CHECK(fs.opens == 0);
    CHECK(bytes == std::vector<uint8_t>{99});
    CHECK(text == "unchanged");
    fs.failSize = true;
    CHECK_FALSE(fs.readBytes("data", bytes, 10));
    CHECK(fs.opens == 0);
    CHECK(bytes == std::vector<uint8_t>{99});
}

TEST_CASE("whole-file limits also apply when files grow after the size check") {
    ReportedSizeFileSystem fs;
    fs.files["data"] = std::vector<uint8_t>(1100, 7);
    fs.reportedSize = 1;
    std::vector<uint8_t> bytes{99};
    std::string text = "unchanged";
    CHECK_FALSE(fs.readBytes("data", bytes, 1024));
    CHECK(bytes == std::vector<uint8_t>{99});
    CHECK_FALSE(fs.readText("data", text, 1024));
    CHECK(text == "unchanged");
    REQUIRE(fs.readBytes("data", bytes, 1100));
    CHECK(bytes == fs.files["data"]);
    fs.files["data"].clear();
    fs.reportedSize = 0;
    REQUIRE(fs.readBytes("data", bytes, 0));
    CHECK(bytes.empty());
    fs.files["data"] = {7};
    CHECK_FALSE(fs.readBytes("data", bytes, 0));
    CHECK(bytes.empty());
}

TEST_CASE("whole-file helpers use streams without truncating reads") {
    mg::test::MemoryFileSystem fs;
    const std::string content(1300, 'x');
    REQUIRE(fs.writeText("logs/session/data", content));
    std::string text;
    REQUIRE(fs.readText("logs/session/data", text, content.size()));
    CHECK(text == content);
    CHECK(fs.files.at("logs/session/data").size() == content.size());
    CHECK(fs.directories.contains("logs/session"));
    REQUIRE(fs.writeText("logs/empty", ""));
    REQUIRE(fs.readText("logs/empty", text, 0));
    CHECK(text.empty());
    std::vector<uint8_t> bytes{42};
    CHECK_FALSE(fs.readBytes("missing/data", bytes, 100));
    CHECK(bytes == std::vector<uint8_t>{42});
    CHECK_FALSE(fs.directories.contains("missing"));
    CHECK_FALSE(fs.writeBytes("logs/session/data", nullptr, 1));
    CHECK(fs.files.at("logs/session/data").size() == content.size());
    fs.failDirectory = true;
    CHECK_FALSE(fs.open("other/file", mg::io::FileMode::Write));
    CHECK_FALSE(fs.open("invalid/file", static_cast<mg::io::FileMode>(99)));
    CHECK_FALSE(fs.directories.contains("invalid"));
}

TEST_CASE("streams expose positions and separate file and directory listings") {
    mg::test::MemoryFileSystem fs;
    REQUIRE(fs.writeText("logs/session/data", "abc"));
    auto file = fs.open("logs/session/data", mg::io::FileMode::Append);
    REQUIRE(file);
    REQUIRE(file->seek(0));
    size_t position = 99;
    REQUIRE(file->tell(position));
    CHECK(position == 0);
    REQUIRE(file->write(reinterpret_cast<const uint8_t*>("d"), 1));
    REQUIRE(file->tell(position));
    CHECK(position == 4);
    REQUIRE(file->close());
    CHECK_FALSE(file->tell(position));
    CHECK(position == 4);
    std::vector<std::string> names;
    REQUIRE(fs.listDirectories("logs", names));
    CHECK(names == std::vector<std::string>{"session"});
    REQUIRE(fs.listFiles("logs/session", names));
    CHECK(names == std::vector<std::string>{"data"});
    CHECK_FALSE(fs.listFiles("missing", names));
    CHECK(names.empty());
}
