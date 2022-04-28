#ifndef variant_hpp__
#define variant_hpp__

#include "primitives.hpp"
#include "utility.hpp"

#include <cassert>
#include <cstring>
#include <new>
#include <type_traits>

namespace ryujin
{
    namespace detail
    {
        template <typename T, typename ...Ts>
        struct variant_index_helper;

        template <typename T, typename ... Ts>
        struct variant_index_helper<T, T, Ts...> : public ryujin::integral_constant<sz, 0> {};

        template <typename T, typename U, typename... Ts>
        struct variant_index_helper<T, U, Ts...> : public ryujin::integral_constant<sz, 1 + variant_index_helper<T, Ts...>::value> {};

        template <typename ... Ts>
        inline constexpr sz variant_index_helper_v = variant_index_helper<Ts...>::value;

        template <sz arg1, sz... others>
        struct max_value;

        template <sz arg>
        struct max_value<arg>
        {
            static constexpr sz value = arg;
        };

        template <sz arg1, sz arg2, sz... others>
        struct max_value<arg1, arg2, others...>
        {
            static constexpr sz value = arg1 >= arg2 ? max_value<arg1, others...>::value : max_value<arg2, others...>::value;
        };

        template <sz ... Vs>
        inline constexpr sz max_value_v = max_value<Vs...>::value;

        template <typename... Types>
        struct variant_assignment_helper;

        template <typename T, typename ... Ts>
        struct variant_assignment_helper<T, Ts...>
        {
            inline static void destroy(const sz index, void* addr)
            {
                if (index == 0)
                {
                    reinterpret_cast<T*>(addr)->~T();
                }
                else
                {
                    variant_assignment_helper<Ts...>::destroy(index - 1, addr);
                }
            }

            inline static void copy(const sz index, const void* oldAddr, void* newAddr)
            {
                if (index == 0)
                {
                    ::new(newAddr) T(*reinterpret_cast<const T*>(oldAddr));
                }
                else
                {
                    variant_assignment_helper<Ts...>::copy(index - 1, oldAddr, newAddr);
                }
            }

            inline static void copy_assign(const sz index, const void* oldAddr, void* newAddr) noexcept
            {
                if (index == 0)
                {
                    const T& oldRef = *reinterpret_cast<const T*>(oldAddr);
                    T& newRef = *reinterpret_cast<T*>(newAddr);
                    newRef = oldRef;
                }
                else
                {
                    variant_assignment_helper<Ts...>::copy_assign(index - 1, oldAddr, newAddr);
                }
            }

            inline static void move(const sz index, void* oldAddr, void* newAddr)
            {
                if (index == 0)
                {
                    ::new(newAddr) T(ryujin::move(*reinterpret_cast<T*>(oldAddr)));
                }
                else
                {
                    variant_assignment_helper<Ts...>::move(index - 1, oldAddr, newAddr);
                }
            }

            inline static void move_assign(const sz index, void* oldAddr, void* newAddr) noexcept
            {
                if (index == 0)
                {
                    T& newRef = *reinterpret_cast<T*>(newAddr);
                    newRef = ryujin::move(*reinterpret_cast<T*>(oldAddr));
                }
                else
                {
                    variant_assignment_helper<Ts...>::move_assign(index - 1, oldAddr, newAddr);
                }
            }
        };

        template <>
        struct variant_assignment_helper<>
        {
            inline static void destroy(const sz, void*) { assert(false && "Invalid index"); }
            inline static void copy(const sz, const void*, void*) { assert(false && "Invalid index");  }
            inline static void copy_assign(const sz index, const void* oldAddr, void* newAddr) { assert(false && "Invalid index");  }
            inline static void move(const sz, void*, void*) noexcept { assert(false && "Invalid index"); }
            inline static void move_assign(const sz index, void* oldAddr, void* newAddr) noexcept { assert(false && "Invalid index"); }
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
    class alignas(detail::max_value_v<8, alignof(Ts)...>) variant
    {
    public:
        constexpr variant() noexcept(ryujin::is_nothrow_default_constructible_v<variant_alternative_t<0, variant<Ts...>>>);
        constexpr variant(const variant& other) noexcept(all_nothrow_copy_constructible_v<Ts...>);
        constexpr variant(variant&& other) noexcept(all_nothrow_move_constructible_v<Ts...>);
        
