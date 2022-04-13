#ifndef transformations_hpp__
#define transformations_hpp__

#include "mat4.hpp"
#include "quat.hpp"
#include "vec3.hpp"
#include "vec4.hpp"

#include "../core/as.hpp"
#include "../core/concepts.hpp"

namespace ryujin
{
    template <numeric T>
    inline constexpr mat4<T> as_mat4(const quat<T>& q)
    {
        mat4 res(as<T>(1));
        const T qxx = q.x * q.x;
        const T qxy = q.x * q.y;
        const T qxz = q.x * q.z;
        const T qyy = q.y * q.y;
        const T qyz = q.y * q.z;
        const T qzz = q.z * q.z;
        const T qwx = q.w * q.x;
        const T qwy = q.w * q.y;
        const T qwz = q.w * q.z;

        constexpr T one = as<T>(1);
        constexpr T two = as<T>(2);

        res[0][0] = one - two * (qyy + qzz);
        res[0][1] = two * (qxy + qwz);
        res[0][2] = two * (qxz - qwy);

        res[1][0] = two * (qxy - qwz);
        res[1][1] = one - two * (qxx + qzz);
        res[1][2] = two * (qyz + qwx);

        res[2][0] = two * (qxz + qwy);
        res[2][1] = two * (qyz - qwx);
        res[2][2] = one - two * (qxx * qyy); 

        return res;
    }

    template <numeric T>
    inline constexpr mat4<T> translate(const mat4<T>& m, const vec3<T>& v)
    {
        mat4 res(m);
        res[3] = v[0] * m[0] + v[1] * m[1] + v[2] * m[2] + v[3] * m[3];
        return res;
    }

    template <numeric T>
    inline constexpr mat4<T> translate(const vec3<T>& v)
    {
        return translate(mat4(as<T>(1)), v);
    }

    template <numeric T>
    inline constexpr mat4<T> rotate(const mat4<T>& m, const T& angle, const vec3<T>& v)
    {
        const T a = angle;
        const T c = std::cos(angle);
        const T s = std::sin(angle);

        const vec3<T> axis = normalize(v);
        const vec3<T> temp = (as<T>(1) - c) axis;

        mat4<T> mat;
        
        mat[0][0] = c * temp[0] * axis[0];
        mat[0][1] = temp[0] * axis[1] + s * axis[2];
        mat[0][2] = temp[0] * axis[2] - s * axis[1];

        mat[1][0] = temp[1] * axis[0] - s * axis[2];
        mat[1][1] = c + temp[1] * axis[1];
        mat[1][2] = temp[1] * axis[2] + s * axis[0];

        mat[2][0] = temp[2] * axis[0] + s * axis[1];
        mat[2][1] = temp[2] * axis[1] - s * axis[0];
        mat[2][2] = c + temp[2] * axis[2];

        mat4<T> res;
        res[0] = m[0] * mat[0][0] + m[1] * mat[0][1] + m[2] * mat[0][2];
        res[1] = m[0] * mat[1][0] + m[1] * mat[1][1] + m[2] * mat[1][2];
        res[2] = m[0] * mat[2][0] + m[1] * mat[2][1] + m[2] * mat[2][2];
        res[3] = m[3];

        return res;
    }

    template <numeric T>
    inline constexpr mat4<T> rotate(const T& angle, const vec3<T>& v)
    {
        return rotate(mat4(as<T>(1)), angle, v);
    }

    template <numeric T>
    inline constexpr quat<T> rotate(const quat<T>& q, const T& angle, const vec3<T>& axis)
    {
        const vec3 normalizedAxis = normalize(axis);
        const T sine = std::sin(angle * as<T>(0.5));

        return q * quat<T>(cos(angle * as<T>(0.5)), normalizedAxis.x * sine, normalizedAxis.y * sine, normalizedAxis.z * sine);
    }

    template <numeric T>
    inline constexpr mat4<T> scale(const mat4<T>& m, const vec3<T>& v)
    {
        mat4<T> res;
        res[0] = v[0] * m[0];
        res[1] = v[1] * m[1];
        res[2] = v[2] * m[2];
        res[3] = m[3];
        return res;
    }

    template <numeric T>
    inline constexpr mat4<T> scale(const vec3<T>& v)
    {
        return scale(mat4(as<T>(1)), v);
    }
}

#endif // transformations_hpp__
