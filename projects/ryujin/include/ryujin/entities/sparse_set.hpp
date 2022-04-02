#ifndef sparse_set_hpp__
#define sparse_set_hpp__

#include <cstddef>
#include <memory>
#include <vector>

namespace ryujin
{

    template <typename EntityType, std::size_t PageSize>
    class sparse_set
    {
    public:
        using type = EntityType;

        sparse_set() = default;
        sparse_set(const sparse_set&) = delete;
        sparse_set(sparse_set&&) noexcept = default;
        ~sparse_set() = default;

        sparse_set& operator=(const sparse_set&) = delete;
        sparse_set& operator=(sparse_set&&) noexcept = default;

        bool empty() const noexcept;
        std::size_t size() const noexcept;
        std::size_t capacity() const noexcept;
        bool contains(const type& tp) const noexcept;
        
        void shrink_to_fit();

        void clear();
        void insert(const type& tp);
        void remove(const type& tp);

        void reserve(const std::size_t count);
    private:
        using sparse_page_type = EntityType[];

        std::vector<std::unique_ptr<sparse_page_type>> _sparse;
        std::vector<EntityType> _packed;

        std::size_t _page(const type& tp) const noexcept;
        std::size_t _offset(const type& tp) const noexcept;

        static constexpr type _tombstone = entity_traits<EntityType::type>::from_type(~EntityType::type(0));
    };

    template <typename EntityType, std::size_t PageSize>
    bool operator==(const sparse_set<EntityType, PageSize>& lhs, const sparse_set<EntityType, PageSize>& rhs) noexcept
    {
        using tp = sparse_set<EntityType, PageSize>::type::type;

        if (lhs.size() == rhs.size())
        {
            for (size_t i = 0; i < lhs.size(); ++i)
            {
                const auto entity = entity_traits<tp>::from_type(static_cast<tp>(i));
                const auto left = lhs.contains(entity);
                const auto right = rhs.contains(entity);

                if (left != right)
                {
                    return false;
                }
            }
            return true;
        }
        return false;
    }

    template <typename EntityType, std::size_t PageSize>
    bool operator!=(const sparse_set<EntityType, PageSize>& lhs, const sparse_set<EntityType, PageSize>& rhs) noexcept
    {
        using tp = sparse_set<EntityType, PageSize>::type::type;

        if (lhs.size() == rhs.size())
        {
            for (size_t i = 0; i < lhs.size(); ++i)
            {
                const auto entity = entity_traits<tp>::from_type(static_cast<tp>(i));
                const auto left = lhs.contains(entity);
                const auto right = rhs.contains(entity);

                if (left != right)
                {
                    return true;
                }
            }
            return false;
        }
        return true;
    }

    template<typename EntityType, std::size_t PageSize>
    inline bool sparse_set<EntityType, PageSize>::empty() const noexcept
    {
        return size() == 0;
    }

    template<typename EntityType, std::size_t PageSize>
    inline std::size_t sparse_set<EntityType, PageSize>::size() const noexcept
    {
        return _packed.size();
    }

    template<typename EntityType, std::size_t PageSize>
    inline std::size_t sparse_set<EntityType, PageSize>::capacity() const noexcept
    {
        return _packed.capacity();
    }

    template<typename EntityType, std::size_t PageSize>
    inline bool sparse_set<EntityType, PageSize>::contains(const type& tp) const noexcept
    {
        const auto page = _page(tp);
        const auto offset = _offset(tp);

        if (page < _sparse.size())
        {
            const auto trampoline = _sparse[page][offset];
            return trampoline.identifier < _packed.size() && _packed[trampoline.identifier] == tp;
        }
        return false;
    }

    template<typename EntityType, std::size_t PageSize>
    inline void sparse_set<EntityType, PageSize>::shrink_to_fit()
    {
        if (empty())
        {
            _sparse.clear();
        }
    }

    template<typename EntityType, std::size_t PageSize>
    inline void sparse_set<EntityType, PageSize>::clear()
    {
        _sparse.clear();
        _packed.clear();
    }

    template<typename EntityType, std::size_t PageSize>
    inline void sparse_set<EntityType, PageSize>::insert(const type& tp)
    {
        const auto sparsePage = _page(tp);
        const auto sparseOffset = _offset(tp);

        for (auto pg = _sparse.size(); pg <= sparsePage; ++pg)
        {
            auto page = std::make_unique<EntityType[]>(PageSize);
            for (std::size_t i = 0; i < PageSize; ++i)
            {
                page[i] = _tombstone;
            }
            _sparse.push_back(std::move(page));
        }

        auto& page = _sparse[sparsePage];
        if (page[sparseOffset] == _tombstone)
        {
            // Value is not in the set
            page[sparseOffset] = entity_traits<EntityType::type>::from_type(static_cast<EntityType::type>(_packed.size()));
            _packed.push_back(tp);
        }
    }

    template<typename EntityType, std::size_t PageSize>
    inline void sparse_set<EntityType, PageSize>::remove(const type& tp)
    {
        const auto sparsePage = _page(tp);
        const auto sparseOffset = _offset(tp);

        if (sparsePage < _sparse.size())
        {
            const auto trampoline = _sparse[sparsePage][sparseOffset];
            if (trampoline == tp)
            {
                const auto packedIndex = trampoline.identifier;
                _sparse[sparsePage][sparseOffset] = _tombstone;
                
                const auto back = _packed.back();
                _packed[packedIndex] = back;
                _packed.pop_back();

                const auto toMove = entity_traits<EntityType::type>::from_type(packedIndex);
                _sparse[_page(toMove)][_offset(toMove)] = toMove;
            }
        }
    }

    template<typename EntityType, std::size_t PageSize>
    inline void sparse_set<EntityType, PageSize>::reserve(const std::size_t count)
    {
        const auto pageCount = 1 + ((count - 1) / PageSize);
        _sparse.reserve(pageCount);
        _packed.reserve(count);

        for (auto i = _sparse.size(); i < pageCount; ++i)
        {
            _sparse[i] = std::make_unique<EntityType[]>(PageSize);
            for (auto j = 0; j < PageSize; ++j)
            {
                _sparse[i][j] = _tombstone;
            }
        }
    }

    template<typename EntityType, std::size_t PageSize>
    inline std::size_t sparse_set<EntityType, PageSize>::_page(const type& tp) const noexcept
    {
        const auto result = tp.identifier / PageSize;
        return result;
    }

    template<typename EntityType, std::size_t PageSize>
    inline std::size_t sparse_set<EntityType, PageSize>::_offset(const type& tp) const noexcept
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

} // namespace entities

#endif // sparse_set_hpp__