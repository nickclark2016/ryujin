#ifndef variant_hpp__
#define variant_hpp__

#include "primitives.hpp"
#include "type_traits.hpp"
#include "utility.hpp"

#include <cassert>
#include <new>

namespace ryujin
{
    namespace detail
    {
        template <typename T, typename ...Ts>
        struct variant_index_helper
        {
        };

        template <typename T, typename ... Ts>
        struct variant_index_helper<T, T, Ts...> : public ryujin::integral_constant<sz, 0> {};

        template <typename T, typename U, typename... Ts>
        struct variant_index_helper<T, U, Ts...> : public ryujin::integral_constant<sz, 1 + variant_index_helper<T, Ts...>::value> {};

        template <sz I, typename ... Ts>
        struct variant_element_helper;

        template <typename T, typename... Ts>
        struct variant_element_helper<0, T, Ts...>
        {
            using type = T;
        };

        template <sz I, typename T, typename ... Ts>
        struct variant_element_helper<I, T, Ts...> : variant_element_helper<I - 1, Ts...>
        {
        };

        template <sz arg1, sz... others>
        struct max_value;

        template <sz arg>
        struct max_value<arg>
        {
            static const sz value = arg;
        };

        template <sz arg1, sz arg2, sz... others>
        struct max_value<arg1, arg2, others...>
        {
            static const sz value = arg1 >= arg2 ? max_value<arg1, others...>::value : max_value<arg2, others...>::value;
        };

        template <typename... Types>
        struct variant_assignment_helper;

        template <typename T, typename ... Ts>
        struct variant_assignment_helper<T, Ts...>
        {
            inline static void destroy(const sz index, void* addr)
            {
                if (index == sizeof...(Ts))
                {
                    reinterpret_cast<T*>(addr)->~T();
                }
                else
                {
                    variant_assignment_helper<Ts...>::destroy(index, addr);
                }
            }

            inline static void copy(const sz index, const void* oldAddr, void* newAddr)
            {
                if (index == sizeof...(Ts))
                {
                    ::new(newAddr) T(*reinterpret_cast<const T*>(oldAddr));
                }
                else
                {
                    variant_assignment_helper<Ts...>::copy(index, oldAddr, newAddr);
                }
            }

            inline static void copy_assign(const sz index, const void* oldAddr, void* newAddr)
            {
                if (index == sizeof...(Ts))
                {
                    const T& oldRef = *reinterpret_cast<const T*>(oldAddr);
                    T& newRef = *reinterpret_cast<T*>(newAddr);
                    newRef = oldRef;
                }
                else
                {
                    variant_assignment_helper<Ts...>::copy_assign(index, oldAddr, newAddr);
                }
            }

            inline static void move(const sz index, void* oldAddr, void* newAddr)
            {
                if (index == sizeof...(Ts))
                {
                    ::new(newAddr) T(ryujin::move(*reinterpret_cast<T*>(oldAddr)));
                }
                else
                {
                    variant_assignment_helper<Ts...>::move(index, oldAddr, newAddr);
                }
            }

            inline static void move_assign(const sz index, void* oldAddr, void* newAddr)
            {
                if (index == sizeof...(Ts))
                {
                    T& newRef = *reinterpret_cast<T*>(newAddr);
                    newRef = ryujin::move(*reinterpret_cast<T*>(oldAddr));
                }
                else
                {
                    variant_assignment_helper<Ts...>::move_assign(index, oldAddr, newAddr);
                }
            }
        };


        template <>
        struct variant_assignment_helper<>
        {
            inline static void destroy(const sz, void*) {}
            inline static void copy(const sz, const void*, void*) {}
            inline static void copy_assign(const sz index, const void* oldAddr, void* newAddr) {}
            inline static void move(const sz, void*, void*) {}
            inline static void move_assign(const sz index, void* oldAddr, void* newAddr) {}
        };
    }

    inline constexpr sz variant_npos = static_cast<sz>(-1);

    template <typename ... Ts>
    class variant;

    template <sz I, typename T>
    struct variant_alternative;

    template <sz I, typename T, typename ... Ts>
    struct variant_alternative<I, variant<T, Ts...>> : variant_alternative<I - 1, variant<Ts...>> 
    {
        static_assert(sizeof...(Ts) > I - 1, "variant_alternative index out of range");
    };

    template <typename T, typename ... Ts>
    struct variant_alternative<0, variant<T, Ts...>>
    {
        using type = T;
    };

