#ifndef vec3_hpp__
#define vec3_hpp__

#include "math_utils.hpp"

#include "../core/concepts.hpp"
#include "../core/primitives.hpp"

#include <cstddef>
#include <type_traits>

namespace ryujin
{
    template <numeric T>
    struct alignas(sizeof(T) * 4) vec3
    {
        union
        {
            T data[4];
            struct
            {
                T r;
                T g;
                T b;
            };
            struct
            {
                T x;
                T y;
                T z;
            };
        };

        constexpr vec3();
        constexpr vec3(const T scalar);
        constexpr vec3(const T x, const T y, const T z);

        constexpr T& operator[](const sz index) noexcept;
        constexpr const T& operator[](const sz index) const noexcept;

        vec3& operator+=(const vec3& rhs) noexcept;
        vec3& operator-=(const vec3& rhs) noexcept;
        vec3& operator*=(const vec3& rhs) noexcept;
        vec3& operator/=(const vec3& rhs) noexcept;
    };

    template <numeric T>
    vec3(const T) -> vec3<T>;

    template <numeric T>
    vec3(const T, const T, const T, const T) -> vec3<T>;

    template <numeric T>
    vec3(const vec3<T>&) -> vec3<T>;

    template <numeric T>
    vec3(vec3<T>&&) -> vec3<T>;

    // Implementation
    
    template <numeric T>
    inline constexpr vec3<T>::vec3()
        : vec3(T())
    {
    }

    template <numeric T>
    inline constexpr vec3<T>::vec3(const T scalar)
        : vec3(scalar, scalar, scalar)
    {
    }

    template <numeric T>
    inline constexpr vec3<T>::vec3(const T x, const T y, const T z)
        : x(x), y(y), z(z)
    {
    }
    
    template <numeric T>
    inline constexpr T& vec3<T>::operator[](const sz index) noexcept
    {
        return data[index];
    }
    
    template <numeric T>
    inline constexpr const T& vec3<T>::operator[](const sz index) const noexcept
    {
        return data[index];
    }

    template <numeric T>
    inline vec3<T>& vec3<T>::operator+=(const vec3& rhs) noexcept
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }

    template <numeric T>
    inline vec3<T>& vec3<T>::operator-=(const vec3& rhs) noexcept
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }

    template <numeric T>
    inline vec3<T>& vec3<T>::operator*=(const vec3& rhs) noexcept
    {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
        return *this;
    }

    template <numeric T>
    inline vec3<T>& vec3<T>::operator/=(const vec3& rhs) noexcept
    {
        x /= rhs.x;
        y /= rhs.y;
        z /= rhs.z;
        return *this;
    }

    template <numeric T>
    inline bool operator==(const vec3<T>& lhs, const vec3<T>& rhs) noexcept
    {
        return lhs[0] == rhs[0] && lhs[1] == rhs[1] && lhs[2] == rhs[2];
    }

    template <numeric T>
    inline constexpr bool operator!=(const vec3<T>& lhs, const vec3<T>& rhs) noexcept
    {
        return lhs[0] != rhs[0] || lhs[1] != rhs[1] || lhs[2] != rhs[2];
    }

    template <numeric T>
    inline constexpr vec3<T> operator+(const vec3<T>& lhs, const vec3<T>& rhs) noexcept
    {
        const vec3<T> result = { lhs[0] + rhs[0], lhs[1] + rhs[1], lhs[2] + rhs[2] };
        return result;
    }

    template <numeric T>
    inline constexpr vec3<T> operator-(const vec3<T>& lhs, const vec3<T>& rhs) noexcept
    {
        const vec3<T> result = { lhs[0] - rhs[0], lhs[1] - rhs[1], lhs[2] - rhs[2] };
        return result;
    }

    template <numeric T>
    inline constexpr vec3<T> operator*(const vec3<T>& lhs, const vec3<T>& rhs) noexcept
    {
        const vec3<T> result = { lhs[0] * rhs[0], lhs[1] * rhs[1], lhs[2] * rhs[2] };
        return result;
    }

    template <numeric T>
    inline constexpr vec3<T> operator*(const T scalar, const vec3<T>& rhs) noexcept
    {
        const vec3<T> result = { scalar * rhs[0], scalar * rhs[1], scalar * rhs[2] };
        return result;
    }

    template <numeric T>
    inline constexpr vec3<T> operator/(const vec3<T>& lhs, const vec3<T>& rhs) noexcept
    {
        const vec3<T> result = { lhs[0] / rhs[0], lhs[1] / rhs[1], lhs[2] / rhs[2] };
        return result;
    }

    template <numeric T>
    inline constexpr T norm(const vec3<T>& v)
    {
        const T magSquared = v.x * v.x + v.y * v.y + v.z * v.z;
        return std::sqrt(magSquared);
    }

    template <numeric T>
    inline constexpr vec3<T> normalize(const vec3<T>& v)
    {
        const T magSquared = v.x * v.x + v.y * v.y + v.z * v.z;
        return fast_inv_sqrt(magSquared) * v;
    }

    template <numeric T>
    inline constexpr vec3<T> cross(const vec3<T>& lhs, const vec3<T>& rhs)
    {
        return vec3(
            lhs[1] * rhs[2] - rhs[1] * lhs[2],
            lhs[2] * rhs[0] - rhs[2] * lhs[0],
            lhs[0] * rhs[1] - rhs[0] * lhs[1]
        );
    }

    template <numeric T>
    inline constexpr T dot(const vec3<T>& lhs, const vec3<T>& rhs)
    {
        const auto prod = lhs * rhs;
        return prod[0] + prod[1] + prod[2];
    }
}

#endif // vec3_hpp__
