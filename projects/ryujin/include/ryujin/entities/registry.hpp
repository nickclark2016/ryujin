#ifndef registry_hpp__
#define registry_hpp__

#include "entity.hpp"
#include "entity_relationship_component.hpp"
#include "events.hpp"
#include "function_table.hpp"
#include "sparse_set.hpp"
#include "sparse_map.hpp"
#include "transform_component.hpp"

#include "../core/vector.hpp"

#include <utility>

namespace ryujin
{
    template <typename Type>
    class base_registry;

    namespace detail
    {
        struct pool_function_table
        {
            function_table default_table;
            bool (*contains)(void*, void*); // pool, entity pointer
            void (*remove)(void*, void*); // pool, entity pointer
            void (*delete_map)(void*); // pool
        };

        struct pool
        {
            std::size_t identifier;
            void* sparse_map;
            pool_function_table fn;
        };

        struct component_identifier_utility
        {
            static std::size_t id;

            template <typename T>
            static std::size_t fetch_identifier()
            {
                static std::size_t typeId = id++;
                return typeId;
            }
        };

        template <typename EntityType, typename ComponentType, std::size_t PageSize>
        class component_view_iterable
        {
        public:
            auto begin() noexcept;
            auto begin() const noexcept;
            const auto cbegin() const noexcept;

            auto end() noexcept;
            auto end() const noexcept;
            const auto cend() const noexcept;
        private:
            component_view_iterable() = default;

            pool _pool;

            friend class base_registry<EntityType>;
        };

        template <typename EntityType, typename PoolValueType, std::size_t PageSize>
        constexpr pool_function_table construct_pool_fn_table()
        {
            pool_function_table table;
            table.default_table = construct_fn_table<pool>();
            table.contains = [](void* raw, void* e) {
                pool* p = reinterpret_cast<pool*>(raw);
                void* rawSparseMap = p->sparse_map;
                sparse_map<EntityType, PoolValueType, PageSize>* sparseMap = reinterpret_cast<sparse_map<EntityType, PoolValueType, PageSize>*>(rawSparseMap);
                return sparseMap->contains(*reinterpret_cast<EntityType*>(e));
            };
            table.remove = [](void* raw, void* e) {
                pool* p = reinterpret_cast<pool*>(raw);
                void* rawSparseMap = p->sparse_map;
                sparse_map<EntityType, PoolValueType, PageSize>* sparseMap = reinterpret_cast<sparse_map<EntityType, PoolValueType, PageSize>*>(rawSparseMap);
                sparseMap->remove(*reinterpret_cast<EntityType*>(e));
            };
            table.delete_map = [](void* raw) {
                pool* p = reinterpret_cast<pool*>(raw);
                void* rawSparseMap = p->sparse_map;
                sparse_map<EntityType, PoolValueType, PageSize>* sparseMap = reinterpret_cast<sparse_map<EntityType, PoolValueType, PageSize>*>(rawSparseMap);
                delete sparseMap;
            };
            return table;
        }

        template <typename EntityType, typename ValueType, std::size_t PageSize>
        pool allocate_pool(const std::size_t identifier)
        {
            pool p;
            p.identifier = identifier;
            p.fn = construct_pool_fn_table<EntityType, ValueType, PageSize>();
            p.sparse_map = new sparse_map<EntityType, ValueType, PageSize>();
            return p;
        }

        template <typename EntityType, typename ComponentType, std::size_t PageSize>
        inline auto ryujin::detail::component_view_iterable<EntityType, ComponentType, PageSize>::begin() noexcept
        {
            sparse_map<EntityType, ComponentType, PageSize>* map = reinterpret_cast<sparse_map<EntityType, ComponentType, PageSize>*>(_pool.sparse_map);
            return map->value_begin();
        }

        template <typename EntityType, typename ComponentType, std::size_t PageSize>
        inline auto ryujin::detail::component_view_iterable<EntityType, ComponentType, PageSize>::begin() const noexcept
        {
            const sparse_map<EntityType, ComponentType, PageSize>* map = reinterpret_cast<const sparse_map<EntityType, ComponentType, PageSize>*>(_pool.sparse_map);
            return map->value_begin();
        }

