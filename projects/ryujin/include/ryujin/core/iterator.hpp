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

    struct input_iterator_tag {};
    struct output_iterator_tag {};
    struct forward_iterator_tag {};
    struct bidirectional_iterator_tag : public forward_iterator_tag {};
    struct random_access_iterator_tag : public bidirectional_iterator_tag {};
    struct contiguous_iterator_tag : public random_access_iterator_tag {};

    template <typename T>
    struct iterator_traits
    {
        using difference_type = typename T::difference_type;
        using value_type = typename T::value_type;
        using pointer = typename T::pointer;
        using reference = typename T::reference;
    };

    template <typename T>
    struct iterator_traits<T*>
    {
        using difference_type = ptr_diff;
        using value_type = remove_cv_t<T>;
        using pointer = T*;
        using reference = T&;
        using iterator_category = random_access_iterator_tag;
        using iterator_concept = contiguous_iterator_tag;
    };
}

#endif // iterator_hpp__
