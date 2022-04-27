#ifndef type_traits_hpp__
#define type_traits_hpp__

#include <type_traits>

namespace ryujin
{
    template <typename T, T v>
    struct integral_constant
    {
        static constexpr T value = v;
        using value_type = T;
        using type = integral_constant;
        inline constexpr operator value_type() const noexcept { return value; }
        inline constexpr value_type operator()() const noexcept { return value; }
    };

    template <bool B>
    using bool_constant = integral_constant<bool, B>;

    using true_type = bool_constant<true>;
    using false_type = bool_constant<false>;

    template <typename...>
    using void_t = void;

    template <typename T>
    struct type_identity
    {
        using type = T;
    };

    template <typename T>
    using type_identity_t = typename type_identity<T>::type;

    namespace detail
    {
        template <typename T, typename ... Args>
        struct is_constructible_impl : false_type
        {
        };

        template <typename T, typename ... Args>
        struct is_constructible_impl<void_t<decltype(::new T(std::declval<Args>()...))>, T, Args...> : true_type
        {
        };

        // constructible, reference, type, arguments
        template <bool, bool, typename T, typename ... Args> struct is_nothrow_constructible_impl;
        
        template <typename T, typename ... Args>
        struct is_nothrow_constructible_impl<true, false, T, Args...> : public bool_constant<noexcept(T(std::declval<Args>()...))>
        {
        };

        template <typename T>
        void implicit_conversion_to(T) noexcept {};

        template <typename T, typename Arg>
        struct is_nothrow_constructible_impl<true, true, T, Arg> :  public bool_constant<noexcept(implicit_conversion_to<T>(std::declval<Arg>()))>
        {
        };

        template <typename T, bool Ref, typename ... Args>
        struct is_nothrow_constructible_impl<false, Ref, T, Args...> : public false_type
        {
        };

        template <typename T>
        auto try_add_lvalue_reference(int)->type_identity<T&>;

        template <typename T>
        auto try_add_lvalue_reference(...)->type_identity<T>;

        template <typename T>
        auto try_add_rvalue_reference(int)->type_identity<T&&>;

        template <typename T>
        auto try_add_rvalue_reference(...)->type_identity<T>;
    }

    template <typename T>
    struct add_lvalue_reference : public decltype(detail::try_add_lvalue_reference<T>(0)) {};

    template <typename T>
    using add_lvalue_reference_t = typename add_lvalue_reference<T>::type;

    template <typename T>
    struct add_rvalue_reference : public decltype(detail::try_add_rvalue_reference<T>(0)) {};

    template <typename T>
    using add_rvalue_reference_t = typename add_rvalue_reference<T>::type;

    template <typename T>
    struct add_const
    {
        using type = const T;
    };

    template <typename T>
    using add_const_t = typename add_const<T>::type;

    template <typename T>
    struct is_reference : false_type {};

    template <typename T>
    struct is_reference<T&> : true_type {};

    template <typename T>
    struct is_reference<T&&> : true_type {};

    template <typename T>
    inline constexpr bool is_reference_v = is_reference<T>::value;

    template <typename T, typename ... Args>
    struct is_constructible : public detail::is_constructible_impl<void_t<>, T, Args...>
    {
    };

    template <typename T, typename ... Args>
    inline constexpr bool is_constructible_v = is_constructible<T, Args...>::value;

    template <typename T, typename ... Args>
    struct is_nothrow_constructible : public detail::is_nothrow_constructible_impl<is_constructible<T, Args...>::value, is_reference<T>::value, T, Args...>
    {
    };

    template <typename T, typename ... Args>
    inline constexpr bool is_nothrow_constructible_v = is_nothrow_constructible<T, Args...>::value;

    template <typename T>
    struct is_default_constructible : public is_constructible<T>
    {
    };

    template <typename T>
    inline constexpr bool is_default_constructible_v = is_default_constructible<T>::value;

    template <typename T>
    struct is_copy_constructible : public is_constructible<T, add_lvalue_reference_t<add_const_t<T>>>
    {
    };

    template <typename T>
    inline constexpr bool is_copy_constructible_v = is_copy_constructible<T>::value;

    template <typename T>
    struct is_nothrow_default_constructible : public is_nothrow_constructible<T>
    {
    };

    template <typename T>
    inline constexpr bool is_nothrow_default_constructible_v = is_nothrow_default_constructible<T>::value;

    template <typename T>
    struct is_nothrow_copy_constructible : public is_nothrow_constructible<T, add_lvalue_reference_t<add_const_t<T>>>
    {
    };

    template <typename T>
    inline constexpr bool is_nothrow_copy_constructible_v = is_nothrow_copy_constructible<T>::value;

    template <typename ... Ts>
    struct all_nothrow_copy_constructible : std::bool_constant<(is_nothrow_copy_constructible_v<Ts> && ...)>
    {
    };

    template <typename ...Ts>
    inline constexpr bool all_nothrow_copy_constructible_v = all_nothrow_copy_constructible<Ts...>::value;

    template <typename ... Ts>
    struct all_nothrow_copy_assignable : std::bool_constant<(std::is_nothrow_copy_assignable_v<Ts> && ...)>
    {
    };

    template <typename ...Ts>
    inline constexpr bool all_nothrow_copy_assignable_v = all_nothrow_copy_assignable<Ts...>::value;

    template <typename ... Ts>
    struct all_nothrow_move_constructible : std::bool_constant<(std::is_nothrow_move_constructible_v<Ts> && ...)>
    {
    };

    template <typename ...Ts>
    inline constexpr bool all_nothrow_move_constructible_v = all_nothrow_move_constructible<Ts...>::value;

    template <typename ... Ts>
    struct all_nothrow_move_assignable : std::bool_constant<(std::is_nothrow_move_assignable_v<Ts> && ...)>
    {
    };

    template <typename ...Ts>
    inline constexpr bool all_nothrow_move_assignable_v = all_nothrow_move_assignable<Ts...>::value;

    template <typename ... Ts>
    struct all_nothrow_destructible : std::bool_constant<(std::is_nothrow_destructible_v<Ts> && ...)>
    {
    };

    template <typename ...Ts>
    inline constexpr bool all_nothrow_destructible_v = all_nothrow_destructible<Ts...>::value;
}

#endif // type_traits_hpp__
