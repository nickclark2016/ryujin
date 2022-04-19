#ifndef slot_map_hpp__
#define slot_map_hpp__

#include "as.hpp"
#include "primitives.hpp"
#include "vector.hpp"

#include <memory>
#include <numeric>

namespace ryujin
{
    struct slot_map_key
    {
        u32 index;
        u32 generation;
    };
    static constexpr slot_map_key invalid_slot_map_key = { .index = std::numeric_limits<u32>::max(), .generation = std::numeric_limits<u32>::max() };

    inline constexpr bool operator==(const slot_map_key& lhs, const slot_map_key& rhs) noexcept
    {
        return lhs.index == rhs.index && lhs.generation == rhs.generation;
    }

    inline constexpr bool operator!=(const slot_map_key& lhs, const slot_map_key& rhs) noexcept
    {
        return lhs.index != rhs.index || lhs.generation != rhs.generation;
    }

    struct slot_map_key_hash
    {
        constexpr sz operator()(const slot_map_key& k) const noexcept;
    };

    inline constexpr sz slot_map_key_hash::operator()(const slot_map_key& k) const noexcept
    {
        sz hash = 7;
        hash = 31 * hash + k.index;
        hash = 31 * hash + k.generation;
        return hash;
    }

    template <typename T, typename KeyAllocator = std::allocator<slot_map_key>, typename ValueAllocator = std::allocator<T>, typename IndexAllocator = std::allocator<u32>>
    class slot_map
    {
    public:
        slot_map_key insert(const T& value);

        T* try_get(const slot_map_key& k) noexcept;
        const T* try_get(const slot_map_key& k) const noexcept;
        sz index_of(const slot_map_key& k) const noexcept;

        bool erase(const slot_map_key& k);
        void clear();

        sz size() const noexcept;
        sz capacity() const noexcept;

        auto begin() noexcept;
        const auto begin() const noexcept;
        const auto cbegin() const noexcept;

        auto end() noexcept;
        const auto end() const noexcept;
        const auto cend() const noexcept;

        static constexpr slot_map_key invalid = invalid_slot_map_key;

    private:
        bool _increase_capacity(const sz requested);

        vector<slot_map_key, KeyAllocator> _keys;
        vector<T, ValueAllocator> _values;
        vector<u32, IndexAllocator> _erase; // maps value index back to key index for O(1) erasure

        sz _size = 0;
        sz _capacity = 0;

        u32 _freeListHead = 0;
        u32 _freeListTail = 0;
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
        trampoline.index = as<u32>(_values.size());
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
    inline T* slot_map<T, KeyAllocator, ValueAllocator, IndexAllocator>::try_get(const slot_map_key& k) noexcept
    {
        if (k.index >= _keys.size())
        {
            return nullptr;
        }
        const auto trampoline = _keys[k.index];
        if (k.generation == trampoline.generation)
        {
            return _values.data() + trampoline.index;
        }
        return nullptr;
    }

    template<typename T, typename KeyAllocator, typename ValueAllocator, typename IndexAllocator>
    inline const T* slot_map<T, KeyAllocator, ValueAllocator, IndexAllocator>::try_get(const slot_map_key& k) const noexcept
    {
        const auto trampoline = _keys[k.index];
        if (k.generation == trampoline.generation)
        {
            return _values.data() + trampoline.index;
        }
        return nullptr;
    }

    template<typename T, typename KeyAllocator, typename ValueAllocator, typename IndexAllocator>
    inline sz slot_map<T, KeyAllocator, ValueAllocator, IndexAllocator>::index_of(const slot_map_key& k)  const noexcept
    {
        if (k == invalid)
        {
            return ~as<sz>(0);
        }
        const auto trampoline = _keys[k.index];
        if (k.generation == trampoline.generation)
        {
            return trampoline.index;
        }
        return ~as<sz>(0);
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
                // update trampoline corresponding to the value moved to point to the new value location
                auto trampolineIdx = _erase[_values.size() - 1];
                _keys[trampolineIdx].index = idxToErase;

                _values[idxToErase] = std::move(_values[_values.size() - 1]);
            }
            _values.pop_back();

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
    inline sz slot_map<T, KeyAllocator, ValueAllocator, IndexAllocator>::size() const noexcept
    {
        return _size;
    }

    template<typename T, typename KeyAllocator, typename ValueAllocator, typename IndexAllocator>
    inline sz slot_map<T, KeyAllocator, ValueAllocator, IndexAllocator>::capacity() const noexcept
    {
        return _capacity;
    }

    template<typename T, typename KeyAllocator, typename ValueAllocator, typename IndexAllocator>
    inline bool slot_map<T, KeyAllocator, ValueAllocator, IndexAllocator>::_increase_capacity(const sz requested)
    {
        if (requested > _capacity)
        {
            _keys.resize(requested);
            _values.reserve(requested);
            _erase.resize(requested);

            for (sz i = _capacity; i < requested; ++i)
            {
                slot_map_key& k = _keys[i];
                k.index = as<u32>(i) + 1;
            }

            _freeListHead = as<u32>(_size);
            _freeListTail = as<u32>(requested) - 1;
            _capacity = requested;
        }

        return true;
    }

    template<typename T, typename KeyAllocator, typename ValueAllocator, typename IndexAllocator>
    auto slot_map<T, KeyAllocator, ValueAllocator, IndexAllocator>::begin() noexcept
    {
        return _values.begin();
    }

    template<typename T, typename KeyAllocator, typename ValueAllocator, typename IndexAllocator>
    const auto slot_map<T, KeyAllocator, ValueAllocator, IndexAllocator>::begin() const noexcept
    {
        return _values.begin();
    }

    template<typename T, typename KeyAllocator, typename ValueAllocator, typename IndexAllocator>
    const auto slot_map<T, KeyAllocator, ValueAllocator, IndexAllocator>::cbegin() const noexcept
    {
        return _values.cbegin();
    }

    template<typename T, typename KeyAllocator, typename ValueAllocator, typename IndexAllocator>
    auto slot_map<T, KeyAllocator, ValueAllocator, IndexAllocator>::end() noexcept
    {
        return _values.end();
    }

    template<typename T, typename KeyAllocator, typename ValueAllocator, typename IndexAllocator>
    const auto slot_map<T, KeyAllocator, ValueAllocator, IndexAllocator>::end() const noexcept
    {
        return _values.end();
    }

    template<typename T, typename KeyAllocator, typename ValueAllocator, typename IndexAllocator>
    const auto slot_map<T, KeyAllocator, ValueAllocator, IndexAllocator>::cend() const noexcept
    {
        return _values.cend();
    }
}

#endif // slot_map_hpp__
