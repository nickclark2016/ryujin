#ifndef unordered_map__
#define unordered_map__

#include "allocator.hpp"
#include "as.hpp"
#include "functional.hpp"
#include "utility.hpp"
#include "vector.hpp"

namespace ryujin
{
    template <typename Key, typename Value, typename Hash, typename KeyEqual, template<typename> typename Allocator>
    class unordered_map;

    namespace detail
    {
        enum class unordered_map_slot_status
        {
            VACANT = 0,
            EVICTED = 1,
            OCCUPIED = 2,
        };

        inline unordered_map_slot_status get_status(const u64 data, sz bit)
        {
            constexpr auto mask = sz(3);
            const sz value = (data >> bit) & mask;
            return as<unordered_map_slot_status>(value);
        }

        inline void set_status(u64& data, sz bit, const unordered_map_slot_status status)
        {
            const auto firstBit = as<sz>(status) & 0x1;
            const auto secondBit = (as<sz>(status) & 0x2) >> 1;

            data &= ~(as<sz>(1) << bit);
            data &= ~(as<sz>(1) << (bit + 1));
            data |= firstBit << bit;
            data |= secondBit << (bit + 1);
        }

        template <typename Key, typename Value, typename Hash, typename KeyEqual, template<typename> typename Allocator>
        class unordered_map_iterator
        {
        public:
            constexpr pair<const Key, Value>& operator*() noexcept;
            constexpr const pair<const Key, Value>& operator*() const noexcept;
            constexpr pair<const Key, Value>* operator->() const noexcept;

            // prefix
            constexpr unordered_map_iterator& operator++() noexcept;

            // postfix
            constexpr unordered_map_iterator operator++(int) noexcept;

            constexpr auto operator<=>(const unordered_map_iterator& rhs) const = default;

        private:
            friend class unordered_map<Key, Value, Hash, KeyEqual, Allocator>;

            constexpr unordered_map_iterator(const unordered_map<Key, Value, Hash, KeyEqual, Allocator>* parent, sz page, sz index);

            const unordered_map<Key, Value, Hash, KeyEqual, Allocator>* _parent;
            sz _page;
            sz _index;
        };
    };

    template <typename Key, typename Value, typename Hash = hash<Key>, typename KeyEqual = equal_to<Key>, template<typename> typename Allocator = ryujin::allocator>
    class unordered_map
    {
        static constexpr sz values_per_page = 32;

        struct page
        {
            u64 tags;
            pair<const Key, Value> values[values_per_page];
        };

    public:
        using iterator = detail::unordered_map_iterator<Key, Value, Hash, KeyEqual, Allocator>;

        constexpr iterator begin() noexcept;
        constexpr const iterator begin() const noexcept;
        constexpr const iterator cbegin() const noexcept;

        constexpr iterator end() noexcept;
        constexpr const iterator end() const noexcept;
        constexpr const iterator cend() const noexcept;

        constexpr bool contains(const Key& k) const noexcept;
        constexpr iterator find(const Key& k) noexcept;
        constexpr const iterator find(const Key& k) const noexcept;

        constexpr iterator insert(const pair<const Key, Value>& value);
        constexpr iterator insert(pair<const Key, Value>&& value);

        constexpr void clear();
        constexpr void erase(const Key& k);

        constexpr sz size() const noexcept;
        constexpr sz capacity() const noexcept;
        constexpr bool empty() const noexcept;

        constexpr Value& operator[](const Key& k) noexcept;
    private:
        Hash _hasher;
        KeyEqual _equality;
        Allocator<page> _pageAllocator;

        page* _pages = nullptr;
        sz _size = 0;
        sz _capacity = 0;
        sz _pageCount = 0;

        constexpr bool _needs_resize() const noexcept;
        constexpr void _make_new_pages(const sz newCapacity);
        constexpr bool _is_occupied(const sz page, const sz el) const noexcept;

        friend class detail::unordered_map_iterator<Key, Value, Hash, KeyEqual, Allocator>;
    };
    
