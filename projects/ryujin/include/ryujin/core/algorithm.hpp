#ifndef algorithm_hpp__
#define algorithm_hpp__

#include "export.hpp"
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
    RYUJIN_API inline constexpr Output move(Input first, Input last, Output result)
    {
        while (first != last)
        {
            *result = ryujin::move(*first);
            ++first;
            ++result;
        }
        return result;
    }

    /// <summary>
    /// Copies the values from one iterator to another.
    /// </summary>
    /// <typeparam name="Input">Input iterator type</typeparam>
    /// <typeparam name="Output">Output iterator type</typeparam>
    /// <param name="first">Iterator at start of range to copy from (inclusive)</param>
    /// <param name="last">Iterator at end of range to copy from (exclusive)</param>
    /// <param name="result">Iterator at start of result range to copy into</param>
    /// <returns>Iterator to the end of the output range copy to</returns>
    /// \ingroup Algorithms
    template <typename Input, typename Output>
    RYUJIN_API inline constexpr Output copy(Input first, Input last, Output out)
    {
        const ptr_diff diff = last - first;
        if constexpr (is_same_v<Input, Output> && is_trivially_copyable_v<Input>)
        {
            ryujin::memcpy(out, first, diff * sizeof(decltype(*first)));
            return out + diff;
        }
        else
        {
            while (first != last)
            {
                *out = *first;

                ++first;
                ++out;
            }
            return out;
        }
    }
    
    /// <summary>
    /// Returns the minimum of two values.  Evaluated as lhs < rhs.
    /// </summary>
    /// <typeparam name="T">Type of values to get minimum of. Type must be comparable.</typeparam>
    /// <param name="lhs">Left hand argument</param>
    /// <param name="rhs">Right hand argument</param>
    /// <returns>Minimum argument</returns>
    /// \ingroup Algorithms
    template <typename T>
    inline constexpr const T& min(const T& lhs, const T& rhs)
    {
        return (lhs < rhs) ? lhs : rhs;
    }

    /// <summary>
    /// Returns the minimum of two values.  Evaluated as lhs < rhs.
    /// </summary>
    /// <typeparam name="T">Type of values to get minimum of.</typeparam>
    /// <typeparam name="Compare">Comparison functor type.  The signature of the comparator must be of form
    /// bool cmp(const T& lhs, const T& rhs).  The function returns true if the left hand argument is less
    /// than the right hand argument.</typeparam>
    /// <param name="lhs">Left hand argument</param>
    /// <param name="rhs">Right hand argument</param>
    /// <param name="comp">Comparator</param>
    /// <returns>Minimum argument</returns>
    /// \ingroup Algorithms
    template <typename T, typename Compare>
    inline constexpr const T& min(const T& lhs, const T& rhs, Compare comp)
    {
        return (comp(lhs, rhs)) ? lhs : rhs;
    }

    /// <summary>
    /// Returns the maximum of two values.  Evaluated as lhs > rhs.
    /// </summary>
    /// <typeparam name="T">Type of values to get maximum of. Type must be comparable.</typeparam>
    /// <param name="lhs">Left hand argument</param>
    /// <param name="rhs">Right hand argument</param>
    /// <returns>Maximum argument</returns>
    /// \ingroup Algorithms
    template <typename T>
    inline constexpr const T& max(const T& lhs, const T& rhs)
    {
        return (lhs > rhs) ? lhs : rhs;
    }

    /// <summary>
    /// Returns the maximum of two values.  Evaluated as lhs > rhs.
    /// </summary>
    /// <typeparam name="T">Type of values to get maximum of.</typeparam>
    /// <typeparam name="Compare">Comparison functor type.  The signature of the comparator must be of form
    /// bool cmp(const T& lhs, const T& rhs).  The function returns true if the left hand argument is greater
    /// than the right hand argument.</typeparam>
    /// <param name="lhs">Left hand argument</param>
    /// <param name="rhs">Right hand argument</param>
    /// <param name="comp">Comparator</param>
    /// <returns>Maximum argument</returns>
    /// \ingroup Algorithms
    template <typename T, typename Compare>
    inline constexpr const T& max(const T& lhs, const T& rhs, Compare comp)
    {
        return (comp(lhs, rhs)) ? lhs : rhs;
    }
}

#endif // algorithm_hpp__
