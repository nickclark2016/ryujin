#ifndef type_traits_hpp__
#define type_traits_hpp__

#include <type_traits>

namespace ryujin
{
    template <typename T>
    struct type_identity
    {
        using type = T;
    };

    template <typename T>
    using type_identity_t = typename type_identity<T>::type;

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

    namespace detail
    {
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
    add_rvalue_reference_t<T> declval() noexcept;

    template <typename...>
    using void_t = void;

    template <typename T, typename U>
    struct is_same : false_type
    {
    };

    template <typename T>
    struct is_same<T, T> : true_type
    {
    };

    template <typename T, typename U>
    inline constexpr bool is_same_v = is_same<T, U>::value;

    template <typename T>
    struct is_void : is_same<T, void>
    {
    };

    template <typename T>
    inline constexpr bool is_void_v = is_void<T>::value;

    template <typename T>
    struct remove_all_extents
    {
        using type = T;
    };

    template <typename T>
    struct remove_all_extents<T[]>
    {
        using type = typename remove_all_extents<T>::type;
    };

    template <typename T, sz N>
    struct remove_all_extents<T[N]>
    {
        using type = typename remove_all_extents<T>::type;
    };

    template <typename T>
    using remove_all_extents_t = typename remove_all_extents<T>::type;

    template <typename T>
    struct is_const : public false_type
    {
    };

    template <typename T>
    struct is_const<T const> : public true_type
    {
    };

    template <typename T>
    inline constexpr bool is_const_v = is_const<T>::value;

    template <typename T>
    struct is_function : public bool_constant<!(is_reference_v<T> || !is_const_v<const T>)>
    {
    };

    template <typename T>
    inline constexpr bool is_function_v = is_function<T>::value;

    namespace detail
    {
        template <typename T, typename ... Args>
        struct is_constructible_impl : false_type
        {
        };

        template <typename T, typename ... Args>
        struct is_constructible_impl<void_t<decltype(::new T(declval<Args>()...))>, T, Args...> : true_type
        {
        };

        // constructible, reference, type, arguments
        template <bool, bool, typename T, typename ... Args>
        struct is_nothrow_constructible_impl;
        
        template <typename T, typename ... Args>
        struct is_nothrow_constructible_impl<true, false, T, Args...> : public bool_constant<noexcept(T(declval<Args>()...))>
        {
        };

        template <typename T>
        void implicit_conversion_to(T) noexcept {};

        template <typename T, typename Arg>
        struct is_nothrow_constructible_impl<true, true, T, Arg> :  public bool_constant<noexcept(implicit_conversion_to<T>(declval<Arg>()))>
        {
        };

        template <typename T, bool Ref, typename ... Args>
        struct is_nothrow_constructible_impl<false, Ref, T, Args...> : public false_type
        {
        };

        template <typename, typename T>
        struct select_second
        {
            using type = T;
        };

        template <typename, typename T>
        using select_second_t = typename select_second<void, T>::type;

        template <typename T, typename Arg>
        select_second_t<decltype((declval<T>() = declval<Arg>())), true_type> is_assignable_test(int);

        template <typename, typename>
        false_type is_assignable_test(...);

        template <typename T, typename Arg, bool = is_void_v<T> || is_void_v<Arg>>
        struct is_assignable_impl : public decltype((is_assignable_test<T, Arg>(0)))
        {
        };

        template <typename T, typename Arg>
        struct is_assignable_impl<T, Arg, true> : public false_type
        {
        };

        template <bool, typename T, typename Arg>
        struct is_nothrow_assignable_impl;

        template <typename T, typename Arg>
        struct is_nothrow_assignable_impl<false, T, Arg> : public true_type
        {
        };

        template <typename T, typename Arg>
        struct is_nothrow_assignable_impl<true, T, Arg> : public bool_constant<noexcept(declval<T>() = declval<Arg>())>
        {
        };

        template <typename>
        struct is_destructible_apply
        {
            using type = int;
        };

        struct test_res
        {
            char x[2];
        };

        template <typename T>
        struct is_destructor_well_formed
        {
            template <typename U>
            static char test(typename is_destructible_apply<decltype(declval<U&>().~U())>::type);

            template <typename U>
            static test_res test(...);

            static constexpr bool value = sizeof(test<T>(12)) == sizeof(char);
        };

        template <typename T, bool>
        struct destructible_impl;

        template <typename T>
        struct destructible_impl<T, false> : public bool_constant<is_destructor_well_formed<remove_all_extents_t<T>>::value>
        {
        };

        template <typename T>
        struct destructible_impl<T, true> : true_type
        {
        };

        template <typename T, bool>
        struct cannot_be_destructed;

        template <typename T>
        struct cannot_be_destructed<T, false> : public destructible_impl<T, is_reference_v<T>>
        {
        };
        
        template <typename T>
        struct cannot_be_destructed<T, true> : public false_type
        {
        };

        template <bool Destructible, typename T>
        struct is_nothrow_destructible_impl;

        template <typename T>
        struct is_nothrow_destructible_impl<false, T> : public false_type
        {
        };

        template <typename T>
        struct is_nothrow_destructible_impl<true, T> : public bool_constant<noexcept(declval<T>().~T())>
        {
        };
    }

    template <typename T, typename ... Args>
    struct is_constructible : public detail::is_constructible_impl<void_t<>, T, Args...>
    {
    };

