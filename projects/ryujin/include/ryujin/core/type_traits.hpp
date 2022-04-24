#ifndef type_traits_hpp__
#define type_traits_hpp__

namespace ryujin
{
    template <typename T, T v>
    struct integral_constant
    {
        using value_type = T;
        using type = integral_constant<T, v>;

        static constexpr T value = v;

        inline constexpr operator value_type() const noexcept
        {
            return value;
        }

        inline constexpr value_type operator()() const noexcept
        {
            return value;
        }
    };

    template <typename T>
    struct type_identity
    {
        using type = T;
    };

    template <typename T>
    using type_identity_t = typename type_identity<T>::type;

    template <bool B>
    using bool_constant = integral_constant<bool, B>;

    using true_type = bool_constant<true>;
    using false_type = bool_constant<false>;

    namespace detail::lvalue_reference
    {
        template <typename T>
        auto add_lvalue_reference_helper(int)->type_identity<T&>;

        template <typename T>
        auto add_lvalue_reference_helper(...)->type_identity<T>;
    }

    namespace detail::rvalue_reference
    {
        template <typename T>
        auto add_rvalue_reference_helper(int)->type_identity<T&&>;

        template <typename T>
        auto add_rvalue_reference_helper(...)->type_identity<T>;
    }

    template <typename T>
    struct add_lvalue_reference : decltype(detail::lvalue_reference::add_lvalue_reference_helper<T>(0)) {};

    template <typename T>
    using add_lvalue_reference_t = typename add_lvalue_reference<T>::type;

    template <typename T>
    struct add_rvalue_reference : decltype(detail::rvalue_reference::add_rvalue_reference_helper<T>(0)) {};

    template <typename T>
    using add_rvalue_reference_t = typename add_rvalue_reference<T>::type;

    template <typename T>
    struct is_lvalue_reference : false_type {};

    template <typename T>
    struct is_lvalue_reference<T&> : true_type {};

    template <typename T>
    inline constexpr bool is_lvalue_reference_v = is_lvalue_reference<T>::value;

    template <typename T>
    struct is_rvalue_reference : false_type {};

    template <typename T>
    struct is_rvalue_reference<T&&> : true_type {};

    template <typename T>
    inline constexpr bool is_rvalue_reference_v = is_rvalue_reference<T>::value;

    template <typename T>
    struct is_reference : false_type {};

    template <typename T>
    struct is_reference<T&> : true_type {};

    template <typename T>
    struct is_reference<T&&> : true_type {};

    template <typename T>
    inline constexpr bool is_reference_v = is_reference<T>::value;

    template <typename T>
    struct remove_reference { using type = T; };

    template <typename T>
    struct remove_reference<T&> { using type = T; };

    template <typename T>
    struct remove_reference<T&&> { using type = T; };

    template <typename T>
    using remove_reference_t = typename remove_reference<T>::type;

    template <bool B, typename T, typename F>
    struct conditional : type_identity<T> {};

    template <typename T, typename F>
    struct conditional<false, T, F> : type_identity<F> {};

    template <bool B, typename T, typename F>
    using conditional_t = typename conditional<B, T, F>::type;

    template <typename ...>
    struct conjunction : true_type {};

    template <typename T>
    struct conjunction<T> : T {};

    template <typename T, typename ... Ts>
    struct conjunction<T, Ts...> : conditional_t<bool(T::value), conjunction<Ts...>, T>
    {};

    template <typename ... Ts>
    inline constexpr bool conjunction_v = conjunction_v<Ts...>::value;

    namespace detail::declval
    {
        template <typename T, typename U = T&&>
        U declval_helper(int);

        template <typename T>
        T declval_helper(long);
    }

    template <typename T>
    auto declval() noexcept -> decltype(detail::declval::declval_helper<T>(0));

#ifdef _MSC_VER
    template <typename T, typename ... Args>
    struct is_constructible : bool_constant<__is_constructible(T, Args...)> {};

    template <typename To, typename From>
    struct is_assignable : bool_constant<__is_assignable(To, From)> {};

    template <typename T>
    struct is_destructible : bool_constant<__is_destructible(T)> {};

    template <typename T, typename ... Args>
    struct is_trivially_constructible : bool_constant<__is_trivially_constructible(T, Args...)> {};

    template <typename To, typename From>
    struct is_trivially_assignable : bool_constant<__is_trivially_assignable(To, From)> {};

    template <typename T, typename ... Args>
    struct is_nothrow_constructible : bool_constant<__is_nothrow_constructible(T, Args...)> {};

    template <typename To, typename From>
    struct is_nothrow_assignable : bool_constant<__is_nothrow_assignable(To, From)> {};

    template <typename T>
    struct is_nothrow_destructible : bool_constant<__is_nothrow_destructible(T)> {};
#else
#error Type traits not fully implemented for your compiler
#endif

    template <typename T>
    struct is_default_constructible : is_constructible<T> {};

    template <typename T>
    struct is_copy_constructible :is_constructible<T, add_lvalue_reference_t<const T>> {};

    template <typename T>
    struct is_move_constructible : is_constructible<T, T> {};

    template <typename T>
    struct is_copy_assignable : is_assignable<add_lvalue_reference_t<T>, add_lvalue_reference_t<const T>> {};

    template <typename T>
    struct is_move_assignable : is_assignable<add_lvalue_reference_t<T>, T> {};

    template <typename T>
    struct is_trivially_default_constructible : is_trivially_constructible<T> {};

