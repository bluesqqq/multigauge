#include <doctest/doctest.h>

#include <multigauge/editor/Manager.h>
#include <multigauge/editor/Editor.h>
#include <multigauge/gauge/GaugeFace.h>
#include <multigauge/gauge/elements/CustomElement.h>
#include <multigauge/gauge/elements/FrameElement.h>
#include <multigauge/gauge/elements/Graph.h>
#include <multigauge/gauge/elements/Horizon.h>
#include <multigauge/gauge/elements/Radial.h>
#include <multigauge/gauge/elements/primitives/CircleElement.h>
#include <multigauge/gauge/elements/primitives/ImageElement.h>
#include <multigauge/gauge/elements/primitives/RectangleElement.h>
#include <multigauge/gauge/elements/primitives/TextElement.h>
#include <multigauge/gauge/ticks/RootTick.h>
#include <multigauge/gauge/ticks/SubTick.h>
#include <multigauge/gauge/ticks/TickList.h>
#include <multigauge/graphics/Graphics.h>
#include <multigauge/graphics/TextPaint.h>
#include <multigauge/graphics/colors/ColorTimeline.h>
#include <multigauge/graphics/colors/StaticColor.h>
#include <multigauge/graphics/colors/TimeColor.h>
#include <multigauge/graphics/colors/UserColor.h>
#include <multigauge/graphics/colors/ValueColor.h>
#include <multigauge/value/ValueView.h>

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
    std::vector<mg::graphics::rgba> clears;

    void clear(mg::graphics::rgba color) override { clears.push_back(color); }
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

TEST_CASE("a face without a background clears transparently") {
    mg::gauge::GaugeFace face;
    RecordingGraphicsContext context;
    REQUIRE(context.resize(250, 240));
    mg::graphics::Graphics graphics(context);

    graphics.setFill(mg::graphics::rgba(234, 53, 31));
    face.draw(graphics);

    REQUIRE(context.clears.size() == 1);
    CHECK(context.clears[0].r == 0);
    CHECK(context.clears[0].g == 0);
    CHECK(context.clears[0].b == 0);
    CHECK(context.clears[0].a == 0);
}

TEST_CASE("built-in elements provide inspector layouts") {
    const auto checkLayout = [](mg::PropertyObject& object) {
        auto document = mg::json::object();
        auto writer = document.writer();
        REQUIRE(object.writeInspectorMeta(writer));
        CHECK(document.root().member("layout").isArray());
    };

    mg::gauge::FrameElement frame;
    mg::gauge::Graph graph;
    mg::gauge::Horizon horizon;
    mg::gauge::CircleElement circle;
    mg::gauge::ImageElement image;
    mg::gauge::RectangleElement rectangle;
    mg::gauge::TextElement text;
    mg::gauge::Radial radial;
    mg::graphics::Paint paint;
    mg::graphics::TextPaint textPaint;
    mg::graphics::StaticColor staticColor;
    mg::graphics::TimeColor timeColor;
    mg::graphics::UserColor userColor;
    mg::graphics::ValueColor valueColor;
    mg::graphics::ColorTimeline timeline;
    mg::graphics::PaintTimeline paintTimeline;
    mg::ValueView valueView;
    mg::gauge::RootTick rootTick;
    mg::gauge::SubTick subTick;
    mg::gauge::TickList tickList;
    checkLayout(frame);
    checkLayout(graph);
    checkLayout(horizon);
    checkLayout(circle);
    checkLayout(image);
    checkLayout(rectangle);
    checkLayout(text);
    checkLayout(radial);
    checkLayout(paint);
    checkLayout(textPaint);
    checkLayout(staticColor);
    checkLayout(timeColor);
    checkLayout(userColor);
    checkLayout(valueColor);
    checkLayout(timeline);
    checkLayout(paintTimeline);
    checkLayout(valueView);
    checkLayout(rootTick);
    checkLayout(subTick);
    checkLayout(tickList);
}

