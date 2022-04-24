#ifndef iterator_hpp__
#define iterator_hpp__

#include "primitives.hpp"

namespace ryujin
{
    template <typename C>
    inline constexpr auto begin(C& c) -> decltype(c.begin())
    {
        return c.begin();
    }

    template <typename C>
    inline constexpr auto begin(const C& c) -> decltype(c.begin())
    {
        return c.begin();
    }

    template <typename T, sz N>
    inline constexpr T* begin(T(&arr)[N]) noexcept
    {
        return arr;
    }

    template <typename C>
    inline constexpr auto cbegin(const C& c) noexcept -> decltype(begin(c))
    {
        return begin(c);
    }

    template <typename C>
    inline constexpr auto end(C& c) -> decltype(c.end())
    {
        return c.end();
    }

    template <typename C>
    inline constexpr auto begin(const C& c) -> decltype(c.end())
    {
        return c.end();
    }

    template <typename T, sz N>
    inline constexpr T* end(T(&arr)[N]) noexcept
    {
        return arr + N;
    }

    template <typename C>
    inline constexpr auto cend(const C& c) noexcept -> decltype(end(c))
    {
        return end(c);
    }
}

#endif // iterator_hpp__