        template <typename T>
        constexpr variant(const T& t) noexcept(ryujin::is_nothrow_copy_constructible_v<T>);

        template <typename T>
        constexpr variant(T&& t) noexcept(ryujin::is_nothrow_move_constructible_v<T>);

        template <typename T, typename ... Args>
        constexpr explicit variant(ryujin::in_place_type_t<T>, Args&& ... args) noexcept(ryujin::is_nothrow_constructible_v<T, Args...>);

        template <sz I, typename ... Args>
        constexpr explicit variant(ryujin::in_place_index_t<I>, Args&& ... args) noexcept(ryujin::is_nothrow_constructible_v<variant_alternative_t<I, variant<Ts...>>, Args...>);

        constexpr ~variant() noexcept(all_nothrow_destructible_v<Ts...>);

        constexpr variant& operator=(const variant& rhs) noexcept(all_nothrow_copy_assignable_v<Ts...> && all_nothrow_destructible_v<Ts...>);
        constexpr variant& operator=(variant&& rhs) noexcept(all_nothrow_move_assignable_v<Ts...> && all_nothrow_destructible_v<Ts...>);

        template <typename T>
        constexpr variant& operator=(const T& rhs) noexcept(ryujin::is_nothrow_copy_assignable_v<T> && all_nothrow_destructible_v<Ts...>);

        template <typename T>
        constexpr variant& operator=(T&& rhs) noexcept(ryujin::is_nothrow_move_assignable_v<T> && all_nothrow_destructible_v<Ts...>);

        constexpr sz index() const noexcept;

        template <typename T, typename ... Args>
        constexpr T& emplace(Args&& ... args) noexcept(ryujin::is_nothrow_constructible_v<T, Args...> && all_nothrow_destructible_v<Ts...>);

        template <sz I, typename ... Args>
        constexpr variant_alternative_t<I, variant>& emplace(Args&& ... args) noexcept(ryujin::is_nothrow_constructible_v<variant_alternative_t<I, variant<Ts...>>, Args...> && all_nothrow_destructible_v<Ts...>);

        constexpr void swap(variant& rhs) noexcept(all_nothrow_move_constructible_v<Ts...>);

    private:
        static constexpr sz data_size = detail::max_value_v<sizeof(Ts)...>;
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
    inline constexpr variant<Ts...>::variant() noexcept(ryujin::is_nothrow_default_constructible_v<variant_alternative_t<0, variant<Ts...>>>)
    {
        using first_type = variant_alternative_t<0, variant<Ts...>>;
        ::new(_data) first_type();
        _heldIndex = 0;
    }
    
    template <typename ... Ts>
    inline constexpr variant<Ts...>::variant(const variant& v) noexcept(all_nothrow_copy_constructible_v<Ts...>)
    {
        detail::variant_assignment_helper<Ts...>::copy(v._heldIndex, v._data, _data);
        _heldIndex = v._heldIndex;
    }

    template <typename ... Ts>
    inline constexpr variant<Ts...>::variant(variant&& v) noexcept(all_nothrow_move_constructible_v<Ts...>)
    {
        detail::variant_assignment_helper<Ts...>::move(v._heldIndex, v._data, _data);
        _heldIndex = v._heldIndex;
    }

    template <typename ... Ts>
    template <typename T>
    inline constexpr variant<Ts...>::variant(const T& t) noexcept(ryujin::is_nothrow_copy_constructible_v<T>)
    {
        using type = remove_cvref_t<T>;
        constexpr sz idx = detail::variant_index_helper_v<T, Ts...>;
        static_assert(idx < sizeof...(Ts), "Illegal type specified in variant construction.");
        _heldIndex = idx;
        ::new(_data) type(t);
    }

