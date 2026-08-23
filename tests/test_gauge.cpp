#include <doctest/doctest.h>

#include <multigauge/editor/Api.h>
#include <multigauge/editor/EditorRegistry.h>
#include <multigauge/editor/Editor.h>
#include <multigauge/gauge/GaugeFace.h>
#include <multigauge/gauge/elements/CustomElement.h>
#include <multigauge/gauge/elements/Graph.h>
#include <multigauge/graphics/Graphics.h>

#include <string>
#include <string_view>
#include <vector>

namespace {

mg::gauge::NodeHandle readHandle(mg::json::Reader value) {
    std::uint64_t slot = 0;
    std::uint64_t generation = 0;
    if (!value.isObject() || !value.member("slot").read(slot) ||
        !value.member("generation").read(generation)) {
        return mg::gauge::NodeHandle::invalid();
    }
    return mg::gauge::NodeHandle::make(static_cast<std::uint32_t>(slot),
                                       static_cast<std::uint32_t>(generation));
}

TEST_CASE("gauge face owns a handle-based hierarchy") {
    mg::gauge::GaugeFace face;
    const auto root = face.createElement("root");
    const auto first = face.createElement("first");
    const auto second = face.createElement("second");

    CHECK(face.moveElement(first, root, 0));
    CHECK(face.moveElement(second, root, 0));
    CHECK(face.parentOf(first) == root);
    CHECK(face.parentOf(second) == root);
    const auto* secondElement = face.get(second);
    REQUIRE(secondElement != nullptr);
    const char* secondType = secondElement->typeId();
    REQUIRE(secondType != nullptr);
    CHECK(std::string_view(secondType) == "second");

    CHECK(face.moveElement(second, mg::gauge::NodeHandle::invalid(), 0));
    CHECK(face.parentOf(second) == mg::gauge::NodeHandle::invalid());
}

TEST_CASE("gauge rejects cycles and invalidates deleted handles") {
    mg::gauge::GaugeFace face;
    const auto root = face.createElement("root");
    const auto child = face.createElement("child");

    CHECK(face.moveElement(child, root, 0));
    CHECK_FALSE(face.moveElement(root, child, 0));
    CHECK(face.deleteElement(root));
    CHECK(face.get(root) == nullptr);
    CHECK(face.get(child) == nullptr);
}

TEST_CASE("gauge reorders roots and preserves unknown element type IDs") {
    mg::gauge::GaugeFace face;
    const auto first = face.createElement("first");
    const auto second = face.createElement("second");
    REQUIRE(face.moveElement(second, mg::gauge::NodeHandle::invalid(), 0));

    std::vector<std::string_view> roots;
    face.forEachRoot([&](mg::gauge::NodeHandle, const mg::gauge::Element& element) {
        roots.emplace_back(element.typeId());
    });
    REQUIRE(roots.size() == 2);
    CHECK(roots[0] == "second");
    CHECK(roots[1] == "first");

    REQUIRE(face.deleteElement(first));
    CHECK(face.get(first) == nullptr);
    const auto replacement = face.createElement("replacement");
    CHECK(replacement != first);
    CHECK(face.get(first) == nullptr);
    REQUIRE(face.get(replacement) != nullptr);
    CHECK(std::string_view(face.get(replacement)->typeId()) == "replacement");

    mg::gauge::Element::OwnedElement decoded;
    {
        const auto unknown = mg::json::parse(R"({"type":"plugin-element","layout":{}})");
        REQUIRE(unknown.valid());
        REQUIRE(mg::decodeAny(unknown.root(), decoded));
    }
    REQUIRE(decoded != nullptr);
    CHECK(dynamic_cast<mg::gauge::CustomElement*>(decoded.get()) != nullptr);
    CHECK(std::string_view(decoded->typeId()) == "plugin-element");

    auto encoded = mg::json::object();
    auto encodedWriter = encoded.writer();
    REQUIRE(mg::encodeAny(encodedWriter, decoded));
    std::string_view encodedType;
    REQUIRE(encoded.root().member("type").read(encodedType));
    CHECK(encodedType == "plugin-element");
}

TEST_CASE("gauge face exposes Clay layout properties") {
    mg::gauge::GaugeFace face;

    CHECK(face.typeId() == nullptr);
    CHECK(face.findProperty("bgColor") != nullptr);
    CHECK(face.findProperty("layout") != nullptr);
}

class RecordingGraphicsContext final : public mg::graphics::GraphicsContext {
public:
    struct RoundedRect {
        int x;
        int y;
        int width;
        int height;
        float radius;
    };

