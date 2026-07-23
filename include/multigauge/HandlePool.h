#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace mg {

template<typename T>
class HandlePool {
public:
    using Id = uint32_t;

    static constexpr Id Invalid = 0;

private:
    struct Slot {
        uint32_t index;
        uint32_t generation;
    };

    std::vector<T> values;
    std::vector<Slot> slots;
    std::vector<uint32_t> valueToSlot;
    std::vector<uint32_t> freeSlots;

    static constexpr uint32_t INDEX_BITS = 16;
    static constexpr uint32_t INDEX_MASK = (1u << INDEX_BITS) - 1;

    static Id makeId(uint32_t slot, uint32_t gen) { return (gen << INDEX_BITS) | slot; }

    static uint32_t getSlot(Id id) { return id & INDEX_MASK; }
    static uint32_t getGeneration(Id id) { return id >> INDEX_BITS; }

    Slot* findSlot(Id id) {
        if (id == Invalid) return nullptr;

        uint32_t slot = getSlot(id);
        uint32_t gen  = getGeneration(id);

        if (slot >= slots.size()) return nullptr;

        Slot& s = slots[slot];
        if (s.generation != gen) return nullptr;

        return &s;
    }

    const Slot* findSlot(Id id) const {
        if (id == Invalid) return nullptr;

        uint32_t slot = getSlot(id);
        uint32_t gen  = getGeneration(id);

        if (slot >= slots.size()) return nullptr;

        const Slot& s = slots[slot];
        if (s.generation != gen) return nullptr;

        return &s;
    }

public:
    //----------[ ITERATORS ]----------//
    auto begin() { return values.begin(); }
    auto begin() const { return values.begin(); }

    auto end() { return values.end(); }
    auto end() const { return values.end(); }

    //----------[ CAPACITY ]----------//
    std::size_t size() const { return values.size(); }
    bool empty() const { return values.empty(); }

    //----------[ MODIFIERS ]----------//
    void clear() {
        values.clear();
        slots.clear();
        valueToSlot.clear();
        freeSlots.clear();
    }

    Id add(T value) { return emplace(std::move(value)); }

    template<typename... Args>
    Id emplace(Args&&... args) {
        uint32_t slotIndex;

        if (!freeSlots.empty()) {
            slotIndex = freeSlots.back();
            freeSlots.pop_back();
        } else {
            slotIndex = static_cast<uint32_t>(slots.size());
            slots.push_back({0, 1});
        }

        uint32_t valueIndex = static_cast<uint32_t>(values.size());

        values.emplace_back(std::forward<Args>(args)...);
        valueToSlot.push_back(slotIndex);

        slots[slotIndex].index = valueIndex;

        return makeId(slotIndex, slots[slotIndex].generation);
    }

    bool remove(Id id) {
        if (id == Invalid) return false;

        uint32_t slot = getSlot(id);
        uint32_t gen  = getGeneration(id);

        if (slot >= slots.size()) return false;

        Slot& s = slots[slot];
        if (s.generation != gen) return false;

        uint32_t removeIndex = s.index;
        uint32_t lastIndex   = static_cast<uint32_t>(values.size() - 1);

        if (removeIndex != lastIndex) {
            std::swap(values[removeIndex], values[lastIndex]);

            uint32_t movedSlot = valueToSlot[lastIndex];
            valueToSlot[removeIndex] = movedSlot;
            slots[movedSlot].index = removeIndex;
        }

        values.pop_back();
        valueToSlot.pop_back();

        s.generation++;
        if (s.generation == 0) s.generation = 1;

        freeSlots.push_back(slot);

        return true;
    }

    //----------[ ACCESS ]----------//
    T* get(Id id) {
        Slot* s = findSlot(id);
        return s ? &values[s->index] : nullptr;
    }

    const T* get(Id id) const {
        const Slot* s = findSlot(id);
        return s ? &values[s->index] : nullptr;
    }

    bool exists(Id id) const { return findSlot(id) != nullptr; }
};

} // namespace mg