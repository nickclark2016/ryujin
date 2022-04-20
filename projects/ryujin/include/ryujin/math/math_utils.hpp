#ifndef math_utils_hpp__
#define math_utils_hpp__

#include "../core/as.hpp"
#include "../core/concepts.hpp"
#include "../core/primitives.hpp"

#include <bit>
#include <cmath>
#include <type_traits>

namespace ryujin
{
    namespace constants
    {
        template <numeric T>
        constexpr T pi = as<T>(3.14159265359);

        template <numeric T>
        constexpr T half_pi = pi<T> / as<T>(2);

        template <numeric T>
        constexpr T inv_pi = as<T>(1) / pi<T>;
    };

    namespace detail
    {
        inline constexpr u32 as_u32_bits(const float v)
        {
            return std::bit_cast<u32>(v);
        }

        inline constexpr float as_float_bits(const u32 v)
        {
            return std::bit_cast<float>(v);
        }
    }

    template <numeric T>
    inline constexpr T fast_inv_sqrt(T value) noexcept
    {
        if constexpr (std::is_same_v<T, float>)
        {
            // Quake 3 Fast Inversion Square Root
            long i;
            float x2, y;
            const float threeHalfs = 1.5f;

            x2 = value * 0.5f;
            y = value;
            i = *reinterpret_cast<long*>(&y);
            i = 0x5f3759df - (i >> 1);
            y = *reinterpret_cast<float*>(&i);
            y = y * (threeHalfs - (x2 * y * y));
            return as<T>(y);
        }
        return as<T>(std::sqrt(value));
    }

    template <numeric T>
    inline constexpr T clamp(T value, T lower, T upper)
    {
        const T u = as<T>(std::min(value, upper));
        return as<T>(std::max(lower, u));
    }

    template <numeric T>
    inline constexpr T as_radians(const T degrees) noexcept
    {
        const T halfCircle = as<T>(180);
        return degrees / halfCircle * constants::pi<T>;
    }

    template <numeric T>
    inline constexpr T as_degrees(const T radians) noexcept
    {
        const T piRadians = radians * constants::inv_pi<T>;
        return piRadians * as<T>(180);
    }

    inline constexpr u16 compress_to_half(const float value)
    {
        return as<u16>(std::round(clamp(value, -1.0f, +1.0f) * 32767.0f));
    }

    inline constexpr f32 inflate_to_float(const u16 value) { // IEEE-754 16-bit floating-point format (without infinity): 1-5-10, exp-15, +-131008.0, +-6.1035156E-5, +-5.9604645E-8, 3.311 digits
        const u32 e = (value & 0x7C00) >> 10; // exponent
        const u32 m = (value & 0x03FF) << 13; // mantissa
        const u32 v = detail::as_u32_bits(as<float>(m)) >> 23; // evil log2 bit hack to count leading zeros in denormalized format
        return detail::as_float_bits((value & 0x8000) << 16 | (e != 0) * ((e + 112) << 23 | m) | ((e == 0) & (m != 0)) * ((v - 37) << 23 | ((m << (150 - v)) & 0x007FE000))); // sign : normalized : denormalized
    }

    template <numeric T>
    inline constexpr T inverse_lerp(const T value, const T low, const T high)
    {
        return (value - low) / (high - low);
    }

    template <numeric T>
    inline constexpr T lerp(const T low, const T high, const T t)
    {
        return low + t * (high - low);
    }

    template <numeric T>
    inline constexpr T reproject(const T value, const T oldMin, const T oldMax, const T newMin = as<T>(-1), const T newMax = as<T>(1))
    {
        const auto t = inverse_lerp(value, oldMin, oldMax);
        return lerp(newMin, newMax, t);
    }
}

#endif // math_utils_hpp__