    template <typename T, typename ... Args>
    inline constexpr bool is_constructible_v = is_constructible<T, Args...>::value;

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
    struct is_move_constructible : public is_constructible<T, add_rvalue_reference_t<T>>
    {
    };

    template <typename T>
    inline constexpr bool is_move_constructible_v = is_move_constructible<T>::value;

    template <typename T, typename ... Args>
    struct is_nothrow_constructible : public detail::is_nothrow_constructible_impl<is_constructible<T, Args...>::value, is_reference<T>::value, T, Args...>
    {
    };

    template <typename T, typename ... Args>
    inline constexpr bool is_nothrow_constructible_v = is_nothrow_constructible<T, Args...>::value;

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

    template <typename T>
    struct is_nothrow_move_constructible : public is_nothrow_constructible<T, add_rvalue_reference_t<T>>
    {
    };

    template <typename T>
    inline constexpr bool is_nothrow_move_constructible_v = is_nothrow_copy_constructible<T>::value;

    template <typename T, typename Arg>
    struct is_assignable : public detail::is_assignable_impl<T, Arg>
    {
    };

    template <typename T, typename Arg>
    inline constexpr bool is_assignable_v = is_assignable<T, Arg>::value;

    template <typename T>
    struct is_copy_assignable : public is_assignable<add_lvalue_reference_t<T>, add_lvalue_reference_t<add_const_t<T>>>
    {
    };

    template <typename T>
    inline constexpr bool is_copy_assignable_v = is_copy_assignable<T>::value;

    template <typename T>
    struct is_move_assignable : public is_assignable<add_lvalue_reference_t<T>, add_rvalue_reference_t<T>>
    {
    };

    template <typename T>
    inline constexpr bool is_move_assignable_v = is_move_assignable<T>::value;

    template <typename T, typename Arg>
    struct is_nothrow_assignable : public detail::is_nothrow_assignable_impl<is_assignable_v<T, Arg>, T, Arg>
    {
    };

    template <typename T, typename Arg>
    inline constexpr bool is_nothrow_assignable_v = is_nothrow_assignable<T, Arg>::value;

    template <typename T>
    struct is_nothrow_copy_assignable : public is_nothrow_assignable<add_lvalue_reference_t<T>, add_lvalue_reference_t<add_const_t<T>>>
    {
    };

    template <typename T>
    inline constexpr bool is_nothrow_copy_assignable_v = is_nothrow_copy_assignable<T>::value;

    template <typename T>
    struct is_nothrow_move_assignable : public is_nothrow_assignable<add_lvalue_reference_t<T>, add_rvalue_reference_t<T>>
    {
    };

    template <typename T>
    inline constexpr bool is_nothrow_move_assignable_v = is_nothrow_move_assignable<T>::value;

    template <typename T>
    struct is_destructible : public detail::cannot_be_destructed<T, is_function_v<T>>
    {
    };

    template <typename T>
    struct is_destructible<T[]> : public false_type
    {
    };

    template <>
    struct is_destructible<void> : public false_type
    {
    };

    template <typename T>
    inline constexpr bool is_destructible_v = is_destructible<T>::value;

    template <typename T>
    struct is_nothrow_destructible : public detail::is_nothrow_destructible_impl<is_destructible_v<T>, T>
    {
    };

    template <typename T, sz N>
    struct is_nothrow_destructible<T[N]> : public detail::is_nothrow_destructible_impl<is_destructible_v<T>, T>
    {
    };

    template <typename T>
    struct is_nothrow_destructible<T&> : public true_type
    {
    };

    template <typename T>
    struct is_nothrow_destructible<T&&> : public true_type
    {
    };

    template <typename T>
    inline constexpr bool is_nothrow_destructible_v = is_nothrow_destructible<T>::value;

    template <typename ... Ts>
    struct all_nothrow_copy_constructible : bool_constant<(is_nothrow_copy_constructible_v<Ts> && ...)>
    {
    };

    template <typename ...Ts>
    inline constexpr bool all_nothrow_copy_constructible_v = all_nothrow_copy_constructible<Ts...>::value;

    template <typename ... Ts>
    struct all_nothrow_copy_assignable : bool_constant<(is_nothrow_copy_assignable_v<Ts> && ...)>
    {
    };

    template <typename ...Ts>
    inline constexpr bool all_nothrow_copy_assignable_v = all_nothrow_copy_assignable<Ts...>::value;

    template <typename ... Ts>
    struct all_nothrow_move_constructible : bool_constant<(is_nothrow_move_constructible_v<Ts> && ...)>
    {
    };

    template <typename ...Ts>
    inline constexpr bool all_nothrow_move_constructible_v = all_nothrow_move_constructible<Ts...>::value;

    template <typename ... Ts>
    struct all_nothrow_move_assignable : bool_constant<(is_nothrow_move_assignable_v<Ts> && ...)>
    {
    };

    template <typename ...Ts>
    inline constexpr bool all_nothrow_move_assignable_v = all_nothrow_move_assignable<Ts...>::value;

    template <typename ... Ts>
    struct all_nothrow_destructible : bool_constant<(is_nothrow_destructible_v<Ts> && ...)>
    {
    };

    template <typename ...Ts>
    inline constexpr bool all_nothrow_destructible_v = all_nothrow_destructible<Ts...>::value;
}

#endif // type_traits_hpp__
