#ifndef math_utils_hpp__
#define math_utils_hpp__

#include "../core/as.hpp"
#include "../core/concepts.hpp"

#include <cmath>
#include <type_traits>

namespace ryujin
{
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
}

#endif // math_utils_hpp__