    std::vector<RoundedRect> roundedRects;

    void clear(mg::graphics::rgba) override {}
    void pixel(int, int, mg::graphics::rgba) override {}
    void line(int, int, int, int, mg::graphics::rgba, float) override {}
    void rect(int, int, int, int, mg::graphics::rgba) override {}
    void strokeRect(int, int, int, int, mg::graphics::rgba, float) override {}
    void roundRect(int x, int y, int width, int height, float radius, mg::graphics::rgba) override {
        roundedRects.push_back({x, y, width, height, radius});
    }
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

TEST_CASE("Clay layout properties serialize grouped padding and floating placement") {
    const auto source = mg::json::parse(R"({
        "width":{"mode":"percent","value":1,"limit":320},
        "height":{"mode":"grow","value":8,"limit":480},
        "direction":"top-to-bottom",
        "padding":{"left":4,"right":8,"top":12,"bottom":16},
        "childGap":6,
        "childAlignment":{"x":"center","y":"bottom"},
        "floating":{
            "attachTo":"parent",
            "elementAnchor":"center",
            "parentAnchor":"center",
            "offset":{"x":3,"y":-2},
            "expand":{"width":4,"height":5},
            "zIndex":7,
            "fillParent":true
        },
        "aspectRatio":1.25
    })");
    REQUIRE(source.valid());

    mg::gauge::layout::Layout layout;
    REQUIRE(layout.loadProperties(source.root()));
    CHECK(layout.width.mode == mg::gauge::layout::SizeMode::Percent);
    CHECK(layout.width.value == 1.0F);
    CHECK(layout.width.limit == 320.0F);
    CHECK(layout.padding.left == 4);
    CHECK(layout.padding.bottom == 16);
    CHECK(layout.childAlignment.x == mg::gauge::layout::AlignmentX::Center);
    CHECK(layout.childAlignment.y == mg::gauge::layout::AlignmentY::Bottom);
    CHECK(layout.floating.attachTo == mg::gauge::layout::FloatingAttachTo::Parent);
    CHECK(layout.floating.zIndex == 7);
    CHECK(layout.floating.fillParent);
    CHECK(layout.aspectRatio == 1.25F);

    auto saved = mg::json::object();
    auto writer = saved.writer();
    REQUIRE(layout.saveProperties(writer));
    CHECK(saved.root().member("padding").member("left").type() == mg::json::Type::Int);
    CHECK(saved.root().member("floating").member("attachTo").type() == mg::json::Type::String);
    CHECK(saved.root().member("width").member("limit").type() == mg::json::Type::Number);
}

TEST_CASE("floating children share their padded parent's bounds") {
    const auto source = mg::json::parse(R"({
        "layout":{"padding":{"left":10,"right":10,"top":10,"bottom":10}},
        "children":[{
            "type":"frame",
            "layout":{"width":{"mode":"percent","value":1},"height":{"mode":"percent","value":1}},
            "children":[
                {
                    "type":"rectangle",
                    "layout":{
                        "width":{"mode":"percent","value":1},
                        "height":{"mode":"percent","value":1},
                        "floating":{"attachTo":"parent","fillParent":true}
                    },
                    "paint":{"fill":"#FF0000FF"},
                    "radius":8
                },
                {
                    "type":"rectangle",
                    "layout":{
                        "width":{"mode":"percent","value":1},
                        "height":{"mode":"percent","value":1},
                        "floating":{"attachTo":"parent","fillParent":true,"zIndex":1}
                    },
                    "paint":{"fill":"#00FF00FF"},
                    "radius":8
                }
            ]
        }]
    })");
    REQUIRE(source.valid());

    mg::gauge::GaugeFace face;
    REQUIRE(face.load(source.root()));
    RecordingGraphicsContext context;
    REQUIRE(context.resize(250, 240));
    mg::graphics::Graphics graphics(context);
    mg::graphics::ColorFrame colorFrame;
    mg::graphics::UserPalette palette;
    colorFrame.refresh({}, palette);
    graphics.beginFrame(colorFrame);
    face.layout(graphics);
    face.draw(graphics);
    graphics.endFrame();

