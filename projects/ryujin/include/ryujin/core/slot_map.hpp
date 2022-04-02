#ifndef slot_map_hpp__
#define slot_map_hpp__

#include "as.hpp"
#include "vector.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <numeric>

namespace ryujin
{
    struct slot_map_key
    {
        std::uint32_t index;
        std::uint32_t generation;
    };

    inline constexpr bool operator==(const slot_map_key& lhs, const slot_map_key& rhs) noexcept
    {
        return lhs.index == rhs.index && lhs.generation == rhs.generation;
    }

    inline constexpr bool operator!=(const slot_map_key& lhs, const slot_map_key& rhs) noexcept
    {
        return lhs.index != rhs.index || lhs.generation != rhs.generation;
    }

    template <typename T, typename KeyAllocator = std::allocator<slot_map_key>, typename ValueAllocator = std::allocator<T>, typename IndexAllocator = std::allocator<std::uint32_t>>
    class slot_map
    {
    public:
        slot_map_key insert(const T& value);

        T* try_get(const slot_map_key& k);

        bool erase(const slot_map_key& k);
        void clear();

        std::size_t size() const noexcept;
        std::size_t capacity() const noexcept;

        static constexpr slot_map_key invalid = { .index = std::numeric_limits<std::uint32_t>::max(), .generation = std::numeric_limits<std::uint32_t>::max() };

    private:
        bool _increase_capacity(const std::size_t requested);

        vector<slot_map_key, KeyAllocator> _keys;
        vector<T, ValueAllocator> _values;
        vector<std::uint32_t, IndexAllocator> _erase; // maps value index back to key index for O(1) erasure

        std::size_t _size = 0;
        std::size_t _capacity = 0;

        std::uint32_t _freeListHead = 0;
        std::uint32_t _freeListTail = 0;
    };

    template<typename T, typename KeyAllocator, typename ValueAllocator, typename IndexAllocator>
    inline slot_map_key slot_map<T, KeyAllocator, ValueAllocator, IndexAllocator>::insert(const T& value)
    {
        if (_size == _capacity)
        {
            _increase_capacity(_capacity == 0 ? 8 : _capacity * 2);
        }

        // pop head of free list
        const auto head = _freeListHead;
        const auto next = _keys[head].index;
        _freeListHead = next;

        // point trampoline to inserted value
        slot_map_key& trampoline = _keys[head];
        trampoline.index = as<std::uint32_t>(_values.size());
        _values.push_back(value);

        // point erasure lookup to trampoline index
        _erase[trampoline.index] = head;

        ++_size;

        return slot_map_key{
            .index = head,
            .generation = trampoline.generation
        };
    }

    template<typename T, typename KeyAllocator, typename ValueAllocator, typename IndexAllocator>
    inline T* slot_map<T, KeyAllocator, ValueAllocator, IndexAllocator>::try_get(const slot_map_key& k)
    {
        const auto trampoline = _keys[k.index];
        if (k.generation == trampoline.generation)
        {
            return _values.data() + trampoline.index;
        }
        return nullptr;
    }


    template<typename T, typename KeyAllocator, typename ValueAllocator, typename IndexAllocator>
    inline bool slot_map<T, KeyAllocator, ValueAllocator, IndexAllocator>::erase(const slot_map_key& k)
    {
        const auto trampoline = _keys[k.index];
        if (k.generation == trampoline.generation)
        {
            const auto idxToErase = trampoline.index;
            
            // if not the last element, move the last element into the vacated slot
            if (idxToErase != _values.size() - 1)
            {
                _values[idxToErase] = std::move(_values[_values.size() - 1]);
            }
            _values.pop_back();

            // update trampoline corresponding to the value moved to point to the new value location
            auto trampolineIdx = _erase[idxToErase];
            _keys[trampolineIdx].index = idxToErase;

            // add the opened trampoline slot to the front of the free list and icrement the generation
            auto next = _freeListHead;
            _keys[k.index].index = next;
            ++_keys[k.index].generation;
            _freeListHead = k.index;

            --_size;

            return true;
        }
        return false;
    }

    template<typename T, typename KeyAllocator, typename ValueAllocator, typename IndexAllocator>
    inline void slot_map<T, KeyAllocator, ValueAllocator, IndexAllocator>::clear()
    {
        _values.clear();
        _freeListHead = 0;
        _freeListTail = _capacity - 1;

        for (size_t i = 0; i < _capacity; ++i)
        {
            _keys[i].index = i + 1;
            ++_keys[i].generation;
        }
        _size = 0;
    }

    template<typename T, typename KeyAllocator, typename ValueAllocator, typename IndexAllocator>
    inline std::size_t slot_map<T, KeyAllocator, ValueAllocator, IndexAllocator>::size() const noexcept
    {
        return _size;
    }

    template<typename T, typename KeyAllocator, typename ValueAllocator, typename IndexAllocator>
    inline std::size_t slot_map<T, KeyAllocator, ValueAllocator, IndexAllocator>::capacity() const noexcept
    {
        return _capacity;
    }

    template<typename T, typename KeyAllocator, typename ValueAllocator, typename IndexAllocator>
    inline bool slot_map<T, KeyAllocator, ValueAllocator, IndexAllocator>::_increase_capacity(const std::size_t requested)
    {
        if (requested > _capacity)
        {
            _keys.resize(requested);
            _values.reserve(requested);
            _erase.resize(requested);

            for (std::size_t i = _capacity; i < requested; ++i)
            {
                slot_map_key& k = _keys[i];
                k.index = as<std::uint32_t>(i) + 1;
            }

            _freeListHead = as<std::uint32_t>(_size);
            _freeListTail = as<std::uint32_t>(requested) - 1;
            _capacity = requested;
        }

        return true;
    }
}

#endif // slot_map_hpp__