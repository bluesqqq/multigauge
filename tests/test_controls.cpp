#include <doctest/doctest.h>

#include <multigauge/control/Manager.h>
#include <multigauge/Config.h>
#if MG_BUILD_EDITOR
#include <multigauge/editor/Manager.h>
#endif
#include <multigauge/graphics/GraphicsContext.h>
#include <multigauge/graphics/UserPalette.h>
#include <multigauge/io/FileSystem.h>
#include <multigauge/navigation/Manager.h>
#include <multigauge/package/Manager.h>
#include <multigauge/screens/GaugeScreen.h>
#include <multigauge/screens/Screen.h>

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

class NullGraphicsContext final : public mg::graphics::GraphicsContext {
public:
    void clear(mg::graphics::rgba) override {}
    void pixel(int, int, mg::graphics::rgba) override {}
    void line(int, int, int, int, mg::graphics::rgba, float) override {}
    void rect(int, int, int, int, mg::graphics::rgba) override {}
    void strokeRect(int, int, int, int, mg::graphics::rgba, float) override {}
    void roundRect(int, int, int, int, float, mg::graphics::rgba) override {}
    void roundRect(int, int, int, int, float, float, float, float, mg::graphics::rgba) override {}
    void strokeRoundRect(int, int, int, int, float, mg::graphics::rgba, float) override {}
    void strokeRoundRect(int, int, int, int, float, float, float, float, mg::graphics::rgba, float) override {}
    void circle(int, int, int, mg::graphics::rgba) override {}
    void strokeCircle(int, int, int, mg::graphics::rgba, float) override {}
    void ellipse(int, int, int, int, mg::graphics::rgba) override {}
    void strokeEllipse(int, int, int, int, mg::graphics::rgba, float) override {}
    void ring(int, int, int, int, mg::graphics::rgba) override {}
    void strokeRing(int, int, int, int, mg::graphics::rgba, float) override {}
    void arc(int, int, int, int, float, float, mg::graphics::rgba) override {}
    void strokeArc(int, int, int, int, float, float, mg::graphics::rgba, float) override {}
    void tri(int, int, int, int, int, int, mg::graphics::rgba) override {}
    void strokeTri(int, int, int, int, int, int, mg::graphics::rgba, float) override {}
    float getTextWidth(const char*, std::string, float, mg::graphics::FontWeight, mg::graphics::FontSlant) override { return 0.0F; }
    void drawText(const char*, int, int, std::string, float, mg::graphics::FontWeight, mg::graphics::FontSlant, mg::graphics::rgba, mg::Anchor) override {}
    mg::images::Image createNativeImage(const mg::graphics::rgba*, int, int) override { return {}; }
    void drawImage(const mg::images::Image&, int, int) override {}
    void drawImageRotated(const mg::images::Image&, int, int, float, int, int) override {}
    void drawImageScaled(const mg::images::Image&, int, int, float, float) override {}
    void drawImageTransformed(const mg::images::Image&, int, int, float, float, float, int, int) override {}
    void drawImageStretched(const mg::images::Image&, int, int, int, int) override {}
    void drawImageRegion(const mg::images::Image&, int, int, int, int, int, int, int, int) override {}
    void clip(int, int, int, int) override {}
    void clearClip() override {}
};

class ControlScreen final : public mg::Screen {
public:
    mg::control::Result onControl(mg::control::Action action) override {
        received = action;
        return mg::control::Result::handled();
    }
    void update(mg::context::Context&, std::chrono::microseconds) override {}
    void draw(mg::context::Context&, mg::graphics::Graphics&) override {}

    mg::control::Action received = mg::control::Action::Next;
};

