#pragma once

#include <chrono>
#include <cstddef>
#include <memory>
#include <string_view>
#include <utility>
#include <vector>

#include <multigauge/container/HandlePool.h>
#include <multigauge/gauge/Element.h>
#include <multigauge/graphics/colors/Color.h>

namespace mg {

// Forward Declarations
namespace graphics { class Graphics; class GraphicsContext; }
class AssetManager;

namespace gauge {

/// @brief Owns an element tree, layout state, and rendering lifecycle.
class GaugeFace : public ::mg::PropertyObject {
    MG_EDITOR_NAME("New Gauge Face")

private:
    struct Node; // Forward Declaration

public:
    //----------[ CTOR + DTOR ]----------//

    /// @brief Creates an empty face and its Clay context.
    GaugeFace();

    /// @brief Releases every owned element and the Clay context.
    ~GaugeFace();

    //----------[ TREE ]----------//

    /// @brief Creates a registered element as a root.
    /// @param typeId Stable registry type identifier.
    /// @return Handle for the new element, or an invalid handle on failure.
    [[nodiscard]] NodeHandle createElement(std::string_view typeId);

    /// @brief Adds already-constructed element state as a root.
    /// @param element Exclusive element ownership transferred to this face.
    /// @return Handle for the new element, or an invalid handle when element is null.
    [[nodiscard]] NodeHandle addElement(std::unique_ptr<Element> element);

    /// @brief Destroys an element and all of its descendants.
    /// @param root Handle of the subtree root.
    /// @return True when root identified a live element.
    bool deleteElement(NodeHandle root);

    /// @brief Replaces one element's type-specific state without changing its handle or tree links.
    /// @param node Live handle whose owned element is replaced.
    /// @param element New exclusively owned element state.
    /// @return False when node or element is invalid.
    bool replaceElement(
        NodeHandle node,
        std::unique_ptr<Element> element
    );

    /// @brief Moves or reorders an element within this face.
    /// @param element Live element handle to move.
    /// @param parent Destination parent handle, or an invalid handle to make a root.
    /// @param index Sibling insertion index; values beyond the end append.
    /// @return False for invalid elements, self-parenting, or hierarchy cycles.
    bool moveElement(
        NodeHandle element,
        NodeHandle parent,
        std::size_t index
    );

    //----------[ LOOKUP ]----------//

    /// @brief Returns mutable element state for a live handle.
    /// @return Null when node is invalid or stale.
    [[nodiscard]] Element* get(NodeHandle node) noexcept;

    /// @brief Returns read-only element state for a live handle.
    /// @return Null when node is invalid or stale.
    [[nodiscard]] const Element* get(NodeHandle node) const noexcept;

    /// @brief Returns the immediate parent handle.
    /// @return An invalid handle when node is a root, invalid, or stale.
    [[nodiscard]] NodeHandle parentOf(NodeHandle node) const noexcept;

    /// @brief Visits root elements in sibling order.
    /// @param visitor Receives each live root handle and its element.
    template <typename Visitor> void forEachRoot(Visitor&& visitor) const {
        for (NodeHandle handle = firstRoot_; handle.valid();) {
            const Node* current = node(handle);

            if (!current) return;

            const NodeHandle next = current->nextSibling;
            visitor(handle, *current->element);
            handle = next;
        }
    }

    /// @brief Visits children of parent in sibling order.
    /// @param parent Live parent handle.
    /// @param visitor Receives each live child handle and its element.
    template <typename Visitor> void forEachChild(
        NodeHandle parent,
        Visitor&& visitor
    ) const {
        const Node* parentNode = node(parent);

        if (!parentNode) return;

        for (NodeHandle handle = parentNode->firstChild; handle.valid();) {
            const Node* current = node(handle);

            if (!current) return;

            const NodeHandle next = current->nextSibling;
            visitor(handle, *current->element);
            handle = next;
        }
    }

    //----------[ SERIALIZATION ]----------//

    /// @brief Replaces face properties and hierarchy from a property-system payload.
    /// @see docs/schemas/gauge.schema.json for the document shape.
    /// @param value JSON object containing face properties and flat recursive children.
    /// @return True when all properties and element payloads decode successfully.
    bool load(json::Reader value);

    /// @brief Serializes face properties and hierarchy through the property system.
    /// @see docs/schemas/gauge.schema.json for the document shape.
    /// @param writer Destination JSON writer.
    /// @return True when every property and element payload encodes successfully.
    bool save(json::Writer& writer) const;

    //----------[ LIFECYCLE ]----------//

    /// @brief Initializes external resources for every element in depth-first order.
    /// @return False if any element initialization fails.
    bool init(
        ::mg::AssetManager& assetManager,
        ::mg::graphics::GraphicsContext& context
    );

