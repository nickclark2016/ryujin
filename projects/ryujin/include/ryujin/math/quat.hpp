#ifndef quat_hpp__
#define quat_hpp__

#include "math_utils.hpp"
#include "vec3.hpp"

#include "../core/as.hpp"
#include "../core/concepts.hpp"
#include "../core/primitives.hpp"

#include <cmath>
#include <cstddef>
#include <type_traits>

namespace ryujin
{
    template <numeric T>
    struct alignas(sizeof(T) * 4) quat
    {
        union
        {
            T data[4];
            struct
            {
                T w;
                T x;
                T y;
                T z;
            };
        };

        constexpr quat();
        constexpr quat(const T scalar);
        constexpr quat(const T w, const T x, const T y, const T z);
        constexpr quat(const vec3<T>& euler);

        constexpr T& operator[](const sz index) noexcept;
        constexpr const T& operator[](const sz index) const noexcept;
    };

    template <numeric T>
    quat(const T) -> quat<T>;

    template <numeric T>
    quat(const T, const T, const T, const T) -> quat<T>;

    template <typename T>
    quat(const vec3<T>&) -> quat<T>;

    template <numeric T>
    quat(const quat<T>&) -> quat<T>;

    template <numeric T>
    quat(quat<T>&&) -> quat<T>;

    template <numeric T>
    inline constexpr quat<T>::quat()
        : quat(T(1), T(0), T(0), T(0))
    {
    }

    template <numeric T>
    inline constexpr quat<T>::quat(const T scalar)
        : quat(scalar, scalar, scalar, scalar)
    {
    }

    template <numeric T>
    inline constexpr quat<T>::quat(const T w, const T x, const T y, const T z)
        : w(w), x(x), y(y), z(z)
    {
    }

    template <numeric T>
    inline constexpr quat<T>::quat(const vec3<T>& euler)
    {
        const vec3<T> half = as<T>(0.5) * euler;
        const vec3<T> c(std::cos(half.x), std::cos(half.y), std::cos(half.z));
        const vec3<T> s(std::sin(half.x), std::sin(half.y), std::sin(half.z));

        w = c.x * c.y * c.z + s.x * s.y * s.z;
        x = s.x * c.y * c.z - c.x * s.y * s.z;
        y = c.x * s.y * c.z + s.x * c.y * s.z;
        z = c.x * c.y * s.z - s.x * s.y * c.z;
    }
    
    template <numeric T>
    inline constexpr T& quat<T>::operator[](const sz index) noexcept
    {
        return data[index];
    }
    
    template <numeric T>
    inline constexpr const T& quat<T>::operator[](const sz index) const noexcept
    {
        return data[index];
    }

    template <numeric T>
    inline constexpr quat<T> operator*(const quat<T>& lhs, const quat<T>& rhs)
    {
        // w0 * w1 - x0 * x1 - y0 * y1 - z0 * z1
        // w0 * x1 + x0 * w1 + y0 * z1 - z0 * y1
        // w0 * y1 - x0 * z1 + y0 * w1 + z0 * x1
        // w0 * z1 + x0 * y1 - y0 * x1 + z0 * w1

        return quat4(
            lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z,
            lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y,
            lhs.w * rhs.y - lhs.x * rhs.z + lhs.y * rhs.w + lhs.z * rhs.x,
            lhs.w * rhs.z + lhs.x * rhs.y - lhs.y * rhs.x + lhs.z * rhs.w
        );
    }

    template <numeric T>
    inline constexpr quat<T> operator*(const T lhs, const quat<T>& rhs)
    {
        return quat4(
            lhs * rhs.w,
            lhs * rhs.x,
            lhs * rhs.y,
            lhs * rhs.z
        );
    }

    template <numeric T>
    inline constexpr T norm(const quat<T>& q)
    {
        const T magSquared = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
        return std::sqrt(magSquared);
    }

    template <numeric T>
    inline constexpr quat<T> normalize(const quat<T>& q)
    {
        const T magSquared = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
        const T invSqrt = fast_inv_sqrt(magSquared);
        return invSqrt * q;
    }

    template <numeric T>
    inline constexpr T roll(const quat<T>& q)
    {
        const T x = q.w * q.w + q.z * q.z - q.y * q.y - q.z * q.z;
        const T y = as<T>(2) * (q.x * q.y + q.w * q.z);
        
        if (x == as<T>(0) && y == as<T>(0))
        {
            return as<T>(0);
        }

        return as<T>(std::atan2(y, x));
    }

    template <numeric T>
    inline constexpr T pitch(const quat<T>& q)
    {
        const T x = q.w * q.w - q.x * q.x - q.y * q.y + q.z * q.z;
        const T y = as<T>(2) * (q.y * q.z + q.w * q.x);
        
        if (x == as<T>(0) && y == as<T>(0))
        {
            return as<T>(as<T>(2) * std::atan2(q.x, q.w));
        }

        return as<T>(std::atan2(y, x));
    }

    template <numeric T>
    inline constexpr T yaw(const quat<T>& q)
    {
        return std::asin(clamp(as<T>(-2) * (q.x * q.z - q.w * q.y), as<T>(-1), as<T>(1)));
    }

    template <numeric T>
    inline constexpr vec3<T> euler(const quat<T>& q)
    {
        const T x = pitch(q);
        const T y = yaw(q);
        const T z = roll(q);
        return vec3(x, y, z);
    }
}

#endif // quat_hpp__