    template <typename T>
    struct is_trivially_copy_constructible : is_trivially_constructible<T, add_lvalue_reference_t<const T>> {};

    template <typename T>
    struct is_trivially_move_constructible : is_trivially_constructible<T, T> {};

    template <typename T>
    struct is_trivially_copy_assignable : is_trivially_assignable<add_lvalue_reference_t<T>, add_lvalue_reference_t<const T>> {};

    template <typename T>
    struct is_trivially_move_assignable : is_trivially_assignable<add_lvalue_reference_t<T>, T> {};

    template <typename T>
    struct is_nothrow_default_constructible : is_nothrow_constructible<T> {};

    template <typename T>
    struct is_nothrow_copy_constructible : is_nothrow_constructible<T, add_lvalue_reference_t<const T>> {};

    template <typename T>
    struct is_nothrow_move_constructible : is_nothrow_constructible<T, T> {};

    template <typename T>
    struct is_nothrow_copy_assignable : is_nothrow_assignable<add_lvalue_reference_t<T>, add_lvalue_reference_t<const T>> {};

    template <typename T>
    struct is_nothrow_move_assignable : is_nothrow_assignable<add_lvalue_reference_t<T>, T> {};

    template <typename T, typename ... Args>
    inline constexpr bool is_constructible_v = is_constructible<T, Args...>::value;

    template <typename T>
    inline constexpr bool is_default_constructible_v = is_default_constructible<T>::value;

    template <typename T>
    inline constexpr bool is_copy_constructible_v = is_copy_constructible<T>::value;

    template <typename T>
    inline constexpr bool is_move_constructible_v = is_move_constructible<T>::value;

    template <typename To, typename From>
    inline constexpr bool is_assignable_v = is_assignable<To, From>::value;

    template <typename T>
    inline constexpr bool is_copy_assignable_v = is_copy_assignable<T>::value;

    template <typename T>
    inline constexpr bool is_move_assignable_v = is_move_assignable<T>::value;

    template <typename T>
    inline constexpr bool is_destructible_v = is_destructible<T>::value;

    template <typename T, typename ... Args>
    inline constexpr bool is_trivially_constructible_v = is_trivially_constructible<T, Args...>::value;

    template <typename T>
    inline constexpr bool is_trivially_default_constructible_v = is_trivially_default_constructible<T>::value;

    template <typename T>
    inline constexpr bool is_trivially_copy_constructible_v = is_trivially_copy_constructible<T>::value;

    template <typename T>
    inline constexpr bool is_trivially_move_constructible_v = is_trivially_move_constructible<T>::value;

    template <typename To, typename From>
    inline constexpr bool is_trivially_assignable_v = is_trivially_assignable<To, From>::value;

    template <typename T>
    inline constexpr bool is_trivially_copy_assignable_v = is_trivially_copy_assignable<T>::value;

    template <typename T>
    inline constexpr bool is_trivially_move_assignable_v = is_trivially_move_assignable<T>::value;

    template <typename T, typename ... Args>
    inline constexpr bool is_nothrow_constructible_v = is_nothrow_constructible<T, Args...>::value;

    template <typename T>
    inline constexpr bool is_nothrow_default_constructible_v = is_nothrow_default_constructible<T>::value;

    template <typename T>
    inline constexpr bool is_nothrow_copy_constructible_v = is_nothrow_copy_constructible<T>::value;

    template <typename T>
    inline constexpr bool is_nothrow_move_constructible_v = is_nothrow_move_constructible<T>::value;

    template <typename To, typename From>
    inline constexpr bool is_nothrow_assignable_v = is_nothrow_assignable<To, From>::value;

    template <typename T>
    inline constexpr bool is_nothrow_copy_assignable_v = is_nothrow_copy_assignable<T>::value;

    template <typename T>
    inline constexpr bool is_nothrow_move_assignable_v = is_nothrow_move_assignable<T>::value;

    template <typename T>
    inline constexpr bool is_nothrow_destructible_v = is_nothrow_destructible<T>::value;

    template <typename ... Ts>
    struct all_nothrow_copy_constructible : bool_constant<(is_nothrow_copy_constructible<Ts>{} && ...)>
    {
    };

    template <typename ...Ts>
    inline constexpr bool all_nothrow_copy_constructible_v = all_nothrow_copy_constructible<Ts...>::value;

    template <typename ... Ts>
    struct all_nothrow_copy_assignable : bool_constant < (is_nothrow_copy_assignable<Ts>{} && ...) >
    {
    };

    template <typename ...Ts>
    inline constexpr bool all_nothrow_copy_assignable_v = all_nothrow_copy_assignable<Ts...>::value;

    template <typename ... Ts>
    struct all_nothrow_move_constructible : bool_constant<(is_nothrow_move_constructible<Ts>{} && ...)>
    {
    };

    template <typename ...Ts>
    inline constexpr bool all_nothrow_move_constructible_v = all_nothrow_move_constructible<Ts...>::value;

    template <typename ... Ts>
    struct all_nothrow_move_assignable : bool_constant < (is_nothrow_move_assignable<Ts>{} && ...) >
    {
    };

    template <typename ...Ts>
    inline constexpr bool all_nothrow_move_assignable_v = all_nothrow_move_assignable<Ts...>::value;

    template <typename ... Ts>
    struct all_nothrow_destructible : bool_constant<(is_nothrow_destructible<Ts>{} && ...)>
    {
    };

    template <typename ...Ts>
    inline constexpr bool all_nothrow_destructible_v = all_nothrow_destructible<Ts...>::value;
}

#endif // type_traits_hpp__