        template <typename EntityType, typename ComponentType, std::size_t PageSize>
        inline const auto ryujin::detail::component_view_iterable<EntityType, ComponentType, PageSize>::cbegin() const noexcept
        {
            const sparse_map<EntityType, ComponentType, PageSize>* map = reinterpret_cast<const sparse_map<EntityType, ComponentType, PageSize>*>(_pool.sparse_map);
            return map->value_cbegin();
        }

        template <typename EntityType, typename ComponentType, std::size_t PageSize>
        inline auto ryujin::detail::component_view_iterable<EntityType, ComponentType, PageSize>::end() noexcept
        {
            sparse_map<EntityType, ComponentType, PageSize>* map = reinterpret_cast<sparse_map<EntityType, ComponentType, PageSize>*>(_pool.sparse_map);
            return map->value_end();
        }

        template <typename EntityType, typename ComponentType, std::size_t PageSize>
        inline auto ryujin::detail::component_view_iterable<EntityType, ComponentType, PageSize>::end() const noexcept
        {
            const sparse_map<EntityType, ComponentType, PageSize>* map = reinterpret_cast<const sparse_map<EntityType, ComponentType, PageSize>*>(_pool.sparse_map);
            return map->value_end();
        }

        template <typename EntityType, typename ComponentType, std::size_t PageSize>
        inline const auto ryujin::detail::component_view_iterable<EntityType, ComponentType, PageSize>::cend() const noexcept
        {
            const sparse_map<EntityType, ComponentType, PageSize>* map = reinterpret_cast<const sparse_map<EntityType, ComponentType, PageSize>*>(_pool.sparse_map);
            return map->value_cend();
        }
    }

    template <typename Type>
    class entity_handle
    {
    public:
        entity_handle(const Type& handle, base_registry<Type>* reg);

        Type handle() const noexcept;

        template <typename T>
        entity_handle<Type>& assign(const T& t);

        template <typename T>
        entity_handle& assign_or_replace(const T& t);

        template <typename T>
        T& get() const noexcept;

        template <typename T>
        T* try_get() const noexcept;

        template <typename T>
        entity_handle& remove();

        template <typename T>
        entity_handle& replace(const T& t);

        template <typename T>
        bool contains() const;

    private:
        Type _handle;
        base_registry<Type>* _registry;
    };

    namespace detail
    {
        template <typename EntityType, typename ... Ts>
        class entity_view_iterator
        {
        public:
            entity_view_iterator(base_registry<EntityType>* base_registry, std::size_t index, std::size_t nextFreeIndex)
                : _registry(base_registry), _index(index), _nextFreeIndex(nextFreeIndex)
            {
            }

            using iterator_category = std::forward_iterator_tag;
            using value_type = entity_handle<EntityType>;
            using reference_type = entity_handle<EntityType>&;

            bool operator==(const entity_view_iterator& rhs) const noexcept;
            bool operator!=(const entity_view_iterator& rhs) const noexcept;

            entity_view_iterator& operator++();
            entity_view_iterator operator++(int);

            value_type operator*();
            value_type operator*() const;
        private:
            base_registry<EntityType>* _registry;
            std::size_t _index;
            std::size_t _nextFreeIndex;
        };

        template <typename EntityType, typename ... ComponentTypes>
        class entity_view_iterable
        {
        public:
            entity_view_iterable(base_registry<EntityType>* base_registry);

            auto begin() noexcept;
            auto begin() const noexcept;
            const auto cbegin() const noexcept;

            auto end() noexcept;
            auto end() const noexcept;
            const auto cend() const noexcept;
        private:
            base_registry<EntityType>* _registry;
        };
    }

    template <typename ComponentType, typename EntityType>
    struct component_add_event
    {
        component_add_event(const ComponentType& value, const entity_handle<EntityType> entity)
            : value(value), entity(entity)
        {}

        const ComponentType& value;
        const entity_handle<EntityType> entity;
    };

    template <typename ComponentType, typename EntityType>
    struct component_remove_event
    {
        component_remove_event(const entity_handle<EntityType> entity)
            : entity(entity)
        {}