TEST_CASE("Clay layout properties serialize grouped padding and floating placement") {
    const auto source = mg::json::parse(R"({
        "width":{"mode":"percent","value":1,"limit":320},
        "height":{"mode":"grow","value":8,"limit":480},
        "direction":"vertical",
        "padding":{"left":4,"right":8,"top":12,"bottom":16},
        "childGap":6,
        "childAlignment":{"x":"center","y":"bottom"},
        "floating":{
            "mode":"relative",
            "elementAnchor":"center",
            "parentAnchor":"center",
            "offset":{"x":3,"y":-2},
            "expand":{"width":4,"height":5},
            "zIndex":7
        },
        "aspectRatio":1.25
    })");
    REQUIRE(source.valid());

    mg::gauge::Element::Layout layout;
    REQUIRE(layout.loadProperties(source.root()));
    CHECK(layout.width.mode == mg::gauge::layout::SizeMode::Percent);
    CHECK(layout.width.value == 1.0F);
    CHECK(layout.width.limit == 320.0F);
    CHECK(layout.direction == mg::gauge::layout::Direction::Vertical);
    CHECK(layout.padding.left == 4);
    CHECK(layout.padding.bottom == 16);
    CHECK(layout.childAlignment.x == mg::gauge::layout::AlignmentX::Center);
    CHECK(layout.childAlignment.y == mg::gauge::layout::AlignmentY::Bottom);
    CHECK(layout.floating.mode == mg::gauge::layout::FloatingMode::Relative);
    CHECK(layout.floating.zIndex == 7);
    CHECK(layout.aspectRatio == 1.25F);

    auto saved = mg::json::object();
    auto writer = saved.writer();
    REQUIRE(layout.saveProperties(writer));
    CHECK(saved.root().member("padding").member("left").type() == mg::json::Type::Int);
    CHECK(saved.root().member("floating").member("mode").type() == mg::json::Type::String);
    CHECK(saved.root().member("width").member("limit").type() == mg::json::Type::Number);
}

TEST_CASE("layout emits an authoritative structured inspector") {
    mg::gauge::Element::Layout layout;
    auto document = mg::json::object();
    auto writer = document.writer();
    REQUIRE(layout.writeInspectorMeta(writer));

    const auto inspector = document.root();
    REQUIRE(inspector.member("properties").isArray());
    const auto presentation = inspector.member("layout");
    REQUIRE(presentation.isArray());
    REQUIRE(presentation.size() == 3);

    const auto position = presentation.element(2);
    std::string_view title;
    REQUIRE(position.member("title").read(title));
    CHECK(title == "Position");
    const auto positionNodes = position.member("children");
    REQUIRE(positionNodes.isArray());
    REQUIRE(positionNodes.size() == 5);
    const auto anchors = positionNodes.element(1);
    std::string_view widget;
    REQUIRE(anchors.member("widget").read(widget));
    CHECK(widget == "anchor-pair");
    std::string_view path;
    REQUIRE(anchors.member("bindings").member("target").read(path));
    CHECK(path == "floating.parentAnchor");
    REQUIRE(anchors.member("visibleWhen").element(0).member("path").read(path));
    CHECK(path == "floating.mode");

    const auto size = presentation.element(0);
    const auto aspectRatio = size.member("children").element(1);
    REQUIRE(aspectRatio.member("label").read(title));
    CHECK(title == "Aspect Ratio");
    REQUIRE(aspectRatio.member("widget").read(widget));
    CHECK(widget == "number");
}

TEST_CASE("layout inspector emits enum options and accepts nested property updates") {
    mg::gauge::Element element{"test"};
    auto document = mg::json::object();
    auto writer = document.writer();
    REQUIRE(element.writeInspectorMeta(writer));

    const auto layout = document.root().member("properties").element(0);
    const auto width = layout.member("properties").element(0);
    const auto sizeMode = width.member("properties").element(0);
    REQUIRE(sizeMode.member("options").isArray());
    CHECK(sizeMode.member("options").size() == 4);

    const auto floating = layout.member("properties").element(6);
    const auto mode = floating.member("properties").element(0);
    REQUIRE(mode.member("options").isArray());
    CHECK(mode.member("options").size() == 3);

    mg::PropertyObject* owner = nullptr;
    const mg::Property* property = nullptr;
    REQUIRE(element.resolvePath("layout.floating.mode", owner, property));
    const auto value = mg::json::parse(R"("relative")");
    REQUIRE(owner->setProperty(property->key, value.root()));

    REQUIRE(element.resolvePath("layout.width.mode", owner, property));
    const auto fixed = mg::json::parse(R"("fixed")");
    REQUIRE(owner->setProperty(property->key, fixed.root()));
}