    /// @brief Advances every element in depth-first order.
    /// @param delta Elapsed time since the preceding update.
    void update(std::chrono::microseconds delta);

    /// @brief Calculates element rectangles for the current graphics target.
    void layout(::mg::graphics::Graphics& graphics);

    /// @brief Draws every element in depth-first order using the latest layout.
    void draw(::mg::graphics::Graphics& graphics);

private:
    //----------[ NODE ]----------//

    /// @brief Stores one owned element and its handle-based tree links.
    struct Node {
        std::unique_ptr<Element> element; ///< Exclusively owned type-specific state.
        ::mg::Rect<float> bounds{0.0f, 0.0f, 0.0f, 0.0f}; ///< Latest layout rectangle.
        NodeHandle parent;                                ///< Parent handle; invalid for roots.
        NodeHandle firstChild;  ///< First child handle; invalid when empty.
        NodeHandle nextSibling; ///< Next sibling handle; invalid when last.

        /// @brief Takes exclusive ownership of an element for pool storage.
        explicit Node(std::unique_ptr<Element> value) : element(std::move(value)) {}
    };

    using NodePool = ::mg::HandlePool<Node, NodeHandle>;

    //----------[ TREE HELPERS ]----------//

    /// @brief Finds mutable storage for a live handle.
    [[nodiscard]] Node* node(NodeHandle handle) noexcept;

    /// @brief Finds read-only storage for a live handle.
    [[nodiscard]] const Node* node(NodeHandle handle) const noexcept;

    /// @brief Tests whether attaching child beneath parent would create a cycle.
    [[nodiscard]] bool wouldCreateCycle(
        NodeHandle parent,
        NodeHandle child
    ) const noexcept;

    /// @brief Attaches or reparents a child beneath a parent.
    bool insertChild(
        NodeHandle parent,
        NodeHandle child,
        std::size_t index
    );

    /// @brief Inserts or reorders an element in the root sibling list.
    bool insertRoot(
        NodeHandle child,
        std::size_t index
    );

    /// @brief Appends an unattached node to the root sibling list.
    void appendRoot(NodeHandle root);

    /// @brief Removes a node from its current sibling list.
    void unlink(NodeHandle child);

    //----------[ LIFECYCLE HELPERS ]----------//

    /// @brief Updates one subtree after resolving inherited circular values.
    void updateSubtree(
        NodeHandle root,
        std::chrono::microseconds delta
    );

    /// @brief Resolves circular inheritance from node's handle-tracked parent.
    void resolveInherited(Node& node);

    /// @brief Initializes one subtree and accumulates initialization failures.
    bool initSubtree(
        NodeHandle root,
        ::mg::AssetManager& assetManager,
        ::mg::graphics::GraphicsContext& context
    );

    /// @brief Draws one subtree after resolving inherited circular values.
    void drawSubtree(
        NodeHandle root,
        ::mg::graphics::Graphics& graphics
    );

    //----------[ LAYOUT ]----------//

    /// @brief Emits one subtree into Clay's layout declaration stream.
    void declareClaySubtree(NodeHandle root) const;

    /// @brief Copies Clay-calculated rectangles into node metadata.
    void updateBoundsSubtree(NodeHandle root);

    //----------[ SERIALIZATION HELPERS ]----------//

    /// @brief Replaces root children from a hidden face property.
    static bool setChildren(
        ::mg::PropertyObject* object,
        json::Reader value
    );

    /// @brief Serializes root children through a hidden face property.
    static bool getChildren(
        const ::mg::PropertyObject* object,
        json::Writer& writer
    );

    /// @brief Decodes recursive children and attaches them beneath parent.
    bool loadChildren(
        json::Reader value,
        NodeHandle parent
    );

    /// @brief Encodes children beneath parent, or roots when parent is invalid.
    bool saveChildren(
        json::Writer& writer,
        NodeHandle parent
    ) const;

    //----------[ STATE ]----------//

    NodePool nodes_;                             ///< Pool that owns all elements and tree metadata.
    NodeHandle firstRoot_;                       ///< First root in sibling order.
    layout::Layout layout_;                      ///< Layout options.
    ::mg::graphics::OwnedColor backgroundColor_; ///< Serialized face background color.
    std::vector<std::byte> clayMemory_;          ///< Backing storage for the Clay arena.
    void* clayContext_ = nullptr;                ///< Opaque Clay context allocated in clayMemory_.

    MG_PROPS_BEGIN()
        MG_PROP(layout_, "layout", "Layout", "Layout options.")
        MG_PROP(backgroundColor_, "bgColor", "Background Color", "Gauge face background color.")
        MG_PROP_CUSTOM_HIDDEN("children", "Children", "Face element hierarchy.", &GaugeFace::setChildren, &GaugeFace::getChildren)
    MG_PROPS_END()
};

} // namespace gauge

} // namespace mg
