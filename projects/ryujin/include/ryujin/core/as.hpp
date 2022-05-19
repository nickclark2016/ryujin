#ifndef as_hpp__
#define as_hpp__

#include "export.hpp"

namespace ryujin
{
    /// <summary>
    /// Utility method for casting a value from one type to another.
    /// </summary>
    /// <typeparam name="T">Type to cast to</typeparam>
    /// <typeparam name="U">Type to cast from</typeparam>
    /// <param name="value">Value to cast</param>
    /// <returns>Casted value</returns>
    template <typename T, typename U>
    RYUJIN_API inline constexpr T as(U value)
    {
        return static_cast<T>(value);
    }
}

#endif // as_hpp__
