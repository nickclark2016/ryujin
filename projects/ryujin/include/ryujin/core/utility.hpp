#ifndef utility_hpp__
#define utility_hpp__

#include "primitives.hpp"
#include "type_traits.hpp"

namespace ryujin
{
    template <typename T>
    constexpr T&& forward(remove_reference_t<T>& t) noexcept
    {
        return static_cast<T&&>(t);
    }

    template <typename T>
    inline constexpr T&& forward(remove_reference_t<T>&& t) noexcept
    {
        static_assert(!is_lvalue_reference_v<T>);
        return static_cast<T&&>(t);
    }

    template <typename T>
    inline constexpr remove_reference_t<T>&& move(T&& t) noexcept
    {
        return static_cast<remove_reference_t<T>&&>(t);
    }

    struct in_place_t
    {
        explicit in_place_t() = default;
    };

    inline constexpr in_place_t in_place{};

    template <typename T>
    struct in_place_type_t
    {
        explicit in_place_type_t() = default;
    };

    template <typename T>
    inline constexpr in_place_type_t<T> in_place_type{};

    template <sz I>
    struct in_place_index_t
    {
        explicit in_place_index_t() = default;
    };

    template <sz I>
    inline constexpr in_place_index_t in_place_index{};
}

#endif // utility_hpp__
