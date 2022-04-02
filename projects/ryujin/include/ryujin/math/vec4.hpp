#ifndef vec4_hpp__
#define vec4_hpp__

#include <cstddef>
#include <type_traits>

namespace ryujin
{
    template <typename T>
    struct alignas(16) vec4
    {
        union
        {
            float data[4];
            struct
            {
                float r;
                float g;
                float b;
                float a;
            };
            struct
            {
                float x;
                float y;
                float z;
                float w;
            };
        };

        constexpr vec4();
        constexpr vec4(const float scalar);
        constexpr vec4(const float x, const float y, const float z, const float w);

        constexpr T& operator[](const std::size_t index) noexcept;
        constexpr const T& operator[](const std::size_t index) const noexcept;

        vec4& operator+=(const vec4& rhs) noexcept;
        vec4& operator-=(const vec4& rhs) noexcept;
        vec4& operator*=(const vec4& rhs) noexcept;
        vec4& operator/=(const vec4& rhs) noexcept;
    };

    template <typename T>
    vec4(const T) -> vec4<T>;

    template <typename T>
    vec4(const T, const T, const T, const T) -> vec4<T>;

    // Implementation
    
    template<typename T>
    inline constexpr vec4<T>::vec4()
        : vec4(0)
    {
    }

    template<typename T>
    inline constexpr vec4<T>::vec4(const float scalar)
        : vec4(scalar, scalar, scalar, scalar)
    {
    }

    template<typename T>
    inline constexpr vec4<T>::vec4(const float x, const float y, const float z, const float w)
        : x(x), y(y), z(z), w(w)
    {
    }
    
    template<typename T>
    inline constexpr T& vec4<T>::operator[](const std::size_t index) noexcept
    {
        return data[index];
    }
    
    template<typename T>
    inline constexpr const T& vec4<T>::operator[](const std::size_t index) const noexcept
    {
        return data[index];
    }

    template<typename T>
    inline vec4<T>& vec4<T>::operator+=(const vec4& rhs) noexcept
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        w += rhs.w;
        return *this;
    }

    template<typename T>
    inline vec4<T>& vec4<T>::operator-=(const vec4& rhs) noexcept
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        w -= rhs.w;
        return *this;
    }

    template<typename T>
    inline vec4<T>& vec4<T>::operator*=(const vec4& rhs) noexcept
    {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
        w *= rhs.w;
        return *this;
    }

    template<typename T>
    inline vec4<T>& vec4<T>::operator/=(const vec4& rhs) noexcept
    {
        x /= rhs.x;
        y /= rhs.y;
        z /= rhs.z;
        w /= rhs.w;
        return *this;
    }

    template <typename T>
    inline bool operator==(const vec4<T>& lhs, const vec4<T>& rhs) noexcept
    {
        return lhs[0] == rhs[0] && lhs[1] == rhs[1] && lhs[2] == rhs[2] && lhs[3] == rhs[3];
    }

    template <typename T>
    inline constexpr bool operator!=(const vec4<T>& lhs, const vec4<T>& rhs) noexcept
    {
        return lhs[0] != rhs[0] || lhs[1] != rhs[1] || lhs[2] != rhs[2] || lhs[3] != rhs[3];
    }

    template <typename T>
    inline constexpr vec4<T> operator+(const vec4<T>& lhs, const vec4<T>& rhs) noexcept
    {
        const vec4<T> result = { lhs[0] + rhs[0], lhs[1] + rhs[1], lhs[2] + rhs[2], lhs[3] + rhs[3] };
        return result;
    }

    template <typename T>
    inline constexpr vec4<T> operator-(const vec4<T>& lhs, const vec4<T>& rhs) noexcept
    {
        const vec4<T> result = { lhs[0] - rhs[0], lhs[1] - rhs[1], lhs[2] - rhs[2], lhs[3] - rhs[3] };
        return result;
    }

    template <typename T>
    inline constexpr vec4<T> operator*(const vec4<T>& lhs, const vec4<T>& rhs) noexcept
    {
        const vec4<T> result = { lhs[0] * rhs[0], lhs[1] * rhs[1], lhs[2] * rhs[2], lhs[3] * rhs[3] };
        return result;
    }

    template <typename T>
    inline constexpr vec4<T> operator*(const float scalar, const vec4<T>& rhs) noexcept
    {
        const vec4<T> result = { scalar * rhs[0], scalar * rhs[1], scalar * rhs[2], scalar * rhs[3] };
        return result;
    }

    template <typename T>
    inline constexpr vec4<T> operator/(const vec4<T>& lhs, const vec4<T>& rhs) noexcept
    {
        const vec4<T> result = { lhs[0] / rhs[0], lhs[1] / rhs[1], lhs[2] / rhs[2], lhs[3] / rhs[3] };
        return result;
    }
}

#endif // vec4_hpp__
