#ifndef sparse_map_hpp__
#define sparse_map_hpp__

#include "entity.hpp"

#include "../core/primitives.hpp"

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

namespace ryujin
{
    template <typename EntityType, typename ValueType, sz PageSize>
    class sparse_map
    {
    public:
        using key_type = EntityType;
        using value_type = ValueType;

        bool empty() const noexcept;
        sz size() const noexcept;
        sz capacity() const noexcept;
        bool contains(const key_type& key) const noexcept;
        bool contains(const key_type& key, const value_type& value);

        value_type& get(const key_type& key);
        const value_type& get(const key_type& key) const;

        void shrink_to_fit();
        void reserve(const sz count);
        
        void clear();
        bool insert(const key_type& key, const value_type& value);
        bool remove(const key_type& key);
        bool remove(const key_type& key, const value_type& value);
        bool replace(const key_type& key, const value_type& value);
        bool insert_or_replace(const key_type& key, const value_type& value); // true on replace, false on insert

        auto value_begin() noexcept;
        auto value_begin() const noexcept;
        const auto value_cbegin() const noexcept;

        auto value_end() noexcept;
        auto value_end() const noexcept;
        const auto value_cend() const noexcept;

    private:
        using sparse_page_type = EntityType[];

        sz _page(const key_type& tp) const noexcept;
        sz _offset(const key_type& tp) const noexcept;

        std::vector<std::unique_ptr<sparse_page_type>> _sparse;
        std::vector<EntityType> _packed;
        std::vector<ValueType> _values;

        static constexpr EntityType _tombstone = entity_traits<typename EntityType::type>::from_type(~(typename EntityType::type)(0));
    };
    
    template<typename EntityType, typename ValueType, sz PageSize>
    inline bool sparse_map<EntityType, ValueType, PageSize>::empty() const noexcept
    {
        return _packed.empty();
    }
    
    template<typename EntityType, typename ValueType, sz PageSize>
    inline sz sparse_map<EntityType, ValueType, PageSize>::size() const noexcept
    {
        return _packed.size();
    }
    
    template<typename EntityType, typename ValueType, sz PageSize>
    inline sz sparse_map<EntityType, ValueType, PageSize>::capacity() const noexcept
    {
        return _packed.capacity();
    }
    
    template<typename EntityType, typename ValueType, sz PageSize>
    inline bool sparse_map<EntityType, ValueType, PageSize>::contains(const key_type& key) const noexcept
    {
        const auto page = _page(key);
        const auto offset = _offset(key);

        if (page < _sparse.size())
        {
            const auto trampoline = _sparse[page][offset];
            return trampoline.identifier < _packed.size() && _packed[trampoline.identifier] == key;
        }
        return false;
    }
    
    template<typename EntityType, typename ValueType, sz PageSize>
    inline bool sparse_map<EntityType, ValueType, PageSize>::contains(const key_type& key, const value_type& value)
    {
        const auto page = _page(key);
        const auto offset = _offset(key);

        if (page < _sparse.size())
        {
            const auto trampoline = _sparse[page][offset];
            return trampoline.identifier < _packed.size() && _packed[trampoline.identifier] == key && _values[trampoline.identifier] == value;
        }
        return false;
    }

    template<typename EntityType, typename ValueType, sz PageSize>
    inline typename sparse_map<EntityType, ValueType, PageSize>::value_type& sparse_map<EntityType, ValueType, PageSize>::get(const key_type& key)
    {
        const auto page = _page(key);
        const auto offset = _offset(key);

        const auto trampoline = _sparse[page][offset];
        return _values[trampoline.identifier];
    }

    template<typename EntityType, typename ValueType, sz PageSize>
    inline const typename sparse_map<EntityType, ValueType, PageSize>::value_type& sparse_map<EntityType, ValueType, PageSize>::get(const key_type& key) const
    {
        const auto page = _page(key);
        const auto offset = _offset(key);

        const auto trampoline = _sparse[page][offset];
        return _values[trampoline.identifier];
    }
    
    template<typename EntityType, typename ValueType, sz PageSize>
    inline void sparse_map<EntityType, ValueType, PageSize>::shrink_to_fit()
    {
        _packed.shrink_to_fit();
        _sparse.shrink_to_fit();
        _values.shrink_to_fit();
    }
    