TEST_CASE("face inspector includes layout sections without a wrapper section") {
    mg::gauge::GaugeFace face;
    mg::PropertyObject* owner = nullptr;
    const mg::Property* property = nullptr;
    REQUIRE(face.resolvePath("bgColor", owner, property));
    const auto color = mg::json::parse(R"("#336699")");
    REQUIRE(owner->setProperty(property->key, color.root()));

    auto document = mg::json::object();
    auto writer = document.writer();
    REQUIRE(face.writeInspectorMeta(writer));

    const auto inspector = document.root();
    std::string_view backgroundValue;
    REQUIRE(inspector.member("properties").element(1).member("properties").element(0).member("value").read(backgroundValue));
    CHECK(backgroundValue == "#336699FF");
    const auto presentation = inspector.member("layout");
    REQUIRE(presentation.isArray());
    REQUIRE(presentation.size() == 2);

    const auto background = presentation.element(0).member("children").element(0);
    std::string_view widget;
    REQUIRE(background.member("widget").read(widget));
    CHECK(widget == "color");

    std::string_view type;
    std::string_view path;
    REQUIRE(presentation.element(1).member("type").read(type));
    REQUIRE(presentation.element(1).member("path").read(path));
    CHECK(type == "include");
    CHECK(path == "layout");

    const auto layoutProperty = inspector.member("properties").element(0);
    REQUIRE(layoutProperty.member("layout").isArray());
    CHECK(layoutProperty.member("layout").size() == 1);
    CHECK(layoutProperty.member("properties").size() == 4);
}

TEST_CASE("floating children with grow sizing share their padded parent's bounds") {
    const auto source = mg::json::parse(R"({
        "layout":{"padding":{"left":10,"right":10,"top":10,"bottom":10}},
        "children":[{
            "type":"frame",
            "layout":{"width":{"mode":"percent","value":1},"height":{"mode":"percent","value":1}},
            "children":[
                {
                    "type":"rectangle",
                    "layout":{
                        "width":{"mode":"grow"},
                        "height":{"mode":"grow"},
                        "floating":{"mode":"relative"}
                    },
                    "paint":{"fill":"#FF0000FF"},
                    "radius":8
                },
                {
                    "type":"rectangle",
                    "layout":{
                        "width":{"mode":"grow"},
                        "height":{"mode":"grow"},
                        "floating":{"mode":"relative","zIndex":1}
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

TEST_CASE("gauge registry exposes radial element metadata") {
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
        "radial",
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

    const auto radial = registry.create("radial");
    REQUIRE(radial != nullptr);
    CHECK(radial->findProperty("parts") != nullptr);
}

TEST_CASE("gauge face round-trips radial parts") {
    const auto scalePart = mg::json::parse(R"({"type":"scale","radius":0.8,"ticks":{"root":{},"subs":[]}})");
    REQUIRE(scalePart.valid());
    mg::gauge::RadialPart::OwnedPart decodedPart;
    REQUIRE(mg::decodeAny(scalePart.root(), decodedPart));
    CHECK(dynamic_cast<mg::gauge::ScalePart*>(decodedPart.get()) != nullptr);

    mg::gauge::GaugeFace source;
    const auto radial = source.createElement("radial");
    const auto parts = mg::json::parse(R"([{"type":"scale","radius":1,"ticks":{"root":{"interval":10},"subs":[]}},{"type":"needle","radius":0.75,"paint":{"fill":"#FF0000FF","thickness":2}}])");
    REQUIRE(parts.valid());
    REQUIRE(source.get(radial)->setProperty("parts", parts.root()));

    auto document = mg::json::object();
    auto writer = document.writer();
    REQUIRE(source.save(writer));
    CHECK(document.root().member("children").size() == 1);
    CHECK(document.root().member("children").element(0).member("type").type() ==
          mg::json::Type::String);
    CHECK(document.root().member("children").element(0).member("parts").size() == 2);

    mg::gauge::GaugeFace restored;
    REQUIRE(restored.load(document.root()));
    auto savedAgain = mg::json::object();
    auto savedAgainWriter = savedAgain.writer();
    REQUIRE(restored.save(savedAgainWriter));
    CHECK(savedAgain.toString() == document.toString());
}

TEST_CASE("radial parts expose embedded polymorphic collection inspector metadata") {
#if MG_BUILD_EDITOR
    mg::gauge::Radial radial;
    const auto parts = mg::json::parse(
        R"([{"type":"scale","radius":1,"ticks":{"root":{},"subs":[]}},{"type":"needle","radius":0.75}])"
    );
    REQUIRE(parts.valid());
    REQUIRE(radial.setProperty("parts", parts.root()));

    auto metadata = mg::json::array();
    auto writer = metadata.writer();
    REQUIRE(radial.writePropertiesMeta(writer));

    mg::json::Reader partsMeta;
    for (std::size_t index = 0; index < metadata.root().size(); ++index) {
        const auto candidate = metadata.root().element(index);
        std::string_view key;
        if (candidate.member("key").read(key) && key == "parts") {
            partsMeta = candidate;
            break;
        }
    }
    REQUIRE(partsMeta.valid());
    CHECK(partsMeta.member("value").isArray());
    const auto collection = partsMeta.member("collection");
    REQUIRE(collection.isObject());
    REQUIRE(collection.member("types").isArray());
    CHECK(collection.member("types").size() == 2);
    REQUIRE(collection.member("items").isArray());
    CHECK(collection.member("items").size() == 2);

    std::string_view firstType;
    REQUIRE(collection.member("items").element(0).member("type").read(firstType));
    CHECK(firstType == "scale");
    const auto scaleInspector = collection.member("items").element(0).member("inspector");
    CHECK(scaleInspector.member("properties").isArray());
    CHECK(scaleInspector.member("layout").isArray());

    std::string_view secondType;
    REQUIRE(collection.member("items").element(1).member("type").read(secondType));
    CHECK(secondType == "needle");
    const auto needleInspector = collection.member("items").element(1).member("inspector");
    CHECK(needleInspector.member("properties").isArray());
    CHECK(needleInspector.member("layout").isArray());
#endif
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
    const auto faceInspector = editor.getFaceInspector(faceId);
    REQUIRE(faceInspector.ok);
    CHECK(faceInspector.data.root().member("inspector").member("properties").isArray());
#if MG_BUILD_EDITOR
    CHECK(faceInspector.data.root().member("inspector").member("layout").isArray());
#endif
    const auto elementInspector = editor.getElementInspector(rootRef);
    REQUIRE(elementInspector.ok);
    CHECK(elementInspector.data.root().member("inspector").member("properties").isArray());
#if MG_BUILD_EDITOR
    CHECK(elementInspector.data.root().member("inspector").member("layout").isArray());
#endif

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
    mg::editor::Manager editors;
    const auto id = editors.create();
    REQUIRE(id.valid());
    CHECK_FALSE(editors.canUndo(id));
    CHECK_FALSE(editors.canRedo(id));
    const auto values = editors.listValueIDs(id);
    REQUIRE(values.ok);
    CHECK(values.data.root().size() == mg::ValueRegistry::size());
    std::string_view secondValueId;
    REQUIRE(values.data.root().element(1).read(secondValueId));
    CHECK(secondValueId == "engineRPM");
    const auto created = editors.createFace(id, "{}");
    REQUIRE(created.ok);
    CHECK(editors.canUndo(id));
    std::uint64_t rawFaceId = 0;
    REQUIRE(created.data.root().member("id").read(rawFaceId));
    const auto faceId = static_cast<mg::editor::NodeId>(rawFaceId);
    CHECK(editors.isFace(id, faceId));
    CHECK(editors.getFace(id, faceId) != nullptr);
    CHECK(editors.getFaceInspector(id, faceId).ok);
    CHECK(editors.exportPackage(id).ok);
    CHECK(editors.destroy(id));
}

} // namespace
