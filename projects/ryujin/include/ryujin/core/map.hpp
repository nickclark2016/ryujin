#ifndef map_hpp__
#define map_hpp__

#include "allocator.hpp"
#include "tree.hpp"
#include "utility.hpp"

namespace ryujin
{
    namespace detail
    {
        template <typename K, typename V, typename Less>
        struct pair_key_less_than_override
        {
            constexpr bool operator()(const pair<const K, V>& lhs, const pair<const K, V>& rhs) const noexcept;
            constexpr bool operator()(const K& lhs, const pair<const K, V>& rhs) const noexcept;
            constexpr bool operator()(const pair<const K, V>& lhs, const K& rhs) const noexcept;

            Less comparator{};
        };

        template <typename K, typename V, typename Equal>
        struct pair_key_equality_override
        {
            constexpr bool operator()(const pair<const K, V>& lhs, const pair<const K, V>& rhs) const noexcept;
            constexpr bool operator()(const K& lhs, const pair<const K, V>& rhs) const noexcept;
            constexpr bool operator()(const pair<const K, V>& lhs, const K& rhs) const noexcept;

            Equal comparator{};
        };
        
        template<typename K, typename V, typename Less>
        inline constexpr bool pair_key_less_than_override<K, V, Less>::operator()(const pair<const K, V>& lhs, const pair<const K, V>& rhs) const noexcept
        {
            return comparator(lhs.first, rhs.first);
        }
        
        template<typename K, typename V, typename Less>
        inline constexpr bool pair_key_less_than_override<K, V, Less>::operator()(const K& lhs, const pair<const K, V>& rhs) const noexcept
        {
            return comparator(lhs, rhs.first);
        }
        
        template<typename K, typename V, typename Less>
        inline constexpr bool pair_key_less_than_override<K, V, Less>::operator()(const pair<const K, V>& lhs, const K& rhs) const noexcept
        {
            return comparator(lhs.first, rhs);
        }

        template<typename K, typename V, typename Less>
        inline constexpr bool pair_key_equality_override<K, V, Less>::operator()(const pair<const K, V>& lhs, const pair<const K, V>& rhs) const noexcept
        {
            return comparator(lhs.first, rhs.first);
        }

        template<typename K, typename V, typename Less>
        inline constexpr bool pair_key_equality_override<K, V, Less>::operator()(const K& lhs, const pair<const K, V>& rhs) const noexcept
        {
            return comparator(lhs, rhs.first);
        }

        template<typename K, typename V, typename Less>
        inline constexpr bool pair_key_equality_override<K, V, Less>::operator()(const pair<const K, V>& lhs, const K& rhs) const noexcept
        {
            return comparator(lhs.first, rhs);
        }
    }

    template <typename K, typename V, template <typename> typename Allocator = allocator, typename Less = less<K>, typename Equals = equal_to<K>>
    class map
    {
    public:
        using less_than = detail::pair_key_less_than_override<K, V, Less>;
        using equality = detail::pair_key_equality_override<K, V, Equals>;

        using pair_type = pair<const K, V>;
        using storage_type = rb_tree<pair_type, Allocator, less_than, equality, false>;

        using iterator = typename storage_type::iterator;
        using const_iterator = typename storage_type::const_iterator;

        iterator begin() noexcept;
        const_iterator begin() const noexcept;
        const_iterator cbegin() const noexcept;

        iterator end() noexcept;
        const_iterator end() const noexcept;
        const_iterator cend() const noexcept;

        bool contains(const K& k) const noexcept;
        iterator find(const K& k) noexcept;
        const_iterator find(const K& k) const noexcept;

        iterator insert(const pair<const K, V>& value);
        iterator insert(pair<const K, V>&& value);

        void clear();
        iterator erase(const K& k);
        iterator erase(iterator it);

        sz size() const noexcept;
        bool empty() const noexcept;

        V operator[](const K& k);

    private:
        storage_type _storage;

    };
    
    template <typename K, typename V, template <typename> typename Allocator, typename Less, typename Equals>
    inline map<K, V, Allocator, Less, Equals>::iterator map<K, V, Allocator, Less, Equals>::begin() noexcept
    {
        return _storage.begin();
    }

