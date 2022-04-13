#ifndef vec4_hpp__
#define vec4_hpp__

#include "../core/concepts.hpp"

#include <cstddef>
#include <type_traits>

namespace ryujin
{
    template <numeric T>
    struct alignas(sizeof(T) * 4) vec4
    {
        union
        {
            T data[4];
            struct
            {
                T r;
                T g;
                T b;
                T a;
            };
            struct
            {
                T x;
                T y;
                T z;
                T w;
            };
        };

        constexpr vec4();
        constexpr vec4(const T scalar);
        constexpr vec4(const T x, const T y, const T z, const T w);

        constexpr T& operator[](const std::size_t index) noexcept;
        constexpr const T& operator[](const std::size_t index) const noexcept;

        vec4& operator+=(const vec4& rhs) noexcept;
        vec4& operator-=(const vec4& rhs) noexcept;
        vec4& operator*=(const vec4& rhs) noexcept;
        vec4& operator/=(const vec4& rhs) noexcept;
    };

    template <numeric T>
    vec4(const T) -> vec4<T>;

    template <numeric T>
    vec4(const T, const T, const T, const T) -> vec4<T>;

    template <numeric T>
    vec4(const vec4<T>&) -> vec4<T>;

    template <numeric T>
    vec4(vec4<T>&&) -> vec4<T>;

    // Implementation
    
    template <numeric T>
    inline constexpr vec4<T>::vec4()
        : vec4(T())
    {
    }

    template <numeric T>
    inline constexpr vec4<T>::vec4(const T scalar)
        : vec4(scalar, scalar, scalar, scalar)
    {
    }

    template <numeric T>
    inline constexpr vec4<T>::vec4(const T x, const T y, const T z, const T w)
        : x(x), y(y), z(z), w(w)
    {
    }
    
    template <numeric T>
    inline constexpr T& vec4<T>::operator[](const std::size_t index) noexcept
    {
        return data[index];
    }
    
    template <numeric T>
    inline constexpr const T& vec4<T>::operator[](const std::size_t index) const noexcept
    {
        return data[index];
    }

    template <numeric T>
    inline vec4<T>& vec4<T>::operator+=(const vec4& rhs) noexcept
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        w += rhs.w;
        return *this;
    }

    template <numeric T>
    inline vec4<T>& vec4<T>::operator-=(const vec4& rhs) noexcept
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        w -= rhs.w;
        return *this;
    }

    template <numeric T>
    inline vec4<T>& vec4<T>::operator*=(const vec4& rhs) noexcept
    {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
        w *= rhs.w;
        return *this;
    }

    template <numeric T>
    inline vec4<T>& vec4<T>::operator/=(const vec4& rhs) noexcept
    {
        x /= rhs.x;
        y /= rhs.y;
        z /= rhs.z;
        w /= rhs.w;
        return *this;
    }

    template <numeric T>
    inline bool operator==(const vec4<T>& lhs, const vec4<T>& rhs) noexcept
    {
        return lhs[0] == rhs[0] && lhs[1] == rhs[1] && lhs[2] == rhs[2] && lhs[3] == rhs[3];
    }

    template <numeric T>
    inline constexpr bool operator!=(const vec4<T>& lhs, const vec4<T>& rhs) noexcept
    {
        return lhs[0] != rhs[0] || lhs[1] != rhs[1] || lhs[2] != rhs[2] || lhs[3] != rhs[3];
    }

    template <numeric T>
    inline constexpr vec4<T> operator+(const vec4<T>& lhs, const vec4<T>& rhs) noexcept
    {
        const vec4<T> result = { lhs[0] + rhs[0], lhs[1] + rhs[1], lhs[2] + rhs[2], lhs[3] + rhs[3] };
        return result;
    }

    template <numeric T>
    inline constexpr vec4<T> operator-(const vec4<T>& lhs, const vec4<T>& rhs) noexcept
    {
        const vec4<T> result = { lhs[0] - rhs[0], lhs[1] - rhs[1], lhs[2] - rhs[2], lhs[3] - rhs[3] };
        return result;
    }

    template <numeric T>
    inline constexpr vec4<T> operator*(const vec4<T>& lhs, const vec4<T>& rhs) noexcept
    {
        const vec4<T> result = { lhs[0] * rhs[0], lhs[1] * rhs[1], lhs[2] * rhs[2], lhs[3] * rhs[3] };
        return result;
    }

    template <numeric T>
    inline constexpr vec4<T> operator*(const T scalar, const vec4<T>& rhs) noexcept
    {
        const vec4<T> result = { scalar * rhs[0], scalar * rhs[1], scalar * rhs[2], scalar * rhs[3] };
        return result;
    }

    template <numeric T>
    inline constexpr vec4<T> operator/(const vec4<T>& lhs, const vec4<T>& rhs) noexcept
    {
        const vec4<T> result = { lhs[0] / rhs[0], lhs[1] / rhs[1], lhs[2] / rhs[2], lhs[3] / rhs[3] };
        return result;
    }
}

#endif // vec4_hpp__