    template<typename Key, typename Value, typename Hash, typename KeyEqual, template <typename> typename Allocator>
    inline constexpr typename unordered_map<Key, Value, Hash, KeyEqual, Allocator>::iterator unordered_map<Key, Value, Hash, KeyEqual, Allocator>::begin() noexcept
    {
        if (empty())
        {
            return end();
        }

        sz page = 0;
        sz index = 0;

        while (_pages != nullptr && detail::get_status(_pages[page].tags, index * 2) != detail::unordered_map_slot_status::OCCUPIED)
        {
            ++index;
            if (index == values_per_page)
            {
                ++page;
                index = 0;
            }
        }

        return iterator(this, page, index);
    }

    template<typename Key, typename Value, typename Hash, typename KeyEqual, template <typename> typename Allocator>
    inline constexpr const typename unordered_map<Key, Value, Hash, KeyEqual, Allocator>::iterator unordered_map<Key, Value, Hash, KeyEqual, Allocator>::begin() const noexcept
    {
        if (empty())
        {
            return end();
        }

        sz page = 0;
        sz index = 0;

        while (_pages != nullptr && detail::get_status(_pages[page].tags, index * 2) != detail::unordered_map_slot_status::OCCUPIED)
        {
            ++index;
            if (index == values_per_page)
            {
                ++page;
                index = 0;
            }
        }

        return iterator(this, page, index);
    }

    template<typename Key, typename Value, typename Hash, typename KeyEqual, template <typename> typename Allocator>
    inline constexpr const typename unordered_map<Key, Value, Hash, KeyEqual, Allocator>::iterator unordered_map<Key, Value, Hash, KeyEqual, Allocator>::cbegin() const noexcept
    {
        if (empty())
        {
            return end();
        }

        sz page = 0;
        sz index = 0;

        while (_pages != nullptr && detail::get_status(_pages[page].tags, index * 2) != detail::unordered_map_slot_status::OCCUPIED)
        {
            ++index;
            if (index == values_per_page)
            {
                ++page;
                index = 0;
            }
        }

        return iterator(this, page, index);
    }

    template<typename Key, typename Value, typename Hash, typename KeyEqual, template <typename> typename Allocator>
    inline constexpr typename unordered_map<Key, Value, Hash, KeyEqual, Allocator>::iterator unordered_map<Key, Value, Hash, KeyEqual, Allocator>::end() noexcept
    {
        return iterator(this, _pageCount, 0);
    }

    template<typename Key, typename Value, typename Hash, typename KeyEqual, template <typename> typename Allocator>
    inline constexpr const typename unordered_map<Key, Value, Hash, KeyEqual, Allocator>::iterator unordered_map<Key, Value, Hash, KeyEqual, Allocator>::end() const noexcept
    {
        return iterator(this, _pageCount, 0);
    }

    template<typename Key, typename Value, typename Hash, typename KeyEqual, template <typename> typename Allocator>
    inline constexpr const typename unordered_map<Key, Value, Hash, KeyEqual, Allocator>::iterator unordered_map<Key, Value, Hash, KeyEqual, Allocator>::cend() const noexcept
    {
        return iterator(this, _pageCount, 0);
    }

    template<typename Key, typename Value, typename Hash, typename KeyEqual, template <typename> typename Allocator>
    inline constexpr bool unordered_map<Key, Value, Hash, KeyEqual, Allocator>::contains(const Key& k) const noexcept
    {
        return find(k) != end();
    }

    template <typename Key, typename Value, typename Hash, typename KeyEqual, template <typename> typename Allocator>
    inline constexpr typename unordered_map<Key, Value, Hash, KeyEqual, Allocator>::iterator unordered_map<Key, Value, Hash, KeyEqual, Allocator>::find(const Key& k) noexcept
    {
        if (_size == 0)
        {
            return end();
        }

        auto hash = _hasher(k) % _capacity;
        auto page = hash / values_per_page;
        auto index = hash % values_per_page;

        for (auto status = detail::get_status(_pages[page].tags, index * 2); status != detail::unordered_map_slot_status::VACANT; )
        {
            if (status == detail::unordered_map_slot_status::OCCUPIED)
            {
                if (_equality(_pages[page].values[index].first, k))
                {
                    return detail::unordered_map_iterator(this, page, index);
                }
            }

            ++index;
            if (index == values_per_page)
            {
                index = 0;
                page = (page + 1) % _pageCount;
            }
        }

        return end();
    }