        const entity_handle<EntityType> entity;
    };

    template <typename ComponentType, typename EntityType>
    struct component_replace_event
    {
        component_replace_event(const entity_handle<EntityType> entity)
            : entity(entity)
        {}

        const entity_handle<EntityType> entity;
    };

    template <typename Type>
    class base_registry
    {
    public:
        using entity_type = Type;
        using entity_type_traits = entity_traits<typename Type::type>;

        base_registry();
        base_registry(const base_registry&) = delete;
        base_registry(base_registry&&) noexcept = delete;
        ~base_registry();

        base_registry& operator=(const base_registry&) = delete;
        base_registry& operator=(base_registry&&) noexcept = delete;

        std::size_t active() const noexcept;
        std::size_t capacity() const noexcept;
        std::size_t allocated() const noexcept;

        entity_handle<Type> allocate();
        void deallocate(entity_handle<Type>& handle);

        template <typename T>
        void assign(entity_handle<Type>& handle, const T& value);

        template <typename T>
        void assign_or_replace(entity_handle<Type>& handle, const T& value);

        template <typename T, typename ... Ts>
        bool contains(const entity_handle<Type>& handle) const;

        template <typename T>
        T& get(const entity_handle<Type>& handle) const noexcept;

        template <typename T>
        T* try_get(const entity_handle<Type>& handle) const noexcept;

        template <typename T>
        void remove(entity_handle<Type>& handle);

        template <typename T>
        void replace(entity_handle<Type>& handle, const T& value);

        entity_handle<Type> at(const std::size_t idx) noexcept;

        template <typename T>
        auto component_view() noexcept;

        template <typename ... Ts>
        auto entity_view() noexcept;

        auto& events() noexcept;
        auto& events() const noexcept;

    private:
        static constexpr std::size_t _poolSize = 1024;
        static constexpr entity_type _tombstone = entity_traits<typename entity_type::type>::from_type(~(typename entity_type::type)(0));

        vector<detail::pool> _pools;
        vector<entity_type> _entities;
        entity_type _freeListHead = _tombstone;

        entity_type _allocateNewIdentifier();
        entity_type _recycleExistingIdentifier();

        std::size_t _active;

        event_manager _events;

        template <typename EntityType, typename ... ComponentTypes>
        friend class detail::entity_view_iterable;

        friend class detail::entity_view_iterator<Type>;
    };

    template <typename Type>
    inline entity_handle<Type>::entity_handle(const Type& handle, base_registry<Type>* reg)
        : _handle(handle), _registry(reg)
    {
    }

    template <typename Type>
    inline Type entity_handle<Type>::handle() const noexcept
    {
        return _handle;
    }

    template <typename Type>
    template <typename T>
    inline entity_handle<Type>& entity_handle<Type>::assign(const T& t)
    {
        _registry->assign(*this, t);
        return *this;
    }

    template <typename Type>
    template <typename T>
    inline entity_handle<Type>& entity_handle<Type>::assign_or_replace(const T& t)
    {
        _registry->template assign_or_replace(*this, t);
        return *this;
    }

    template <typename Type>
    template <typename T>
    inline entity_handle<Type>& entity_handle<Type>::remove()
    {
        _registry->template remove<T>(*this);
        return *this;
    }

    template <typename Type>
    template <typename T>
    inline entity_handle<Type>& entity_handle<Type>::replace(const T& t)
    {
        _registry->template replace(*this, t);
        return *this;
    }

    template <typename Type>
    template <typename T>
    inline bool entity_handle<Type>::contains() const
    {
        return _registry->template contains<T>(*this);
    }

    template <typename Type>
    template <typename T>
    inline T& entity_handle<Type>::get() const noexcept
    {
        return _registry->template get<T>(*this);
    }

    template <typename Type>
    template <typename T>
    inline T* entity_handle<Type>::try_get() const noexcept
    {
        return _registry->template try_get<T>(*this);
    }


    template <typename Type>
    inline base_registry<Type>::base_registry()
        : _active(0)
    {
        using T = entity_relationship_component<Type>;

        const auto typeId = detail::component_identifier_utility::fetch_identifier<T>();
        auto pool = detail::allocate_pool<entity_type, T, _poolSize>(typeId);
        _pools.push_back(pool);
    }