    template <sz I, typename T>
    using variant_alternative_t = typename variant_alternative<I, T>::type;

    template <typename ... Ts>
    class alignas(detail::max_value<8, alignof(Ts)...>::value) variant
    {
    public:
        constexpr variant();
        constexpr variant(const variant& other);
        constexpr variant(variant&& other) noexcept;
        
        template <typename T>
        constexpr variant(const T& t) noexcept;

        template <typename T>
        constexpr variant(T&& t) noexcept;

        template <typename T, typename ... Args>
        constexpr explicit variant(ryujin::in_place_type_t<T>, Args&& ... args);

        template <sz I, typename ... Args>
        constexpr explicit variant(ryujin::in_place_index_t<I>, Args&& ... args);

        constexpr ~variant();

        constexpr variant& operator=(const variant& rhs);
        constexpr variant& operator=(variant&& rhs) noexcept;

        template <typename T>
        constexpr variant& operator=(T&& rhs) noexcept;

        constexpr sz index() const noexcept;

        template <typename T, typename ... Args>
        constexpr T& emplace(Args&& ... args);

        template <sz I, typename ... Args>
        constexpr variant_alternative_t<I, variant>& emplace(Args&& ... args);

        constexpr void swap(variant& rhs);

    private:
        static constexpr sz data_size = detail::max_value<sizeof(Ts)...>::value;
        u8 _data[data_size];

        sz _heldIndex = variant_npos;

        template <typename T, typename ... Types>
        friend constexpr T& get(ryujin::variant<Types...>& v);

        template <typename T, typename ... Types>
        friend constexpr T&& get(ryujin::variant<Types...>&& v);

        template <typename T, typename ... Types>
        friend constexpr const T& get(const ryujin::variant<Types...>& v);

        template <typename T, typename ... Types>
        friend constexpr const T&& get(const ryujin::variant<Types...>&& v);

        template <sz I, typename ... Types>
        friend constexpr variant_alternative_t<I, variant<Types...>>& get(ryujin::variant<Types...>& v);

        template <sz I, typename ... Types>
        friend constexpr variant_alternative_t<I, variant<Types...>>&& get(ryujin::variant<Types...>&& v);

        template <sz I, typename ... Types>
        friend constexpr const variant_alternative_t<I, variant<Types...>>& get(const ryujin::variant<Types...>& v);

        template <sz I, typename ... Types>
        friend constexpr const variant_alternative_t<I, variant<Types...>>&& get(const ryujin::variant<Types...>&& v);

        template <typename T, typename ... Types>
        friend constexpr T* get_if(ryujin::variant<Types...>* v);

        template <typename T, typename ... Types>
        friend constexpr const T* get_if(const ryujin::variant<Types...>* v);

        template <sz I, typename ... Types>
        friend constexpr variant_alternative_t<I, variant<Types...>>* get_if(ryujin::variant<Types...>* v);

        template <sz I, typename ... Types>
        friend constexpr const variant_alternative_t<I, variant<Types...>>* get_if(const ryujin::variant<Types...>* v);
    };

    template <typename ... Ts>
    inline constexpr variant<Ts...>::variant()
    {
        using first_type = variant_alternative_t<0, variant<Ts>>;
        ::new(_data) first_type();
        _heldIndex = 0;
    }
    
    template <typename ... Ts>
    inline constexpr variant<Ts...>::variant(const variant& v)
    {
        detail::variant_assignment_helper<Ts...>::copy(v._heldIndex, v._data, _data);
        _heldIndex = v._heldIndex;
    }

    template <typename ... Ts>
    inline constexpr variant<Ts...>::variant(variant&& v) noexcept
    {
        detail::variant_assignment_helper<Ts...>::move(v._heldIndex, v._data, _data);
        _heldIndex = v._heldIndex;
    }

    template <typename ... Ts>
    template <typename T>
    inline constexpr variant<Ts...>::variant(const T& t) noexcept
    {
        constexpr sz idx = detail::variant_index_helper<T, Ts...>::value;
        static_assert(idx < sizeof...(Ts), "Illegal type specified in variant construction.");
        _heldIndex = idx;
        ::new(_data) T(t);
    }