    template<typename EntityType, typename ValueType, sz PageSize>
    inline void sparse_map<EntityType, ValueType, PageSize>::reserve(const sz count)
    {
        const auto pageCount = 1 + ((count - 1) / PageSize);
        _sparse.reserve(pageCount);
        _packed.reserve(count);
        _values.reserve(count);

        for (auto i = _sparse.size(); i < pageCount; ++i)
        {
            _sparse[i] = std::make_unique<EntityType[]>(PageSize);
            for (auto j = 0; j < PageSize; ++j)
            {
                _sparse[i][j] = _tombstone;
            }
        }
    }
    
    template<typename EntityType, typename ValueType, sz PageSize>
    inline void sparse_map<EntityType, ValueType, PageSize>::clear()
    {
        _packed.clear();
        _values.clear();
        _sparse.clear();
    }
    
    template<typename EntityType, typename ValueType, sz PageSize>
    inline bool sparse_map<EntityType, ValueType, PageSize>::insert(const key_type& key, const value_type& value)
    {
        const auto sparsePage = _page(key);
        const auto sparseOffset = _offset(key);

        for (auto pg = _sparse.size(); pg <= sparsePage; ++pg)
        {
            auto page = std::make_unique<EntityType[]>(PageSize);
            for (sz i = 0; i < PageSize; ++i)
            {
                page[i] = _tombstone;
            }
            _sparse.push_back(std::move(page));
        }

        auto& page = _sparse[sparsePage];
        if (page[sparseOffset] == _tombstone)
        {
            // Value is not in the map
            page[sparseOffset] = EntityType{ static_cast<entity_traits<typename EntityType::type>::identifier_type>(_packed.size()), 0 };
            _packed.push_back(key);
            _values.push_back(value);
            return true;
        }
        return false;
    }
    
    template<typename EntityType, typename ValueType, sz PageSize>
    inline bool sparse_map<EntityType, ValueType, PageSize>::remove(const key_type& key)
    {
        const auto sparsePage = _page(key);
        const auto sparseOffset = _offset(key);

        if (sparsePage < _sparse.size())
        {
            const auto trampoline = _sparse[sparsePage][sparseOffset];
            if (trampoline == key)
            {
                const auto packedIndex = trampoline.identifier;
                _sparse[sparsePage][sparseOffset] = _tombstone;

                const auto back = _packed.back();
                _packed[packedIndex] = back;
                _packed.pop_back();

                const auto valueBack = _values.back();
                _values[packedIndex] = std::move(valueBack);
                _values.pop_back();

                const auto toMove = entity_traits<typename EntityType::type>::from_type(packedIndex);
                _sparse[_page(toMove)][_offset(toMove)] = toMove;

                return true;
            }
        }
        return false;
    }
    
    template<typename EntityType, typename ValueType, sz PageSize>
    inline bool sparse_map<EntityType, ValueType, PageSize>::remove(const key_type& key, const value_type& value)
    {
        const auto sparsePage = _page(key);
        const auto sparseOffset = _offset(key);

        if (sparsePage < _sparse.size())
        {
            const auto trampoline = _sparse[sparsePage][sparseOffset];
            if (trampoline == key)
            {
                const auto packedIndex = trampoline.identifier;

                if (_values[packedIndex] != value)
                {
                    return false;
                }

                _sparse[sparsePage][sparseOffset] = _tombstone;

                const auto back = _packed.back();
                _packed[packedIndex] = back;
                _packed.pop_back();

                const auto valueBack = _values.back();
                _values[packedIndex] = valueBack;
                _values.pop_back();

                const auto toMove = entity_traits<typename entity_traits<typename EntityType::type>::identifier_type>::from_type(packedIndex);
                _sparse[_page(toMove)][_offset(toMove)] = toMove;

                return true;
            }
        }
        return false;
    }

    template<typename EntityType, typename ValueType, sz PageSize>
    inline bool sparse_map<EntityType, ValueType, PageSize>::replace(const key_type& key, const value_type& value)
    {
        const auto sparsePage = _page(key);
        const auto sparseOffset = _offset(key);

        for (auto pg = _sparse.size(); pg <= sparsePage; ++pg)
        {
            auto page = std::make_unique<EntityType[]>(PageSize);
            for (sz i = 0; i < PageSize; ++i)
            {
                page[i] = _tombstone;
            }
            _sparse.push_back(std::move(page));
        }

        auto& page = _sparse[sparsePage];
        if (page[sparseOffset] != _tombstone)
        {
            auto trampoline = page[sparseOffset];
            _values[trampoline.identifier] = value;
            return true;
        }
        return false;
    }