    template <typename Type>
    inline base_registry<Type>::~base_registry()
    {
        for (auto pool : _pools)
        {
            pool.fn.delete_map(&pool);
            pool.fn.default_table.dtor(&pool);
        }
    }

    template <typename Type>
    inline std::size_t ryujin::base_registry<Type>::active() const noexcept
    {
        return _active;
    }

    template <typename Type>
    inline std::size_t ryujin::base_registry<Type>::capacity() const noexcept
    {
        return _entities.capacity();
    }

    template<typename Type>
    inline std::size_t base_registry<Type>::allocated() const noexcept
    {
        return _entities.size();
    }

    template <typename Type>
    inline entity_handle<Type> base_registry<Type>::allocate()
    {
        entity_type entity = _freeListHead == _tombstone ? _allocateNewIdentifier() : _recycleExistingIdentifier();
        ++_active;
        auto handle = entity_handle(entity, this);
        assign<transform_component>(handle, {});
        return handle;
    }
    
    template <typename Type>
    inline void base_registry<Type>::deallocate(entity_handle<Type>& handle)
    {
        auto entity = handle.handle();
    
        // release value from pools
        for (auto& pool : _pools)
        {
            pool.fn.remove(&pool, &entity);
        }

        auto identifier = entity.identifier;
        auto version = entity.version + 1;

        --_active;

        // add it to the free list in id ascending order
        if (identifier < _freeListHead.identifier)
        {
            _entities[identifier] = entity_type{ _freeListHead.identifier, static_cast<entity_type_traits::version_type>(version) };
            _freeListHead = entity_type{ identifier, 0 };
            return;
        }

        for (auto current = _freeListHead.identifier; current != _tombstone.identifier; current = _entities[current].identifier)
        {
            auto entity = _entities[current];
            if (identifier < entity.identifier)
            {
                _entities[identifier] = entity_type{ entity.identifier, static_cast<entity_type_traits::version_type>(version) };
                _entities[current] = entity_type{ identifier, _entities[current].version };
                return;
            }
        }
    }

    template<typename Type>
    inline entity_handle<Type> ryujin::base_registry<Type>::at(const std::size_t idx) noexcept
    {
        if (idx < _entities.size())
        {
            entity_handle<Type> res(_entities[idx], this);
            return res;
        }
        entity_handle<Type> res(_tombstone, this);
        return res;
    }

    template <typename Type>
    template <typename T>
    inline void base_registry<Type>::assign(entity_handle<Type>& handle, const T& value)
    {
        static const auto typeId = detail::component_identifier_utility::fetch_identifier<T>();
        while (typeId >= _pools.size())
        {
            detail::pool p;
            p.sparse_map = nullptr;
            p.identifier = ~std::size_t(0);

            _pools.push_back(p);
        }

        if (_pools[typeId].sparse_map == nullptr)
        {
            _pools[typeId] = detail::allocate_pool<entity_type, T, _poolSize>(typeId);
        }

        auto entity = handle.handle();
        detail::pool& pool = _pools[typeId];
        sparse_map<entity_type, T, _poolSize>* sparseMap = reinterpret_cast<sparse_map<entity_type, T, _poolSize>*>(pool.sparse_map);
        const auto inserted = sparseMap->insert(entity, value);
        if (inserted)
        {
            _events.emit<component_add_event<T, Type>>(value, handle);
        }
    }

    template <typename Type>
    template <typename T>
    inline void base_registry<Type>::assign_or_replace(entity_handle<Type>& handle, const T& value)
    {
        static const auto typeId = detail::component_identifier_utility::fetch_identifier<T>();
        while (typeId >= _pools.size())
        {
            detail::pool p;
            p.sparse_map = nullptr;
            p.identifier = ~std::size_t(0);

            _pools.push_back(p);
        }

        if (_pools[typeId].sparse_map == nullptr)
        {
            _pools[typeId] = detail::allocate_pool<entity_type, T, _poolSize>(typeId);
        }

        auto entity = handle.handle();
        detail::pool& pool = _pools[typeId];
        sparse_map<entity_type, T, _poolSize>* sparseMap = reinterpret_cast<sparse_map<entity_type, T, _poolSize>*>(pool.sparse_map);
        const auto replaced = sparseMap->insert_or_replace(entity, value);
        if (replaced)
        {
            _events.emit<component_replace_event<T, Type>>(handle);
        }
        else
        {
            _events.emit<component_add_event<T, Type>>(value, handle);
        }
    }

