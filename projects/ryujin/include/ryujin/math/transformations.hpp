#ifndef transformations_hpp__
#define transformations_hpp__

#include "mat3.hpp"
#include "mat4.hpp"
#include "quat.hpp"
#include "vec2.hpp"
#include "vec3.hpp"
#include "vec4.hpp"

#include "../core/as.hpp"
#include "../core/concepts.hpp"
#include "../core/primitives.hpp"

#include <cassert>

namespace ryujin
{
    template <numeric T>
    constexpr vec3<T> front = vec3<T>(as<T>(0), as<T>(0), as<T>(1));

    template <numeric T>
    constexpr vec3<T> up = vec3<T>(as<T>(0), as<T>(1), as<T>(0));

    template <numeric T>
    constexpr vec3<T> right = vec3<T>(as<T>(1), as<T>(0), as<T>(0));

    template <numeric T>
    inline constexpr mat3<T> as_mat3(const quat<T>& q)
    {
        const T n = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
        const T s = n > 0 ? 2 / n : 0;
        const T x = s * q.x;
        const T y = s * q.y;
        const T z = s * q.z;
        const T xx = x * q.x;
        const T xy = x * q.y;
        const T xz = x * q.z;
        const T xw = x * q.w;
        const T yy = y * q.y;
        const T yz = y * q.z;
        const T yw = y * q.w;
        const T zz = z * q.z;
        const T zw = z * q.w;

        mat3<T> res;

        res[0] = vec3<T>{ as<T>(1) - yy - zz, xy + zw, xz - yw};  // NOLINT
        res[1] = vec3<T>{ xy - zw, as<T>(1) - xx - zz, yz + xw };  // NOLINT
        res[2] = vec3<T>{ xz + yw, yz - xw, as<T>(1) - xx - yy };  // NOLINT

        return res;
    }

    template <numeric T>
    inline constexpr quat<T> as_quat(const mat3<T>& m)
    {
        quat<T> quat(as<T>(0));

        // Compute the trace to see if it is positive or not.
        const T trace = m[0][0] + m[1][1] + m[2][2];

        // check the sign of the trace
        if (trace > 0) {
            // trace is positive
            T s = std::sqrt(trace + 1);
            quat.w = T(0.5) * s;
            s = T(0.5) / s;
            quat.x = (m[1][2] - m[2][1]) * s;
            quat.y = (m[2][0] - m[0][2]) * s;
            quat.z = (m[0][1] - m[1][0]) * s;
        }
        else {
            // trace is negative

            // Find the index of the greatest diagonal
            size_t i = 0;
            if (m[1][1] > m[0][0]) { i = 1; }
            if (m[2][2] > m[i][i]) { i = 2; }

            // Get the next indices: (n+1)%3
            static constexpr size_t next_ijk[3] = { 1, 2, 0 };
            size_t j = next_ijk[i];
            size_t k = next_ijk[j];
            T s = std::sqrt((m[i][i] - (m[j][j] + m[k][k])) + 1);
            quat[i] = T(0.5) * s;
            if (s != 0) {
                s = T(0.5) / s;
            }
            quat.w = (m[j][k] - m[k][j]) * s;
            quat[j] = (m[i][j] + m[j][i]) * s;
            quat[k] = (m[i][k] + m[k][i]) * s;
        }
        return quat;
    }

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
        res[3] = v[0] * m[0] + v[1] * m[1] + v[2] * m[2] + as<T>(1) * m[3];
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
        const T c = cos(a);
        const T s = sin(a);

        vec3<T> axis(normalize(v));
        vec3<T> temp((as<T>(1) - c) * axis);

        mat4<T> rot;
        rot[0][0] = c + temp[0] * axis[0];
        rot[0][1] = temp[0] * axis[1] + s * axis[2];
        rot[0][2] = temp[0] * axis[2] - s * axis[1];

        rot[1][0] = temp[1] * axis[0] - s * axis[2];
        rot[1][1] = c + temp[1] * axis[1];
        rot[1][2] = temp[1] * axis[2] + s * axis[0];

        rot[2][0] = temp[2] * axis[0] + s * axis[1];
        rot[2][1] = temp[2] * axis[1] - s * axis[0];
        rot[2][2] = c + temp[2] * axis[2];