    template<typename EntityType, typename ValueType, sz PageSize>
    inline bool sparse_map<EntityType, ValueType, PageSize>::insert_or_replace(const key_type& key, const value_type& value)
    {
        const auto sparsePage = _page(key);
        const auto sparseOffset = _offset(key);

        for (auto pg = _sparse.size(); pg <= sparsePage; ++pg)
        {
            auto page = std::make_unique<EntityType[]>(PageSize);
            for (sz i = 0; i < PageSize; ++i)
            {
                page[i] = _tombstone;
            }
            _sparse.push_back(std::move(page));
        }

        auto& page = _sparse[sparsePage];
        if (page[sparseOffset] == _tombstone)
        {
            // Value is not in the map
            page[sparseOffset] = EntityType{ static_cast<entity_traits<typename EntityType::type>::identifier_type>(_packed.size()), 0 };
            _packed.push_back(key);
            _values.push_back(value);
            return false;
        }
        else
        {
            auto trampoline = page[sparseOffset];
            _values[trampoline.identifier] = value;
            return true;
        }
    }

    template<typename EntityType, typename ValueType, sz PageSize>
    inline auto sparse_map<EntityType, ValueType, PageSize>::value_begin() noexcept
    {
        return _values.begin();
    }

    template<typename EntityType, typename ValueType, sz PageSize>
    inline auto sparse_map<EntityType, ValueType, PageSize>::value_begin() const noexcept
    {
        return _values.begin();
    }

    template<typename EntityType, typename ValueType, sz PageSize>
    inline const auto sparse_map<EntityType, ValueType, PageSize>::value_cbegin() const noexcept
    {
        return _values.cbegin();
    }

    template<typename EntityType, typename ValueType, sz PageSize>
    inline auto sparse_map<EntityType, ValueType, PageSize>::value_end() noexcept
    {
        return _values.end();
    }

    template<typename EntityType, typename ValueType, sz PageSize>
    inline auto sparse_map<EntityType, ValueType, PageSize>::value_end() const noexcept
    {
        return _values.end();
    }

    template<typename EntityType, typename ValueType, sz PageSize>
    inline const auto sparse_map<EntityType, ValueType, PageSize>::value_cend() const noexcept
    {
        return _values.cend();
    }

    template<typename EntityType, typename ValueType, sz PageSize>
    inline bool operator==(const sparse_map<EntityType, ValueType, PageSize>& lhs, const sparse_map<EntityType, ValueType, PageSize>& rhs) noexcept
    {
        using tp = sparse_map<EntityType, ValueType, PageSize>::key_type::type;

        if (lhs.size() == rhs.size())
        {
            for (size_t i = 0; i < lhs.size(); ++i)
            {
                const auto entity = entity_traits<tp>::from_type(static_cast<tp>(i));
                const auto left = lhs.contains(entity);
                const auto right = rhs.contains(entity);

                if (!(left && right && lhs.get(entity) == rhs.get(entity)))
                {
                    return false;
                }
            }
            return true;
        }
        return false;
    }

    template<typename EntityType, typename ValueType, sz PageSize>
    inline bool operator!=(const sparse_map<EntityType, ValueType, PageSize>& lhs, const sparse_map<EntityType, ValueType, PageSize>& rhs) noexcept
    {
        using tp = sparse_map<EntityType, ValueType, PageSize>::key_type::type;

        if (lhs.size() == rhs.size())
        {
            for (size_t i = 0; i < lhs.size(); ++i)
            {
                const auto entity = entity_traits<tp>::from_type(static_cast<tp>(i));
                const auto left = lhs.contains(entity);
                const auto right = rhs.contains(entity);

                if (!(left && right && lhs.get(entity) == rhs.get(entity)))
                {
                    return true;
                }
            }
            return false;
        }
        return true;
    }

    template<typename EntityType, typename ValueType, sz PageSize>
    inline sz sparse_map<EntityType, ValueType, PageSize>::_page(const key_type& tp) const noexcept
    {
        const auto result = tp.identifier / PageSize;
        return result;
    }

    template<typename EntityType, typename ValueType, sz PageSize>
    inline sz sparse_map<EntityType, ValueType, PageSize>::_offset(const key_type& tp) const noexcept
    {
        constexpr bool isPowerOf2 = (PageSize != 0) && ((PageSize & (PageSize - 1)) != 0);
        if constexpr (isPowerOf2)
        {
            return __builtin_ctz(tp.identifier);
        }
        else
        {
            return tp.identifier % PageSize;
        }
    }
}

#endif // sparse_map_hpp__