    template <typename Type>
    template <typename T, typename ... Ts>
    inline bool base_registry<Type>::contains(const entity_handle<Type>& handle) const
    {
        static const auto typeId = detail::component_identifier_utility::fetch_identifier<T>();
        if (typeId < _pools.size())
        {
            const detail::pool& pool = _pools[typeId];
            sparse_map<entity_type, T, _poolSize>* sparseMap = reinterpret_cast<sparse_map<entity_type, T, _poolSize>*>(pool.sparse_map);
            const bool res = sparseMap->contains(handle.handle());
            if constexpr (sizeof...(Ts) > 0)
            {
                return res & contains<Ts...>(handle);
            }
            else
            {
                return res;
            }
        }
        return false;
    }

    template <typename Type>
    template <typename T>
    T& base_registry<Type>::get(const entity_handle<Type>& handle) const noexcept
    {
        static const auto typeId = detail::component_identifier_utility::fetch_identifier<T>();
        T* result = nullptr;
        if (typeId < _pools.size())
        {
            const detail::pool& pool = _pools[typeId];
            sparse_map<entity_type, T, _poolSize>* sparseMap = reinterpret_cast<sparse_map<entity_type, T, _poolSize>*>(pool.sparse_map);
            result = &sparseMap->get(handle.handle());
        }
        return *result;
    }

    template <typename Type>
    template <typename T>
    T* base_registry<Type>::try_get(const entity_handle<Type>& handle) const noexcept
    {
        static const auto typeId = detail::component_identifier_utility::fetch_identifier<T>();
        T* result = nullptr;
        if (typeId < _pools.size())
        {
            const detail::pool& pool = _pools[typeId];
            sparse_map<entity_type, T, _poolSize>* sparseMap = reinterpret_cast<sparse_map<entity_type, T, _poolSize>*>(pool.sparse_map);
            if (sparseMap->contains(handle.handle()))
            {
                result = &sparseMap->get(handle.handle());
            }
        }
        return result;
    }

    template <typename Type>
    template <typename T>
    inline void ryujin::base_registry<Type>::remove(entity_handle<Type>& handle)
    {
        static const auto typeId = detail::component_identifier_utility::fetch_identifier<T>();
        if (typeId < _pools.size())
        {
            detail::pool& pool = _pools[typeId];
            sparse_map<entity_type, T, _poolSize>* sparseMap = reinterpret_cast<sparse_map<entity_type, T, _poolSize>*>(pool.sparse_map);
            const auto removed = sparseMap->remove(handle.handle());
            if (removed)
            {
                _events.emit<component_remove_event<T, Type>>(handle);
            }
        }
    }

    template <typename Type>
    template <typename T>
    inline void ryujin::base_registry<Type>::replace(entity_handle<Type>& handle, const T& value)
    {
        static const auto typeId = detail::component_identifier_utility::fetch_identifier<T>();
        if (typeId < _pools.size())
        {
            detail::pool& pool = _pools[typeId];
            sparse_map<entity_type, T, _poolSize>* sparseMap = reinterpret_cast<sparse_map<entity_type, T, _poolSize>*>(pool.sparse_map);
            const auto replaced = sparseMap->replace(handle.handle(), value);
            if (replaced)
            {
                _events.emit<component_replace_event<T, Type>>(handle);
            }
        }
    }

