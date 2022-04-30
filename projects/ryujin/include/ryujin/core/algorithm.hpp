#ifndef algorithm_hpp__
#define algorithm_hpp__

#include "utility.hpp"

namespace ryujin
{
    /// \defgroup Algorithms

    /// <summary>
    /// Moves the values from one iterator to another.
    /// </summary>
    /// <typeparam name="Input">Input iterator type</typeparam>
    /// <typeparam name="Output">Output iterator type</typeparam>
    /// <param name="first">Iterator at start of range to move from (inclusive)</param>
    /// <param name="last">Iterator at end of range to move from (exclusive)</param>
    /// <param name="result">Iterator at start of result range to move into</param>
    /// <returns>Iterator to the end of the output range moved to</returns>
    /// \ingroup Algorithms
    template <typename Input, typename Output>
    inline constexpr Output move(Input first, Input last, Output result)
    {
        while (first != last)
        {
            *result = ryujin::move(*first);
            ++first;
            ++result;
        }
        return result;
    }
}

#endif // algorithm_hpp__