        mat4<T> res;
        res[0] = m[0] * rot[0][0] + m[1] * rot[0][1] + m[2] * rot[0][2];
        res[1] = m[0] * rot[1][0] + m[1] * rot[1][1] + m[2] * rot[1][2];
        res[2] = m[0] * rot[2][0] + m[1] * rot[2][1] + m[2] * rot[2][2];
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

    template <numeric T>
    inline constexpr mat4<T> transform(const vec3<T>& translation, const quat<T>& rotation, const vec3<T>& scale)
    {
        // transformation = translation * rotation * scale
        const auto translating = translate(translation);
        const auto scaling = ryujin::scale(scale);
        const auto rotating = as_mat4(rotation);
        return translating * rotating * scaling;
    }

    template <numeric T>
    inline constexpr mat4<T> transform(const vec3<T>& translation, const vec3<T>& rotation, const vec3<T>& scale)
    {
        // transformation = translation * rotation * scale
        const auto translating = translate(translation);
        auto tr = rotate(translating, rotation.x, right<T>);
        tr = rotate(tr, rotation.y, up<T>);
        tr = rotate(tr, rotation.z, front<T>);
        const auto scaling = ryujin::scale(tr, scale);
        return scaling;
    }

    template <numeric T>
    inline constexpr bool decompose(const mat4<T>& transformationMatrix, vec3<T>& translate, quat<T> rotation, vec3<T>& scale)
    {
        auto local = transformationMatrix;

        // Matrix normalization
        if (local[3][3] == 0)
        {
            return false;
        }

        for (sz i = 0; i < 4; ++i)
        {
            for (sz j = 0; j < 4; ++j)
            {
                local[i][j] /= local[3][3];
            }
        }

        // solve for translation and remove
        vec4 translation = local[3];
        translate.x = translation.x;
        translate.y = translation.y;
        translate.z = translation.z;
        local[3] = vec4(as<T>(0), as<T>(0), as<T>(0), translation.w);

        // solve for scale
        vec3<float> row[3];
        for (sz i = 0; i < 3; ++i)
        {
            for (sz j = 0; j < 3; ++j)
            {
                row[i][j] = local[i][j];
            }
        }

        scale.x = norm(row[0]); // x scale is the length of the first row
        scale.y = norm(row[1]);
        scale.z = norm(row[2]);

        T root, trace = row[0].x + row[1].y + row[2].z;
        if (trace > as<T>(0))
        {
            root = std::sqrt(trace + as<T>(1));
            rotation.w = as<T>(0.5) * root;
            root = as<T>(0.5) / root;
            rotation.x = root * (row[1].z - row[2].y);
            rotation.y = root * (row[2].x - row[0].z);
            rotation.z = root * (row[0].y - row[1].x);
        }
        else
        {
            constexpr sz next[3] = { 1, 2, 0 };
            sz i = 0;
            if (row[1].y > row[0].x) i = 1;
            if (row[2].z > row[i][i]) i = 2;
            sz j = next[i];
            sz k = next[j];

            root = std::sqrt(row[i][i] - row[j][j] - row[k][k] + as<T>(1));
            rotation[i + 1] = as<T>(0.5) * root;
            root = as<T>(0.5) / root;
            rotation[j + 1] = root * (row[i][j] + row[j][i]);
            rotation[k + 1] = root * (row[i][k] + row[k][i]);
            rotation.w = root * (row[j][k] - row[k][j]);
        }

        return true;
    }

    template <numeric T>
    inline constexpr mat4<T> perspective(const T aspect, const T fov, const T near, const T far)
    {
        assert(near < far && "Near plane distance must be less than far plane distance");
        // 1 / (aspect * tan(fov / 2))  0                       0                               0
        // 0                            1 / tan(fov / 2)        0                               0
        // 0                            0                       (-near - far) / (near - far)    (2 * near * far) / (near - far)
        // 0                            0                       1                               0
        const T fovRad = as_radians(fov);
        const T tanFov2 = as<T>(std::tan(fovRad / as<T>(2)));
        const T invTanFov2 = as<T>(1) / tanFov2;
        const T invAspectTanFov2 = invTanFov2 * (as<T>(1) / aspect);
        const T nearMinusFar = near - far;

        const T zero = as<T>(0);

        const vec4<T> col0(invAspectTanFov2, zero, zero, zero);
        const vec4<T> col1(zero, invTanFov2, zero, zero);
        const vec4<T> col2(zero, zero, (-near - far) / nearMinusFar, as<T>(1));
        const vec4<T> col3(zero, zero, (as<T>(2) * near * far) / nearMinusFar, zero);
        return mat4(col0, col1, col2, col3);
    }