    REQUIRE(context.roundedRects.size() == 2);
    CHECK(context.roundedRects[0].x == 10);
    CHECK(context.roundedRects[0].y == 10);
    CHECK(context.roundedRects[0].width == 230);
    CHECK(context.roundedRects[0].height == 220);
    CHECK(context.roundedRects[1].x == context.roundedRects[0].x);
    CHECK(context.roundedRects[1].y == context.roundedRects[0].y);
    CHECK(context.roundedRects[1].width == context.roundedRects[0].width);
    CHECK(context.roundedRects[1].height == context.roundedRects[0].height);
}

TEST_CASE("gauge registry exposes the migrated legacy element metadata") {
    const auto& registry = mg::gauge::Element::registry();
    const auto custom = registry.create("unknown-element");
    REQUIRE(custom != nullptr);
    CHECK(dynamic_cast<mg::gauge::CustomElement*>(custom.get()) != nullptr);
    CHECK(std::string_view(custom->typeId()) == "unknown-element");

    constexpr std::array ids{
        "frame",
        "rectangle",
        "circle",
        "text",
        "image",
        "circular-element",
        "circular-needle",
        "circular-scale",
        "graph",
        "horizon",
    };
    for (const char* id : ids) {
        const auto* descriptor = registry.find(id);
        REQUIRE(descriptor != nullptr);
        REQUIRE(descriptor->create != nullptr);
        const auto element = descriptor->create();
        CHECK(std::string_view(element->typeId()) == id);
        CHECK(element->findProperty("layout") != nullptr);
    }

    const auto graph = registry.create("graph");
    REQUIRE(graph != nullptr);
    CHECK(graph->findProperty("seconds") != nullptr);
    CHECK(graph->findProperty("value") != nullptr);
}

TEST_CASE("gauge face round-trips polymorphic element trees") {
    mg::gauge::GaugeFace source;
    const auto circular = source.createElement("circular-element");
    const auto needle = source.createElement("circular-needle");
    const auto rectangle = source.createElement("rectangle");
    REQUIRE(source.moveElement(needle, circular, 0));
    REQUIRE(source.moveElement(rectangle, circular, 1));

    const auto setRadius = mg::json::parse("0.75");
    REQUIRE(setRadius.valid());
    REQUIRE(source.get(needle)->setProperty("radius", setRadius.root()));

    auto document = mg::json::object();
    auto writer = document.writer();
    REQUIRE(source.save(writer));
    CHECK(document.root().member("children").size() == 1);
    CHECK(document.root().member("children").element(0).member("type").type() ==
          mg::json::Type::String);

    mg::gauge::GaugeFace restored;
    REQUIRE(restored.load(document.root()));
    auto savedAgain = mg::json::object();
    auto savedAgainWriter = savedAgain.writer();
    REQUIRE(restored.save(savedAgainWriter));
    CHECK(savedAgain.toString() == document.toString());
}

TEST_CASE("gauge element codec owns type and property serialization") {
    auto element = mg::gauge::Element::registry().create("rectangle");
    REQUIRE(element != nullptr);
    const auto radius = mg::json::parse("4.5");
    REQUIRE(radius.valid());
    REQUIRE(element->setProperty("radius", radius.root()));

    auto document = mg::json::object();
    auto writer = document.writer();
    REQUIRE(mg::encodeAny(writer, element));
    CHECK(document.root().member("type").type() == mg::json::Type::String);
    CHECK(document.root().member("radius").type() == mg::json::Type::Number);

    mg::gauge::Element::OwnedElement decoded;
    REQUIRE(mg::decodeAny(document.root(), decoded));
    REQUIRE(decoded != nullptr);
    CHECK(std::string_view(decoded->typeId()) == "rectangle");
}

