#ifndef vec3_hpp__
#define vec3_hpp__

#include "../core/concepts.hpp"

#include <cstddef>
#include <type_traits>

namespace ryujin
{
    template <numeric T>
    struct alignas(sizeof(T) * 4) vec3
    {
        union
        {
            float data[4];
            struct
            {
                float r;
                float g;
                float b;
            };
            struct
            {
                float x;
                float y;
                float z;
            };
        };

        constexpr vec3();
        constexpr vec3(const T scalar);
        constexpr vec3(const T x, const T y, const T z);

        constexpr T& operator[](const std::size_t index) noexcept;
        constexpr const T& operator[](const std::size_t index) const noexcept;

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
    inline constexpr T& vec3<T>::operator[](const std::size_t index) noexcept
    {
        return data[index];
    }
    
    template <numeric T>
    inline constexpr const T& vec3<T>::operator[](const std::size_t index) const noexcept
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
    inline constexpr vec3<T> operator*(const float scalar, const vec3<T>& rhs) noexcept
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
}

#endif // vec3_hpp__