    template <typename Key, typename Value, typename Hash, typename KeyEqual, template <typename> typename Allocator>
    inline constexpr const typename unordered_map<Key, Value, Hash, KeyEqual, Allocator>::iterator unordered_map<Key, Value, Hash, KeyEqual, Allocator>::find(const Key& k) const noexcept
    {
        if (_size == 0)
        {
            return end();
        }

        auto hash = _hasher(k) % _capacity;
        auto page = hash / values_per_page;
        auto index = hash % values_per_page;

        for (auto status = detail::get_status(_pages[page].tags, index * 2); status != detail::unordered_map_slot_status::VACANT; )
        {
            if (status == detail::unordered_map_slot_status::OCCUPIED)
            {
                if (_equality(_pages[page].values[index].first, k))
                {
                    return detail::unordered_map_iterator(this, page, index);
                }
            }

            ++index;
            if (index == values_per_page)
            {
                index = 0;
                page = (page + 1) % _pageCount;
            }
        }

        return end();
    }

    template <typename Key, typename Value, typename Hash, typename KeyEqual, template <typename> typename Allocator>
    inline constexpr typename unordered_map<Key, Value, Hash, KeyEqual, Allocator>::iterator unordered_map<Key, Value, Hash, KeyEqual, Allocator>::insert(const pair<const Key, Value>& value)
    {
        if (_needs_resize())
        {
            _make_new_pages((_pageCount + 1) * values_per_page);
        }

        auto hash = _hasher(value.first) % _capacity;

        auto page = hash / values_per_page;
        auto index = hash % values_per_page;

        while (_is_occupied(page, index))
        {
            if (_equality(_pages[page].values[index].first, value.first))
            {
                return end();
            }

            ++index;
            if (index == values_per_page)
            {
                index = 0;
                page = (page + 1) % _pageCount;
            }
        }

        ::new (&(_pages[page].values[index])) pair<const Key, Value>(value);
        detail::set_status(_pages[page].tags, 2 * index, detail::unordered_map_slot_status::OCCUPIED);
        ++_size;

        return iterator(this, page, index);
    }

    template <typename Key, typename Value, typename Hash, typename KeyEqual, template <typename> typename Allocator>
    inline constexpr typename unordered_map<Key, Value, Hash, KeyEqual, Allocator>::iterator unordered_map<Key, Value, Hash, KeyEqual, Allocator>::insert(pair<const Key, Value>&& value)
    {
        if (_needs_resize())
        {
            _make_new_pages((_pageCount + 1) * values_per_page);
        }

        auto hash = _hasher(value.first) % _capacity;

        auto page = hash / values_per_page;
        auto index = hash % values_per_page;

        while (_is_occupied(page, index))
        {
            if (_equality(_pages[page].values[index].first, value.first))
            {
                return end();
            }

            ++index;
            if (index == values_per_page)
            {
                index = 0;
                page = (page + 1) % _pageCount;
            }
        }

        ::new (&(_pages[page].values[index])) pair<const Key, Value>(ryujin::move(value));
        detail::set_status(_pages[page].tags, 2 * index, detail::unordered_map_slot_status::OCCUPIED);
        ++_size;

        return iterator(this, page, index);
    }

    template <typename Key, typename Value, typename Hash, typename KeyEqual, template <typename> typename Allocator>
    inline constexpr void unordered_map<Key, Value, Hash, KeyEqual, Allocator>::clear()
    {
        for (sz pg = 0; pg < _pageCount; ++pg)
        {
            for (sz el = 0; el < values_per_page; ++el)
            {
                const auto status = detail::get_status(_pages[pg].tags, el * 2);
                if (status == detail::unordered_map_slot_status::OCCUPIED)
                {
                    _pages[pg].values[el].~pair<const Key, Value>();
                }
            }
            _pages[pg].tags = 0;
        }
        _size = 0;
    }

