#ifndef type_traits_hpp__
#define type_traits_hpp__

#include <type_traits>

namespace ryujin
{
    template <typename ... Ts>
    struct all_nothrow_copy_constructible : std::bool_constant<(std::is_nothrow_copy_constructible_v<Ts> && ...)>
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