    template <typename ... Ts>
    template <typename T>
    inline constexpr variant<Ts...>::variant(T&& t) noexcept
    {
        constexpr sz idx = detail::variant_index_helper<T, Ts...>::value;
        static_assert(idx < sizeof...(Ts), "Illegal type specified in variant construction.");
        _heldIndex = idx;
        ::new(_data) T(ryujin::forward<T>(t));
    }

    template <typename ... Ts>
    template <typename T, typename ... Args>
    inline constexpr variant<Ts...>::variant(ryujin::in_place_type_t<T>, Args&& ... args)
    {
        constexpr sz idx = detail::variant_index_helper<T, Ts...>::value;
        static_assert(idx < sizeof...(Ts), "Illegal index specified in variant construction.");
        _heldIndex = idx;
        ::new(_data) T(ryujin::forward<Args>(args)...);
    }

    template <typename ... Ts>
    template <sz I, typename ... Args>
    inline constexpr variant<Ts...>::variant(ryujin::in_place_index_t<I>, Args&& ... args)
    {
        static_assert(I < sizeof...(Ts), "Illegal index specified in variant construction.");
        using T = variant_alternative_t<I, Ts...>;
        _heldIndex = I;
        ::new(_data) T(ryujin::forward<Args>(args)...);
    }

    template <typename ... Ts>
    inline constexpr variant<Ts...>::~variant()
    {
        detail::variant_assignment_helper<Ts...>::destroy(_heldIndex, _data);
        _heldIndex = variant_npos;
    }

    template <typename ... Ts>
    inline constexpr variant<Ts...>& variant<Ts...>::operator=(const variant& rhs)
    {
        if (&rhs == this)
        {
            return *this;
        }

        if (rhs.index() == index())
        {
            detail::variant_assignment_helper<Ts...>::copy_assign(_heldIndex, rhs._data, _data);
        }
        else
        {
            detail::variant_assignment_helper<Ts...>::destroy(_heldIndex, _data);
            detail::variant_assignment_helper<Ts...>::copy(rhs._heldIndex, rhs._data, _data);
            _heldIndex = rhs.index();
        }

        return *this;
    }

    template <typename ... Ts>
    constexpr variant<Ts...>& variant<Ts...>::operator=(variant&& rhs) noexcept
    {
        if (&rhs == this)
        {
            return *this;
        }

        if (rhs.index() == index())
        {
            detail::variant_assignment_helper<Ts...>::move_assign(_heldIndex, rhs._data, _data);
        }
        else
        {
            detail::variant_assignment_helper<Ts...>::destroy(_heldIndex, _data);
            detail::variant_assignment_helper<Ts...>::move(rhs._heldIndex, rhs._data, _data);
            _heldIndex = rhs.index();
            rhs._heldIndex = variant_npos;
        }

        return *this;
    }

    template <typename ... Ts>
    template <typename T>
    inline constexpr variant<Ts...>& variant<Ts...>::operator=(T&& rhs) noexcept
    {
        constexpr sz idx = detail::variant_index_helper<T, Ts...>::value;
        static_assert(idx < sizeof...(Ts), "Illegal type specified in variant assignment.");
        if (idx == index())
        {
            *reinterpret_cast<T*>(_data) = ryujin::move(rhs);
        }
        else
        {
            detail::variant_assignment_helper<Ts...>::destroy(_heldIndex, _data);
            _heldIndex = idx;
            detail::variant_assignment_helper<Ts...>::move(_heldIndex, rhs._data, _data);
        }

        return *this;
    }

    template <typename ... Ts>
    inline constexpr sz variant<Ts...>::index() const noexcept
    {
        return _heldIndex;
    }

    template <typename ... Ts>
    template <typename T, typename ... Args>
    constexpr T& variant<Ts...>::emplace(Args&& ... args)
    {
        constexpr sz idx = detail::variant_index_helper<T, Ts...>::value;
        static_assert(idx < sizeof...(Ts), "Illegal type specified in variant emplace.");
        detail::variant_assignment_helper<Ts...>::destroy(_heldIndex, _data);
        T* ptr = ::new(_data) T(ryujin::forward<Args>(args)...);
        _heldIndex = idx;
        return *ptr;
    }

    template <typename ... Ts>
    template <sz I, typename ... Args>
    constexpr variant_alternative_t<I, variant<Ts...>>& variant<Ts...>::emplace(Args&& ... args)
    {
        static_assert(I < sizeof...(Ts), "Illegal type specified in variant emplace.");
        detail::variant_assignment_helper<Ts...>::destroy(_heldIndex, _data);
        variant_alternative_t<I, variant<Ts...>>* ptr = ::new(_data) variant_alternative_t<I, variant<Ts...>>(ryujin::forward<Args>(args)...);
        _heldIndex = I;
        return *ptr;
    }