    template <typename Type>
    template <typename T>
    inline auto base_registry<Type>::component_view() noexcept
    {
        static const auto typeId = detail::component_identifier_utility::fetch_identifier<T>();
        while (typeId >= _pools.size())
        {
            detail::pool p;
            p.sparse_map = nullptr;
            p.identifier = ~std::size_t(0);

            _pools.push_back(p);
        }

        if (_pools[typeId].sparse_map == nullptr)
        {
            _pools[typeId] = detail::allocate_pool<entity_type, T, _poolSize>(typeId);
        }

        detail::pool& pool = _pools[typeId];
        detail::component_view_iterable<Type, T, _poolSize> it;
        it._pool = pool;
        return it;
    }

    template<typename Type>
    template<typename ...Ts>
    inline auto base_registry<Type>::entity_view() noexcept
    {
        detail::entity_view_iterable<Type, Ts...> iterable(this);
        return iterable;
    }

    template<typename Type>
    inline auto& base_registry<Type>::events() noexcept
    {
        return _events;
    }

    template<typename Type>
    inline auto& base_registry<Type>::events() const noexcept
    {
        return _events;
    }
    
    template <typename Type>
    inline typename base_registry<Type>::entity_type base_registry<Type>::_allocateNewIdentifier()
    {
        const auto identifier = static_cast<typename entity_type_traits::identifier_type>(_entities.size());
        const auto generation = 0;
        entity_type e{ identifier, generation };
        _entities.push_back(e);
        return e;
    }
    
    template <typename Type>
    inline typename base_registry<Type>::entity_type base_registry<Type>::_recycleExistingIdentifier()
    {
        const auto identifier = _freeListHead.identifier;
        const auto version = _entities[identifier].version;
        _freeListHead = _entities[identifier];
        return _entities[identifier] = entity_type{ identifier, version };
    }

    namespace detail
    {
        template<typename EntityType, typename ...Ts>
        inline bool entity_view_iterator<EntityType, Ts...>::operator==(const entity_view_iterator& rhs) const noexcept
        {
            return _index == rhs._index && _registry == rhs._registry;
        }
        
        template<typename EntityType, typename ...Ts>
        inline bool entity_view_iterator<EntityType, Ts...>::operator!=(const entity_view_iterator& rhs) const noexcept
        {
            return _index != rhs._index || _registry != rhs._registry;
        }
        
        template<typename EntityType, typename ...Ts>
        inline entity_view_iterator<EntityType, Ts...>& entity_view_iterator<EntityType, Ts...>::operator++()
        {
            ++_index;

            if constexpr (sizeof...(Ts) == 0)
            {
                while (_index >= _nextFreeIndex)
                {
                    if (_index == _nextFreeIndex)
                    {
                        _nextFreeIndex = _registry->_entities[_nextFreeIndex].identifier;
                    }
                    ++_index;
                }
            }
            else
            {
                while (!_registry->template contains<Ts...>(_registry->at(_index)) || _index >= _nextFreeIndex)
                {
                    if (_index == _nextFreeIndex)
                    {
                        _nextFreeIndex = _registry->at(_nextFreeIndex).handle().identifier;
                    }
                    ++_index;

                    if (_index > _registry->allocated())
                    {
                        _index = _registry->allocated();
                        break;
                    }
                }
            }

            return *this;
        }
        
        template<typename EntityType, typename ...Ts>
        inline entity_view_iterator<EntityType, Ts...> entity_view_iterator<EntityType, Ts...>::operator++(int)
        {
            auto copy = *this;
            this->operator++();
            return copy;
        }
        
        template<typename EntityType, typename ...Ts>
        inline typename entity_view_iterator<EntityType, Ts...>::value_type entity_view_iterator<EntityType, Ts...>::operator*()
        {
            return _registry->at(_index);
        }
        
        template<typename EntityType, typename ...Ts>
        inline typename entity_view_iterator<EntityType, Ts...>::value_type entity_view_iterator<EntityType, Ts...>::operator*() const
        {
            return _registry->at(_index);
        }

        template<typename EntityType, typename ...ComponentTypes>
        inline entity_view_iterable<EntityType, ComponentTypes...>::entity_view_iterable(base_registry<EntityType>* base_registry)
            : _registry(base_registry)
        {
        }
        