    template <typename ... Ts>
    template <typename T>
    inline constexpr variant<Ts...>::variant(T&& t) noexcept(ryujin::is_nothrow_move_constructible_v<T>)
    {
        using type = remove_cvref_t<T>;
        constexpr sz idx = detail::variant_index_helper_v<type, Ts...>;
        static_assert(idx < sizeof...(Ts), "Illegal type specified in variant construction.");
        _heldIndex = idx;
        ::new(_data) type(ryujin::forward<T>(t));
    }

    template <typename ... Ts>
    template <typename T, typename ... Args>
    inline constexpr variant<Ts...>::variant(ryujin::in_place_type_t<T>, Args&& ... args) noexcept(ryujin::is_nothrow_constructible_v<T, Args...>)
    {
        using type = remove_cvref_t<T>;
        constexpr sz idx = detail::variant_index_helper_v<type, Ts...>;
        static_assert(idx < sizeof...(Ts), "Illegal index specified in variant construction.");
        _heldIndex = idx;
        ::new(_data) type(ryujin::forward<Args>(args)...);
    }

    template <typename ... Ts>
    template <sz I, typename ... Args>
    inline constexpr variant<Ts...>::variant(ryujin::in_place_index_t<I>, Args&& ... args) noexcept(ryujin::is_nothrow_constructible_v<variant_alternative_t<I, variant<Ts...>>, Args...>)
    {
        static_assert(I < sizeof...(Ts), "Illegal index specified in variant construction.");
        using T = variant_alternative_t<I, variant<Ts...>>;
        _heldIndex = I;
        ::new(_data) T(ryujin::forward<Args>(args)...);
    }

    template <typename ... Ts>
    inline constexpr variant<Ts...>::~variant() noexcept(all_nothrow_destructible_v<Ts...>)
    {
        detail::variant_assignment_helper<Ts...>::destroy(_heldIndex, _data);
        _heldIndex = variant_npos;
    }

    template <typename ... Ts>
    inline constexpr variant<Ts...>& variant<Ts...>::operator=(const variant& rhs) noexcept(all_nothrow_copy_assignable_v<Ts...> && all_nothrow_destructible_v<Ts...>)
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
    constexpr variant<Ts...>& variant<Ts...>::operator=(variant&& rhs)  noexcept(all_nothrow_move_assignable_v<Ts...> && all_nothrow_destructible_v<Ts...>)
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
    inline constexpr variant<Ts...>& variant<Ts...>::operator=(const T& rhs) noexcept(ryujin::is_nothrow_copy_assignable_v<T> && all_nothrow_destructible_v<Ts...>)
    {
        using type = remove_cvref_t<T>;
        constexpr sz idx = detail::variant_index_helper_v<type, Ts...>;
        static_assert(idx < sizeof...(Ts), "Illegal type specified in variant assignment.");
        if (idx == index())
        {
            *reinterpret_cast<type*>(_data) = rhs;
        }
        else
        {
            detail::variant_assignment_helper<Ts...>::destroy(_heldIndex, _data);
            _heldIndex = idx;
            ::new (_data) type(rhs);
        }

        return *this;
    }