    template <typename Key, typename Value, typename Hash, typename KeyEqual, template <typename> typename Allocator>
    inline constexpr void unordered_map<Key, Value, Hash, KeyEqual, Allocator>::erase(const Key& k)
    {
        if (_size == 0)
        {
            return;
        }

        auto hash = _hasher(k) % _capacity;
        auto page = hash / values_per_page;
        auto index = hash % values_per_page;

        for (auto status = detail::get_status(_pages[page].tags, index * 2); status != detail::unordered_map_slot_status::VACANT; )
        {
            if (status == detail::unordered_map_slot_status::OCCUPIED)
            {
                if (_equality(_pages[page].values[index].first, k))
                {
                    detail::set_status(_pages[page].tags, index * 2, detail::unordered_map_slot_status::EVICTED);
                    _pages[page].values[index].~pair<const Key, Value>();
                    --_size;
                    return;
                }
            }

            ++index;
            if (index == values_per_page)
            {
                index = 0;
                page = (page + 1) % _pageCount;
            }
        }
    }

    template <typename Key, typename Value, typename Hash, typename KeyEqual, template <typename> typename Allocator>
    inline constexpr sz unordered_map<Key, Value, Hash, KeyEqual, Allocator>::size() const noexcept
    {
        return _size;
    }

    template<typename Key, typename Value, typename Hash, typename KeyEqual, template <typename> typename Allocator>
    inline constexpr sz unordered_map<Key, Value, Hash, KeyEqual, Allocator>::capacity() const noexcept
    {
        return _capacity;
    }

    template <typename Key, typename Value, typename Hash, typename KeyEqual, template <typename> typename Allocator>
    inline constexpr bool unordered_map<Key, Value, Hash, KeyEqual, Allocator>::empty() const noexcept
    {
        return _size == 0;
    }

    template <typename Key, typename Value, typename Hash, typename KeyEqual, template <typename> typename Allocator>
    inline constexpr Value& unordered_map<Key, Value, Hash, KeyEqual, Allocator>::operator[](const Key& k) noexcept
    {
        const auto search = find(k);
        if (search == end())
        {
            if (_needs_resize())
            {
                _make_new_pages((_pageCount + 1) * values_per_page);
            }

            auto hash = _hasher(k) % _capacity;

            auto page = hash / values_per_page;
            auto index = hash % values_per_page;

            while (_is_occupied(page, index))
            {
                ++index;
                if (index == values_per_page)
                {
                    index = 0;
                    page = (page + 1) % _pageCount;
                }
            }

            ::new (&(_pages[page].values[index])) pair<const Key, Value>(k, ryujin::move(Value()));
            detail::set_status(_pages[page].tags, 2 * index, detail::unordered_map_slot_status::OCCUPIED);
            ++_size;
            return _pages[page].values[index].second;
        }
        return search->second;
    }

    template <typename Key, typename Value, typename Hash, typename KeyEqual, template <typename> typename Allocator>
    inline constexpr bool unordered_map<Key, Value, Hash, KeyEqual, Allocator>::_needs_resize() const noexcept
    {
        const auto residence = as<double>(_size) / as<double>(_capacity);
        // needs resize if the residence is at least 70%
        return _capacity == 0 || residence >= 0.7;
    }