    template <typename ... Ts>
    constexpr void variant<Ts...>::swap(variant& rhs)
    {
        auto tmp = *this;
        *this = rhs;
        rhs = tmp;
    }

    template <typename T, typename ... Ts>
    inline constexpr T& get(ryujin::variant<Ts...>& v)
    {
        constexpr sz idx = detail::variant_index_helper<T, Ts...>::value;
        assert(idx == v.index());
        return *reinterpret_cast<T*>(v._data);
    }

    template <typename T, typename ... Ts>
    inline constexpr T&& get(ryujin::variant<Ts...>&& v)
    {
        constexpr sz idx = detail::variant_index_helper<T, Ts...>::value;
        assert(idx == v.index());
        return ryujin::move(*reinterpret_cast<T*>(v._data));
    }

    template <typename T, typename ... Ts>
    inline constexpr const T& get(const ryujin::variant<Ts...>& v)
    {
        constexpr sz idx = detail::variant_index_helper<T, Ts...>::value;
        assert(idx == v.index());
        return *reinterpret_cast<const T*>(v._data);
    }

    template <typename T, typename ... Ts>
    constexpr const T&& get(const ryujin::variant<Ts...>&& v)
    {
        constexpr sz idx = detail::variant_index_helper<T, Ts...>::value;
        assert(idx == v.index());
        return ryujin::move(*reinterpret_cast<const T*>(v._data));
    }

    template <sz I, typename ... Ts>
    constexpr variant_alternative_t<I, variant<Ts...>>& get(ryujin::variant<Ts...>& v)
    {
        assert(I == v.index());
        return *reinterpret_cast<variant_alternative_t<I, Ts>*>(v._data);
    }

    template <sz I, typename ... Ts>
    constexpr variant_alternative_t<I, variant<Ts...>>&& get(ryujin::variant<Ts...>&& v)
    {
        assert(I == v.index());
        return ryujin::move(*reinterpret_cast<variant_alternative_t<I, Ts>*>(v._data));
    }

    template <sz I, typename ... Ts>
    constexpr const variant_alternative_t<I, variant<Ts...>>& get(const ryujin::variant<Ts...>& v)
    {
        assert(I == v.index());
        return *reinterpret_cast<const variant_alternative_t<I, Ts>*>(v._data);
    }

    template <sz I, typename ... Ts>
    constexpr const variant_alternative_t<I, variant<Ts...>>&& get(const ryujin::variant<Ts...>&& v)
    {
        assert(I == v.index());
        return ryujin::move(*reinterpret_cast<const variant_alternative_t<I, Ts>*>(v._data));
    }

    template <typename T, typename ... Ts>
    constexpr T* get_if(ryujin::variant<Ts...>* v)
    {
        constexpr sz idx = detail::variant_index_helper<T, Ts...>::value;
        if (idx != v->index()) return nullptr;
        return reinterpret_cast<T*>(v->_data);
    }

    template <typename T, typename ... Ts>
    constexpr const T* get_if(const ryujin::variant<Ts...>* v)
    {
        constexpr sz idx = detail::variant_index_helper<T, Ts...>::value;
        if (idx != v->index()) return nullptr;
        return reinterpret_cast<const T*>(v->_data);
    }

    template <sz I, typename ... Ts>
    constexpr variant_alternative_t<I, variant<Ts...>>* get_if(ryujin::variant<Ts...>* v)
    {
        return reinterpret_cast<variant_alternative_t<I, Ts>*>(v._data);
    }

    template <sz I, typename ... Ts>
    constexpr const variant_alternative_t<I, variant<Ts...>>* get_if(const ryujin::variant<Ts...>* v)
    {
        return reinterpret_cast<const variant_alternative_t<I, Ts>*>(v._data);
    }

    template <typename T, typename ... Ts>
    constexpr bool holds_alternative(const ryujin::variant<Ts...>& v) noexcept
    {
        return v.index() == detail::variant_index_helper<T, Ts...>::value;
    }

    template <typename ... Ts>
    void swap(variant<Ts...>& a, variant<Ts...>& b)
    {
        a.swap(b);
    }
}

#endif // variant_hpp__