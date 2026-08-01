#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace mg {

template<typename T, typename Handle>
class HandlePool {
public:
    /* ----- TYPES ----- */

    using ValueType = T;
    using HandleType = Handle;

    /* ----- ITERATORS ----- */

    auto begin() noexcept {
        return values_.begin();
    }

    auto begin() const noexcept {
        return values_.begin();
    }

    auto end() noexcept {
        return values_.end();
    }

    auto end() const noexcept {
        return values_.end();
    }

    /* ----- CAPACITY ----- */

    [[nodiscard]]
    std::size_t size() const noexcept {
        return values_.size();
    }

    [[nodiscard]]
    bool empty() const noexcept {
        return values_.empty();
    }

    /* ----- MODIFIERS ----- */

    void clear() {
        values_.clear();
        slots_.clear();
        valueToSlot_.clear();
        freeSlots_.clear();
    }

    Handle add(T value) {
        return emplace(std::move(value));
    }

    template<typename... Args>
    Handle emplace(Args&&... args) {
        std::uint32_t slotIndex;

        if (!freeSlots_.empty()) {
            slotIndex = freeSlots_.back();
            freeSlots_.pop_back();
        } else {
            slotIndex = static_cast<std::uint32_t>(slots_.size());

            slots_.push_back({
                0,
                InitialGeneration,
                true
            });
        }

        Slot& slot = slots_[slotIndex];
        const std::uint32_t valueIndex =
            static_cast<std::uint32_t>(values_.size());

        values_.emplace_back(std::forward<Args>(args)...);
        valueToSlot_.push_back(slotIndex);

        slot.index = valueIndex;
        slot.occupied = true;

        return Handle::make(slotIndex, slot.generation);
    }

    bool remove(Handle handle) {
        Slot* slot = findSlot(handle);

        if (!slot) return false;

        const std::uint32_t removedSlot = handle.slot();
        const std::uint32_t removeIndex = slot->index;
        const std::uint32_t lastIndex = static_cast<std::uint32_t>(values_.size() - 1);

        if (removeIndex != lastIndex) {
            values_[removeIndex] = std::move(values_[lastIndex]);

            const std::uint32_t movedSlot = valueToSlot_[lastIndex];

            valueToSlot_[removeIndex] = movedSlot;
            slots_[movedSlot].index = removeIndex;
        }

        values_.pop_back();
        valueToSlot_.pop_back();

        slot->occupied = false;
        slot->generation = nextGeneration(slot->generation);

        freeSlots_.push_back(removedSlot);

        return true;
    }

    /* ----- ACCESS ----- */

    [[nodiscard]]
    T* get(Handle handle) noexcept {
        Slot* slot = findSlot(handle);
        return slot ? &values_[slot->index] : nullptr;
    }

    [[nodiscard]]
    const T* get(Handle handle) const noexcept {
        const Slot* slot = findSlot(handle);
        return slot ? &values_[slot->index] : nullptr;
    }

    [[nodiscard]]
    bool exists(Handle handle) const noexcept {
        return findSlot(handle) != nullptr;
    }

private:
    /* ----- TYPES ----- */

    struct Slot {
        std::uint32_t index = 0;
        std::uint32_t generation = InitialGeneration;
        bool occupied = false;
    };

    /* ----- CONSTANTS ----- */

    static constexpr std::uint32_t InitialGeneration = 1;

    /* ----- INTERNAL HELPERS ----- */

    [[nodiscard]]
    static constexpr std::uint32_t nextGeneration(
        std::uint32_t generation
    ) noexcept {
        ++generation;

        if (generation == 0) {
            generation = InitialGeneration;
        }

        return generation;
    }

    [[nodiscard]]
    Slot* findSlot(Handle handle) noexcept {
        if (!handle.valid()) return nullptr;

        const std::uint32_t slotIndex = handle.slot();

        if (slotIndex >= slots_.size()) return nullptr;

        Slot& slot = slots_[slotIndex];

        if (!slot.occupied) return nullptr;

        if (slot.generation != handle.generation()) return nullptr;

        return &slot;
    }

    [[nodiscard]]
    const Slot* findSlot(Handle handle) const noexcept {
        if (!handle.valid()) return nullptr;

        const std::uint32_t slotIndex = handle.slot();

        if (slotIndex >= slots_.size()) return nullptr;

        const Slot& slot = slots_[slotIndex];

        if (!slot.occupied) return nullptr;

        if (slot.generation != handle.generation()) return nullptr;

        return &slot;
    }

    /* ----- DATA ----- */

    std::vector<T> values_;
    std::vector<Slot> slots_;
    std::vector<std::uint32_t> valueToSlot_;
    std::vector<std::uint32_t> freeSlots_;
};

} // namespace mg