    template <typename Key, typename Value, typename Hash, typename KeyEqual, template <typename> typename Allocator>
    inline constexpr void unordered_map<Key, Value, Hash, KeyEqual, Allocator>::_make_new_pages(const sz newCapacity)
    {
        if (newCapacity <= _capacity)
        {
            return;
        }

        const sz pageCount = newCapacity / values_per_page;
        page* allocation = _pageAllocator.allocate(pageCount);

        // clear the allocation tags
        for (sz i = 0; i < pageCount; ++i)
        {
            allocation[i].tags = 0;
        }

        // move the old values
        const sz oldPageCount = _capacity / values_per_page;
        for (sz i = 0; i < oldPageCount; ++i)
        {
            const auto pageAddr = _pages + i;
            for (sz el = 0; el < values_per_page; ++el)
            {
                const bool occupied = _is_occupied(i, el);
                if (occupied)
                {
                    const auto hash = _hasher(_pages[i].values[el].first);
                    auto newPage = hash / values_per_page;
                    auto newEl = hash % values_per_page;

                    while (detail::get_status(allocation[i].tags, newEl * 2) == detail::unordered_map_slot_status::OCCUPIED)
                    {
                        ++newEl;
                        if (newEl == values_per_page)
                        {
                            newPage = (newPage + 1) % pageCount;
                            newEl = 0;
                        }
                    }

                    ::new(&(allocation[newPage].values[newEl])) pair<const Key, Value>(ryujin::move(_pages[i].values[el]));
                    detail::set_status(allocation[newPage].tags, newEl * 2, detail::unordered_map_slot_status::OCCUPIED);
                    _pages[i].values[el].~pair<const Key, Value>();
                }
            }
        }

        for (sz i = oldPageCount; i < pageCount; ++i)
        {
            allocation[i].tags = 0;
        }

        if (_pages)
        {
            _pageAllocator.deallocate(_pages, _pageCount);
        }

        _capacity = newCapacity;
        _pageCount = pageCount;
        _pages = allocation;
    }
    
    template <typename Key, typename Value, typename Hash, typename KeyEqual, template <typename> typename Allocator>
    inline constexpr bool unordered_map<Key, Value, Hash, KeyEqual, Allocator>::_is_occupied(const sz page, const sz el) const noexcept
    {
        auto& pg = _pages[page];
        auto status = detail::get_status(pg.tags, el * 2); // 2 bits per element
        return status == detail::unordered_map_slot_status::OCCUPIED;
    }

    namespace detail
    {
        template <typename Key, typename Value, typename Hash, typename KeyEqual, template <typename> typename Allocator>
        inline constexpr pair<const Key, Value>& unordered_map_iterator<Key, Value, Hash, KeyEqual, Allocator>::operator*() noexcept
        {
            return _parent->_pages[_page].values[_index];
        }

        template <typename Key, typename Value, typename Hash, typename KeyEqual, template <typename> typename Allocator>
        inline constexpr const pair<const Key, Value>& unordered_map_iterator<Key, Value, Hash, KeyEqual, Allocator>::operator*() const noexcept
        {
            return _parent->_pages[_page].values[_index];
        }

        template <typename Key, typename Value, typename Hash, typename KeyEqual, template <typename> typename Allocator>
        inline constexpr pair<const Key, Value>* unordered_map_iterator<Key, Value, Hash, KeyEqual, Allocator>::operator->() const noexcept
        {
            return &(_parent->_pages[_page].values[_index]);
        }

        template <typename Key, typename Value, typename Hash, typename KeyEqual, template <typename> typename Allocator>
        inline constexpr unordered_map_iterator<Key, Value, Hash, KeyEqual, Allocator>& unordered_map_iterator<Key, Value, Hash, KeyEqual, Allocator>::operator++() noexcept
        {
            do
            {
                _index++;
                if (_index == unordered_map<Key, Value, Hash, KeyEqual, Allocator>::values_per_page)
                {
                    _index = 0;
                    ++_page;
                }
                if (_page >= _parent->_pageCount)
                {
                    break;
                }
            } while (detail::get_status(_parent->_pages[_page].tags, _index * 2) != detail::unordered_map_slot_status::OCCUPIED);
            return *this;
        }

        template <typename Key, typename Value, typename Hash, typename KeyEqual, template <typename> typename Allocator>
        inline constexpr unordered_map_iterator<Key, Value, Hash, KeyEqual, Allocator> unordered_map_iterator<Key, Value, Hash, KeyEqual, Allocator>::operator++(int) noexcept
        {
            auto copy = *this;
            operator++();
            return copy;
        }

        template <typename Key, typename Value, typename Hash, typename KeyEqual, template <typename> typename Allocator>
        inline constexpr unordered_map_iterator<Key, Value, Hash, KeyEqual, Allocator>::unordered_map_iterator(const unordered_map<Key, Value, Hash, KeyEqual, Allocator>* parent, sz page, sz index)
            : _parent(parent), _page(page), _index(index)
        {
        }
    }
}

#endif // unordered_map__
