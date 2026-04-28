#pragma once

#include <vector>
#include <cstdint>

namespace mg {

template<typename T>
class HandlePool {
    public:
        using Id = uint32_t;

        using Reference = T&;
        using ConstReference = const T&;

        using Pointer = T*;
        using ConstPointer = const T*;
        using Iterator = typename std::vector<T>::iterator;
        using ConstIterator = typename std::vector<T>::const_iterator;
        
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

        static Id makeId(uint32_t slot, uint32_t gen) {
            return (gen << INDEX_BITS) | slot;
        }

        static uint32_t getSlot(Id id) { return id & INDEX_MASK; }
        static uint32_t getGeneration(Id id) { return id >> INDEX_BITS; }

    public:
        //----------[ CONSTRUCTOR + DESTRUCTOR ]----------//
        HandlePool() = default;
        ~HandlePool() = default;

        //----------[ ITERATORS ]----------//
        auto begin() { return values.begin();  }
        auto begin() const { return values.begin(); }

        auto end() { return values.end() ;  }
        auto end() const { return values.end(); }

        //----------[ CAPACITY ]----------//
        size_t size() const { return values.size(); }
        bool empty() const { return !size(); }

        //----------[ MODIFIERS ]----------//
        void clear() { 
            values.clear();
            slots.clear();
            valueToSlot.clear();
            freeSlots.clear();
        }

        Id add(T value) {
            uint32_t slotIndex;

            if (!freeSlots.empty()) {
                slotIndex = freeSlots.back();
                freeSlots.pop_back();
            } else {
                slotIndex = slots.size();
                slots.push_back({0, 0});
            }

            uint32_t valueIndex = values.size();

            values.push_back(std::move(value));
            valueToSlot.push_back(slotIndex);

            slots[slotIndex].index = valueIndex;

            return makeId(slotIndex, slots[slotIndex].generation);
        }

        template<typename... Args>
        Id emplace(Args&&... args) {
            uint32_t slotIndex;

            if (!freeSlots.empty()) {
                slotIndex = freeSlots.back();
                freeSlots.pop_back();
            } else {
                slotIndex = slots.size();
                slots.push_back({0, 0});
            }

            uint32_t valueIndex = values.size();

            values.emplace_back(std::forward<Args>(args)...);
            valueToSlot.push_back(slotIndex);

            slots[slotIndex].index = valueIndex;

            return makeId(slotIndex, slots[slotIndex].generation);
        }

        bool remove(Id id) {
            uint32_t slot = getSlot(id);
            uint32_t gen  = getGeneration(id);

            if (slot >= slots.size()) return false;

            Slot& s = slots[slot];
            if (s.generation != gen) return false;

            uint32_t removeIndex = s.index;
            uint32_t lastIndex   = values.size() - 1;

            if (removeIndex != lastIndex) {
                std::swap(values[removeIndex], values[lastIndex]);

                uint32_t movedSlot = valueToSlot[lastIndex];
                valueToSlot[removeIndex] = movedSlot;
                slots[movedSlot].index = removeIndex;
            }

            values.pop_back();
            valueToSlot.pop_back();

            s.generation++;
            freeSlots.push_back(slot);

            return true;
        }

        //----------[ ACCESS ]----------//
        Pointer get(Id id) {
            uint32_t slot = getSlot(id);
            uint32_t gen  = getGeneration(id);

            if (slot >= slots.size()) return nullptr;

            Slot& s = slots[slot];

            if (s.generation != gen) return nullptr;

            return &values[s.index];
        }
};

}
