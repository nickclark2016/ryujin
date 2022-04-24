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

    template <bool B>
    using bool_constant = integral_constant<bool, B>;

    using true_type = bool_constant<true>;
    using false_type = bool_constant<false>;

    template <typename T>
    struct is_lvalue_reference : false_type {};

    template <typename T>
    struct is_lvalue_reference<T&> : true_type {};

    template <typename T>
    inline constexpr bool is_lvalue_reference_v = is_lvalue_reference<T>::value;

    template <typename T>
    struct remove_reference { using type = T; };

    template <typename T>
    struct remove_reference<T&> { using type = T; };

    template <typename T>
    struct remove_reference<T&&> { using type = T; };

    template <typename T>
    using remove_reference_t = typename remove_reference<T>::type;
}

#endif // type_traits_hpp__