TEST_CASE("gauge editor preserves hierarchy invariants through editing and history") {
    mg::editor::Editor editor;
    REQUIRE_FALSE(editor.exportPackage().empty());
    const mg::editor::Editor::PackageInfo packageInfo{"Package", "Author", "Description"};
    REQUIRE(editor.setPackageInfo(packageInfo));
    CHECK(editor.packageInfo().name == "Package");
    REQUIRE(editor.undo());
    CHECK(editor.packageInfo().name.empty());
    REQUIRE(editor.redo());
    CHECK(editor.packageInfo().name == "Package");

    const auto createdFace = editor.createFace("{}");
    REQUIRE(createdFace.ok);

    std::uint64_t rawFaceId = 0;
    REQUIRE(createdFace.data.root().member("id").read(rawFaceId));
    const auto faceId = static_cast<mg::editor::Editor::FaceId>(rawFaceId);
    REQUIRE(editor.setFaceName(faceId, "Main"));
    CHECK(editor.getFaceName(faceId) == "Main");

    mg::editor::ElementPlacement rootPlacement{
        faceId, mg::gauge::NodeHandle::invalid(), mg::editor::Editor::Append};
    const auto createdRoot =
        editor.createElement(rootPlacement, R"({"type":"rectangle","radius":4})");
    REQUIRE(createdRoot.ok);
    const auto root = readHandle(createdRoot.data.root().member("element"));
    REQUIRE(root.valid());

    mg::editor::ElementPlacement childPlacement{faceId, root, 0};
    const auto createdChild = editor.createElement(childPlacement, R"({"type":"circle"})");
    REQUIRE(createdChild.ok);
    const auto child = readHandle(createdChild.data.root().member("element"));
    REQUIRE(child.valid());

    auto* face = editor.getFace(faceId);
    REQUIRE(face != nullptr);
    CHECK(face->parentOf(child) == root);

    const mg::editor::ElementRef rootRef{faceId, root};
    REQUIRE(editor.setElementProperty(rootRef, "radius", "8").ok);
    const auto property = editor.getElementProperty(rootRef, "radius");
    REQUIRE(property.ok);
    double radius = 0.0;
    REQUIRE(property.data.root().member("value").read(radius));
    CHECK(radius == 8.0);

    REQUIRE(editor.replaceElement(rootRef, R"({"type":"frame"})").ok);
    REQUIRE(face->get(root) != nullptr);
    CHECK(std::string_view(face->get(root)->typeId()) == "frame");
    CHECK(face->parentOf(child) == root);

    const auto hierarchy = editor.getHierarchy();
    REQUIRE(hierarchy.ok);
    CHECK(hierarchy.data.root().member("faces").size() == 1);
    CHECK(editor.listElementTypes().ok);
    CHECK(editor.getFacePropertiesMeta(faceId).ok);
    CHECK(editor.getElementPropertiesMeta(rootRef).ok);

    const std::string package = editor.exportPackage();
    REQUIRE_FALSE(package.empty());
    const auto packageDocument = mg::json::parse(package);
    REQUIRE(packageDocument.valid());
    CHECK_FALSE(packageDocument.root().member("faces").element(0).member("id").valid());
    mg::editor::Editor restoredEditor;
    REQUIRE(restoredEditor.loadPackage(package));
    CHECK(restoredEditor.faceCount() == 1);
    CHECK(restoredEditor.getHierarchy().ok);

    REQUIRE(editor.removeElement(rootRef).ok);
    CHECK(face->get(root) == nullptr);
    CHECK(face->get(child) == nullptr);
    REQUIRE(editor.undo());
    face = editor.getFace(faceId);
    REQUIRE(face != nullptr);
    CHECK(face->get(root) == nullptr);
    CHECK(editor.canRedo());
    std::size_t restoredRoots = 0;
    face->forEachRoot([&](mg::gauge::NodeHandle, const mg::gauge::Element&) { ++restoredRoots; });
    CHECK(restoredRoots == 1);
    REQUIRE(editor.redo());
    restoredRoots = 0;
    editor.getFace(faceId)->forEachRoot(
        [&](mg::gauge::NodeHandle, const mg::gauge::Element&) { ++restoredRoots; });
    CHECK(restoredRoots == 0);
}

TEST_CASE("editor API drives the screen-facing gauge face") {
    const auto id = mg::editor::create();
    REQUIRE(id.valid());
    CHECK_FALSE(mg::editor::canUndo(id));
    CHECK_FALSE(mg::editor::canRedo(id));
    const auto values = mg::editor::listValueIDs(id);
    REQUIRE(values.ok);
    CHECK(values.data.root().size() == mg::ValueRegistry::size());
    std::string_view secondValueId;
    REQUIRE(values.data.root().element(1).read(secondValueId));
    CHECK(secondValueId == "engineRPM");
    const auto created = mg::editor::createFace(id, "{}");
    REQUIRE(created.ok);
    CHECK(mg::editor::canUndo(id));
    std::uint64_t rawFaceId = 0;
    REQUIRE(created.data.root().member("id").read(rawFaceId));
    const auto faceId = static_cast<mg::editor::NodeId>(rawFaceId);
    CHECK(mg::editor::isFace(id, faceId));
    CHECK(mg::editor::getFace(id, faceId) != nullptr);
    CHECK(mg::editor::getFacePropertiesMeta(id, faceId).ok);
    CHECK(mg::editor::exportPackage(id).ok);
    CHECK(mg::editor::destroy(id));
}

} // namespace