    template <typename ... Ts>
    template <typename T>
    inline constexpr variant<Ts...>& variant<Ts...>::operator=(T&& rhs) noexcept(ryujin::is_nothrow_move_assignable_v<T> && all_nothrow_destructible_v<Ts...>)
    {
        using type = remove_cvref_t<T>;
        constexpr sz idx = detail::variant_index_helper_v<type, Ts...>;
        static_assert(idx < sizeof...(Ts), "Illegal type specified in variant assignment.");
        if (idx == index())
        {
            *reinterpret_cast<type*>(_data) = ryujin::move(rhs);
        }
        else
        {
            detail::variant_assignment_helper<Ts...>::destroy(_heldIndex, _data);
            _heldIndex = idx;
            ::new (_data) type(ryujin::forward<T>(rhs));
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
    constexpr T& variant<Ts...>::emplace(Args&& ... args) noexcept(ryujin::is_nothrow_constructible_v<T, Args...>&& all_nothrow_destructible_v<Ts...>)
    {
        constexpr sz idx = detail::variant_index_helper_v<T, Ts...>;
        static_assert(idx < sizeof...(Ts), "Illegal type specified in variant emplace.");
        detail::variant_assignment_helper<Ts...>::destroy(_heldIndex, _data);
        T* ptr = ::new(_data) T(ryujin::forward<Args>(args)...);
        _heldIndex = idx;
        return *ptr;
    }

    template <typename ... Ts>
    template <sz I, typename ... Args>
    constexpr variant_alternative_t<I, variant<Ts...>>& variant<Ts...>::emplace(Args&& ... args) noexcept(ryujin::is_nothrow_constructible_v<variant_alternative_t<I, variant<Ts...>>, Args...> && all_nothrow_destructible_v<Ts...>)
    {
        static_assert(I < sizeof...(Ts), "Illegal type specified in variant emplace.");
        detail::variant_assignment_helper<Ts...>::destroy(_heldIndex, _data);
        variant_alternative_t<I, variant<Ts...>>* ptr = ::new(_data) variant_alternative_t<I, variant<Ts...>>(ryujin::forward<Args>(args)...);
        _heldIndex = I;
        return *ptr;
    }

    template <typename ... Ts>
    constexpr void variant<Ts...>::swap(variant& rhs) noexcept(all_nothrow_move_constructible_v<Ts...>)
    {
        auto tmp = ryujin::move(rhs);
        rhs = ryujin::move(*this);
        *this = ryujin::move(tmp);
    }

    template <typename T, typename ... Ts>
    inline constexpr T& get(ryujin::variant<Ts...>& v)
    {
        constexpr sz idx = detail::variant_index_helper_v<T, Ts...>;
        assert(idx == v.index());
        return *reinterpret_cast<T*>(v._data);
    }

    template <typename T, typename ... Ts>
    inline constexpr T&& get(ryujin::variant<Ts...>&& v)
    {
        constexpr sz idx = detail::variant_index_helper_v<T, Ts...>;
        assert(idx == v.index());
        return ryujin::move(*reinterpret_cast<T*>(v._data));
    }

    template <typename T, typename ... Ts>
    inline constexpr const T& get(const ryujin::variant<Ts...>& v)
    {
        constexpr sz idx = detail::variant_index_helper_v<T, Ts...>;
        assert(idx == v.index());
        return *reinterpret_cast<const T*>(v._data);
    }

    template <typename T, typename ... Ts>
    constexpr const T&& get(const ryujin::variant<Ts...>&& v)
    {
        constexpr sz idx = detail::variant_index_helper_v<T, Ts...>;
        assert(idx == v.index());
        return ryujin::move(*reinterpret_cast<const T*>(v._data));
    }

    template <sz I, typename ... Ts>
    constexpr variant_alternative_t<I, variant<Ts...>>& get(ryujin::variant<Ts...>& v)
    {
        assert(I == v.index());
        return *reinterpret_cast<variant_alternative_t<I, variant<Ts...>>*>(v._data);
    }

    template <sz I, typename ... Ts>
    constexpr variant_alternative_t<I, variant<Ts...>>&& get(ryujin::variant<Ts...>&& v)
    {
        assert(I == v.index());
        return ryujin::move(*reinterpret_cast<variant_alternative_t<I, variant<Ts...>>*>(v._data));
    }

    template <sz I, typename ... Ts>
    constexpr const variant_alternative_t<I, variant<Ts...>>& get(const ryujin::variant<Ts...>& v)
    {
        assert(I == v.index());
        return *reinterpret_cast<const variant_alternative_t<I, variant<Ts...>>*>(v._data);
    }

    template <sz I, typename ... Ts>
    constexpr const variant_alternative_t<I, variant<Ts...>>&& get(const ryujin::variant<Ts...>&& v)
    {
        assert(I == v.index());
        return ryujin::move(*reinterpret_cast<const variant_alternative_t<I, variant<Ts...>>*>(v._data));
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
        return reinterpret_cast<variant_alternative_t<I, variant<Ts...>>*>(v._data);
    }

    template <sz I, typename ... Ts>
    constexpr const variant_alternative_t<I, variant<Ts...>>* get_if(const ryujin::variant<Ts...>* v)
    {
        return reinterpret_cast<const variant_alternative_t<I, variant<Ts...>>*>(v._data);
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