    template <typename K, typename V, template <typename> typename Allocator, typename Less, typename Equals>
    inline map<K, V, Allocator, Less, Equals>::const_iterator map<K, V, Allocator, Less, Equals>::begin() const noexcept
    {
        return _storage.begin();
    }

    template <typename K, typename V, template <typename> typename Allocator, typename Less, typename Equals>
    inline map<K, V, Allocator, Less, Equals>::const_iterator map<K, V, Allocator, Less, Equals>::cbegin() const noexcept
    {
        return _storage.cbegin();
    }

    template <typename K, typename V, template <typename> typename Allocator, typename Less, typename Equals>
    inline map<K, V, Allocator, Less, Equals>::iterator map<K, V, Allocator, Less, Equals>::end() noexcept
    {
        return _storage.end();
    }

    template <typename K, typename V, template <typename> typename Allocator, typename Less, typename Equals>
    inline map<K, V, Allocator, Less, Equals>::const_iterator map<K, V, Allocator, Less, Equals>::end() const noexcept
    {
        return _storage.end();
    }

    template <typename K, typename V, template <typename> typename Allocator, typename Less, typename Equals>
    inline map<K, V, Allocator, Less, Equals>::const_iterator map<K, V, Allocator, Less, Equals>::cend() const noexcept
    {
        return _storage.cend();
    }

    template <typename K, typename V, template <typename> typename Allocator, typename Less, typename Equals>
    inline bool map<K, V, Allocator, Less, Equals>::contains(const K& k) const noexcept
    {
        return find(k) != cend();
    }

    template <typename K, typename V, template <typename> typename Allocator, typename Less, typename Equals>
    inline map<K, V, Allocator, Less, Equals>::iterator map<K, V, Allocator, Less, Equals>::find(const K& k) noexcept
    {
        return _storage.find(k);
    }

    template <typename K, typename V, template <typename> typename Allocator, typename Less, typename Equals>
    inline map<K, V, Allocator, Less, Equals>::const_iterator map<K, V, Allocator, Less, Equals>::find(const K& k) const noexcept
    {
        return _storage.find(k);
    }

    template <typename K, typename V, template <typename> typename Allocator, typename Less, typename Equals>
    inline map<K, V, Allocator, Less, Equals>::iterator map<K, V, Allocator, Less, Equals>::insert(const pair<const K, V>& value)
    {
        return _storage.insert(value);
    }

    template <typename K, typename V, template <typename> typename Allocator, typename Less, typename Equals>
    inline map<K, V, Allocator, Less, Equals>::iterator map<K, V, Allocator, Less, Equals>::insert(pair<const K, V>&& value)
    {
        return _storage.insert(ryujin::forward<pair<const K, V>>(value));
    }

    template <typename K, typename V, template <typename> typename Allocator, typename Less, typename Equals>
    inline void map<K, V, Allocator, Less, Equals>::clear()
    {
        _storage.clear();
    }

    template <typename K, typename V, template <typename> typename Allocator, typename Less, typename Equals>
    inline map<K, V, Allocator, Less, Equals>::iterator map<K, V, Allocator, Less, Equals>::erase(const K& k)
    {
        auto it = find(k);
        return erase(it);
    }

    template <typename K, typename V, template <typename> typename Allocator, typename Less, typename Equals>
    inline map<K, V, Allocator, Less, Equals>::iterator map<K, V, Allocator, Less, Equals>::erase(iterator it)
    {
        return _storage.erase(it);
    }

    template <typename K, typename V, template <typename> typename Allocator, typename Less, typename Equals>
    inline sz map<K, V, Allocator, Less, Equals>::size() const noexcept
    {
        return _storage.size();
    }

    template <typename K, typename V, template <typename> typename Allocator, typename Less, typename Equals>
    inline bool map<K, V, Allocator, Less, Equals>::empty() const noexcept
    {
        return _storage.empty();
    }
    
    template <typename K, typename V, template <typename> typename Allocator, typename Less, typename Equals>
    inline V map<K, V, Allocator, Less, Equals>::operator[](const K& k)
    {
        auto it = find(k);
        if (it == end())
        {
            it = insert(make_pair(k, V()));
        }
        return it->second;
    }
}

#endif // map_hpp__