        template<typename EntityType, typename ...ComponentTypes>
        inline auto entity_view_iterable<EntityType, ComponentTypes...>::begin() noexcept
        {
            std::size_t index = 0;
            std::size_t nextFree = _registry->_freeListHead.identifier;

            if constexpr (sizeof...(ComponentTypes) == 0)
            {
                while (index >= nextFree)
                {
                    if (index == nextFree)
                    {
                        nextFree = _registry->_entities[nextFree].identifier;
                    }
                    ++index;
                }
            }
            else
            {
                if (index == _registry->_entities.size())
                {
                    return entity_view_iterator<EntityType, ComponentTypes...>(_registry, index, nextFree);
                }

                while ((index != _registry->_entities.size() && index >= nextFree) || !_registry->template contains<ComponentTypes...>(_registry->at(index)))
                {
                    if (index == nextFree)
                    {
                        nextFree = _registry->_entities[nextFree].identifier;
                    }
                    ++index;

                    if (index > _registry->allocated())
                    {
                        index = _registry->allocated();
                        break;
                    }
                }
            }

            return entity_view_iterator<EntityType, ComponentTypes...>(_registry, index, nextFree);
        }
        
        template<typename EntityType, typename ...ComponentTypes>
        inline auto entity_view_iterable<EntityType, ComponentTypes...>::begin() const noexcept
        {
            std::size_t index = 0;
            std::size_t nextFree = _registry->_freeListHead.identifier;

            if constexpr (sizeof...(ComponentTypes) == 0)
            {
                while (index >= nextFree)
                {
                    if (index == nextFree)
                    {
                        nextFree = _registry->_entities[nextFree].identifier;
                    }
                    ++index;
                }
            }
            else
            {
                while ((index != _registry->_entities.size() && index >= nextFree) || !_registry->template contains<ComponentTypes...>(_registry->at(index)))
                {
                    if (index == nextFree)
                    {
                        nextFree = _registry->_entities[nextFree].identifier;
                    }
                    ++index;

                    if (index > _registry->allocated())
                    {
                        index = _registry->allocated();
                        break;
                    }
                }
            }

            return entity_view_iterator<EntityType, ComponentTypes...>(_registry, index, nextFree);
        }
        
        template<typename EntityType, typename ...ComponentTypes>
        inline const auto entity_view_iterable<EntityType, ComponentTypes...>::cbegin() const noexcept
        {
            std::size_t index = 0;
            std::size_t nextFree = _registry->_freeListHead.identifier;

            if constexpr (sizeof...(ComponentTypes) == 0)
            {
                while (index >= nextFree)
                {
                    if (index == nextFree)
                    {
                        nextFree = _registry->_entities[nextFree].identifier;
                    }
                    ++index;
                }
            }
            else
            {
                while ((index != _registry->_entities.size() && index >= nextFree) || !_registry->template contains<ComponentTypes...>(_registry->at(index)))
                {
                    if (index == nextFree)
                    {
                        nextFree = _registry->_entities[nextFree].identifier;
                    }
                    ++index;

                    if (index > _registry->allocated())
                    {
                        index = _registry->allocated();
                        break;
                    }
                }
            }

            return entity_view_iterator<EntityType, ComponentTypes...>(_registry, index, nextFree);
        }
        
        template<typename EntityType, typename ...ComponentTypes>
        inline auto entity_view_iterable<EntityType, ComponentTypes...>::end() noexcept
        {
            return entity_view_iterator<EntityType, ComponentTypes...>(_registry, _registry->_entities.size(), 0);
        }
        
        template<typename EntityType, typename ...ComponentTypes>
        inline auto entity_view_iterable<EntityType, ComponentTypes...>::end() const noexcept
        {
            return entity_view_iterator<EntityType, ComponentTypes...>(_registry, _registry->_entities.size(), 0);
        }
        
        template<typename EntityType, typename ...ComponentTypes>
        inline const auto entity_view_iterable<EntityType, ComponentTypes...>::cend() const noexcept
        {
            return entity_view_iterator<EntityType, ComponentTypes...>(_registry, _registry->_entities.size(), 0);
        }
    }

    using registry = base_registry<entity<std::conditional_t<sizeof(std::size_t) == 8, std::uint64_t, std::uint32_t>>>;
}

#endif // registry_hpp__