    template <numeric T>
    inline constexpr mat4<T> look_at(const vec3<T>& eye, const vec3<T>& target, const vec3<T>& up)
    {
        const auto fwd = normalize(target - eye);
        const auto side = normalize(cross(up, fwd));
        const auto u = cross(fwd, side);

        mat4 look(as<T>(1));

        look[0][0] = side.x;
        look[1][0] = side.y;
        look[2][0] = side.z;
        look[0][1] = u.x;
        look[1][1] = u.y;
        look[2][1] = u.z;
        look[0][2] = fwd.x;
        look[1][2] = fwd.y;
        look[2][2] = fwd.z;
        look[3][0] = -dot(side, eye);
        look[3][1] = -dot(up, eye);
        look[3][2] = -dot(fwd, eye);

        return look;
    }

    template <numeric T>
    inline constexpr mat4<T> look_direction(const vec3<T>& eye, const vec3<T>& forwards, const vec3<T>& up)
    {
        const auto fwd = normalize(forwards);
        const auto side = normalize(cross(up, fwd));
        const auto u = cross(fwd, side);

        mat4 look(as<T>(1));

        look[0][0] = side.x;
        look[1][0] = side.y;
        look[2][0] = side.z;
        look[0][1] = u.x;
        look[1][1] = u.y;
        look[2][1] = u.z;
        look[0][2] = fwd.x;
        look[1][2] = fwd.y;
        look[2][2] = fwd.z;
        look[3][0] = -dot(side, eye);
        look[3][1] = -dot(up, eye);
        look[3][2] = -dot(fwd, eye);

        return look;
    }

    template <numeric T>
    inline constexpr mat3<T> tbn(const vec3<T>& tangent, const vec3<T>& bitangent, const vec3<T>& normal)
    {
        return mat3(tangent, bitangent, normal);
    }

    template <numeric T>
    inline constexpr quat<T> encode_tbn(const mat3<T> tbn)
    {
        const mat3 tmp = { tbn[0], cross(tbn[2], tbn[0]), tbn[2] };
        quat q = normalize(as_quat(tmp));
        q = q.w < 0 ? -q : q;

        constexpr T bias = as<T>(1) / as<T>((1 << (sizeof(u16) * 8 - 1)) - 1);
        if (q.w < bias)
        {
            q.w = bias;
            const T factor = as<T>(std::sqrt(as<T>(1) - bias * bias));
            q.x *= factor;
            q.y *= factor;
            q.z *= factor;
        }

        const vec3<T> binorm = cross(tbn[0], tbn[2]);
        const T direction = dot(binorm, tbn[1]);
        if (direction < 0)
        {
            q = -q;
        }

        return q;
    }

    template <numeric T>
    inline constexpr vec3<T> extract_forward(const quat<T>& rotation)
    {
        const T x = as<T>(2) * (rotation.x * rotation.z + rotation.w * rotation.y);
        const T y = as<T>(2) * (rotation.y * rotation.z - rotation.w * rotation.x);
        const T z = as<T>(1) - as<T>(2) * (rotation.x * rotation.x + rotation.y * rotation.y);
        return vec3(x, y, z);
    }

    template <numeric T>
    inline constexpr vec3<T> extract_up(const quat<T>& rotation)
    {
        const T x = as<T>(2) * (rotation.x * rotation.y - rotation.w * rotation.z);
        const T y = as<T>(1) - as<T>(2) * (rotation.x * rotation.x + rotation.z * rotation.z);
        const T z = as<T>(2) * (rotation.y * rotation.z + rotation.w * rotation.x);
        return vec3(x, y, z);
    }

    template <numeric T>
    inline constexpr vec2<T> encode_direction_to_euler_angles(const vec3<T>& dir)
    {
        const auto d = normalize(dir);
        const T theta = std::atan(as<T>(1) / d.z);
        const T phi = std::atan(d.y / d.x);
        return { as_degrees(theta), as_degrees(phi) };
    }
}

#endif // transformations_hpp__
