#ifndef algorithm_hpp__
#define algorithm_hpp__

#include "utility.hpp"

namespace ryujin
{
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
