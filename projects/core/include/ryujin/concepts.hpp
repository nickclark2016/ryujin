#ifndef concepts_hpp__
#define concepts_hpp__

#include <type_traits>

namespace ryujin
{
    /// <summary>
    /// Concept requiring a type to be an integer or floating point type.
    /// </summary>
    template <typename T>
    concept numeric = std::is_integral_v<T> || std::is_floating_point_v<T>;
}

#endif // concepts_hpp__