struct ControlFixture {
    MemoryFileSystem fs;
    mg::graphics::UserPalette palette;
    mg::package::Manager packages{fs, "/multigauge"};
#if MG_BUILD_EDITOR
    mg::editor::Manager editors;
    mg::context::Manager contexts{fs, "/multigauge", palette};
    mg::navigation::Manager navigation{contexts, packages, editors};
#else
    mg::context::Manager contexts{fs, "/multigauge", palette};
    mg::navigation::Manager navigation{contexts, packages};
#endif
    mg::control::Manager controls{fs, "/multigauge", navigation};
};

TEST_CASE("controls persist bindings for fixed slots") {
    ControlFixture fixture;
    REQUIRE(fixture.controls.load());
    CHECK_FALSE(fixture.controls.bind(mg::control::Manager::MaxPorts, mg::control::Action::Next));
    REQUIRE(fixture.controls.bind(0, mg::control::Action::Next));
    REQUIRE(fixture.controls.save());

    mg::control::Manager restored{fixture.fs, "/multigauge", fixture.navigation};
    REQUIRE(restored.load());
    std::vector<mg::control::Binding> bindings;
    restored.listBindings(bindings);
    REQUIRE(bindings.size() == 1);
    CHECK(bindings[0].portIndex == 0);
    CHECK(bindings[0].action == mg::control::Action::Next);
    CHECK(restored.portCount() == mg::control::Manager::MaxPorts);
}

TEST_CASE("controls deliver navigation actions only from bound slots") {
    ControlFixture fixture;
    NullGraphicsContext graphics;
    const mg::ContextId contextId = fixture.contexts.add(graphics);
    REQUIRE(contextId.valid());

    auto screen = std::make_unique<ControlScreen>();
    ControlScreen* screenView = screen.get();
    REQUIRE(fixture.contexts.setScreen(contextId, std::move(screen)));

    REQUIRE(fixture.controls.bind(1, mg::control::Action::Select));
    CHECK(fixture.controls.trigger(contextId, 1).type ==
          mg::control::Result::Type::Handled);
    CHECK(screenView->received == mg::control::Action::Select);
    CHECK(fixture.controls.trigger(contextId, mg::control::Manager::MaxPorts).type ==
          mg::control::Result::Type::Ignored);
}

TEST_CASE("next and previous cycle the target gauge package faces") {
    ControlFixture fixture;
    REQUIRE(fixture.packages.importPackage(R"({
        "name":"Daily Driver",
        "author":"Multigauge",
        "description":"",
        "faces":[
            {"name":"Tachometer","face":{}},
            {"name":"Speedometer","face":{}}
        ]
    })").ok);

    NullGraphicsContext graphics;
    const mg::ContextId contextId = fixture.contexts.add(graphics);
    REQUIRE(fixture.navigation.showGauge(contextId, "daily-driver", "tachometer"));
    REQUIRE(fixture.controls.bind(2, mg::control::Action::Next));
    CHECK(fixture.controls.trigger(contextId, 2).type ==
          mg::control::Result::Type::NextFace);
    auto* screen = dynamic_cast<mg::GaugeScreen*>(fixture.contexts.getScreen(contextId));
    REQUIRE(screen);
    CHECK(screen->faceId() == "speedometer");

    REQUIRE(fixture.controls.bind(2, mg::control::Action::Previous));
    CHECK(fixture.controls.trigger(contextId, 2).type ==
          mg::control::Result::Type::PreviousFace);
    screen = dynamic_cast<mg::GaugeScreen*>(fixture.contexts.getScreen(contextId));
    REQUIRE(screen);
    CHECK(screen->faceId() == "tachometer");
}

TEST_CASE("select on a gauge screen returns an open-menu result") {
    ControlFixture fixture;
    NullGraphicsContext graphics;
    const mg::ContextId contextId = fixture.contexts.add(graphics);
    REQUIRE(fixture.navigation.showGauge(contextId, "{}"));
    REQUIRE(fixture.controls.bind(3, mg::control::Action::Select));

    CHECK(fixture.controls.trigger(contextId, 3).type ==
          mg::control::Result::Type::OpenMenu);
}

} // namespace
