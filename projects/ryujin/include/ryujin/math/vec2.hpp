#ifndef vec2_hpp__
#define vec2_hpp__

#include "../core/concepts.hpp"
#include "../core/primitives.hpp"

#include <cstddef>
#include <type_traits>

namespace ryujin
{
    template <numeric T>
    struct alignas(sizeof(T) * 2) vec2
    {
        union
        {
            T data[2];
            struct
            {
                T r;
                T g;
            };
            struct
            {
                T x;
                T y;
            };
            struct
            {
                T u;
                T v;
            };
        };

        constexpr vec2();
        constexpr vec2(const T scalar);
        constexpr vec2(const T x, const T y);

        constexpr T& operator[](const sz index) noexcept;
        constexpr const T& operator[](const sz index) const noexcept;

        vec2& operator+=(const vec2& rhs) noexcept;
        vec2& operator-=(const vec2& rhs) noexcept;
        vec2& operator*=(const vec2& rhs) noexcept;
        vec2& operator/=(const vec2& rhs) noexcept;
    };

    template <numeric T>
    vec2(const T) -> vec2<T>;

    template <numeric T>
    vec2(const T, const T, const T, const T) -> vec2<T>;

    template <numeric T>
    vec2(const vec2<T>&) -> vec2<T>;

    template <numeric T>
    vec2(vec2<T>&&) -> vec2<T>;

    // Implementation
    
    template <numeric T>
    inline constexpr vec2<T>::vec2()
        : vec2(T())
    {
    }

    template <numeric T>
    inline constexpr vec2<T>::vec2(const T scalar)
        : vec2(scalar, scalar)
    {
    }

    template <numeric T>
    inline constexpr vec2<T>::vec2(const T x, const T y)
        : x(x), y(y)
    {
    }
    
    template <numeric T>
    inline constexpr T& vec2<T>::operator[](const sz index) noexcept
    {
        return data[index];
    }
    
    template <numeric T>
    inline constexpr const T& vec2<T>::operator[](const sz index) const noexcept
    {
        return data[index];
    }

    template <numeric T>
    inline vec2<T>& vec2<T>::operator+=(const vec2& rhs) noexcept
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    template <numeric T>
    inline vec2<T>& vec2<T>::operator-=(const vec2& rhs) noexcept
    {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    template <numeric T>
    inline vec2<T>& vec2<T>::operator*=(const vec2& rhs) noexcept
    {
        x *= rhs.x;
        y *= rhs.y;
        return *this;
    }

    template <numeric T>
    inline vec2<T>& vec2<T>::operator/=(const vec2& rhs) noexcept
    {
        x /= rhs.x;
        y /= rhs.y;
        return *this;
    }

    template <numeric T>
    inline bool operator==(const vec2<T>& lhs, const vec2<T>& rhs) noexcept
    {
        return lhs[0] == rhs[0] && lhs[1] == rhs[1];
    }

    template <numeric T>
    inline constexpr bool operator!=(const vec2<T>& lhs, const vec2<T>& rhs) noexcept
    {
        return lhs[0] != rhs[0] || lhs[1] != rhs[1];
    }

    template <numeric T>
    inline constexpr vec2<T> operator+(const vec2<T>& lhs, const vec2<T>& rhs) noexcept
    {
        const vec2<T> result = { lhs[0] + rhs[0], lhs[1] + rhs[1] };
        return result;
    }

    template <numeric T>
    inline constexpr vec2<T> operator-(const vec2<T>& lhs, const vec2<T>& rhs) noexcept
    {
        const vec2<T> result = { lhs[0] - rhs[0], lhs[1] - rhs[1] };
        return result;
    }

    template <numeric T>
    inline constexpr vec2<T> operator*(const vec2<T>& lhs, const vec2<T>& rhs) noexcept
    {
        const vec2<T> result = { lhs[0] * rhs[0], lhs[1] * rhs[1] };
        return result;
    }

    template <numeric T>
    inline constexpr vec2<T> operator*(const T scalar, const vec2<T>& rhs) noexcept
    {
        const vec2<T> result = { scalar * rhs[0], scalar * rhs[1] };
        return result;
    }

    template <numeric T>
    inline constexpr vec2<T> operator/(const vec2<T>& lhs, const vec2<T>& rhs) noexcept
    {
        const vec2<T> result = { lhs[0] / rhs[0], lhs[1] / rhs[1] };
        return result;
    }
}

#endif // vec2_hpp__
