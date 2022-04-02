#ifndef as_hpp__
#define as_hpp__

namespace ryujin
{
    template <typename T, typename U>
    constexpr T as(U value)
    {
        return static_cast<T>(value);
    }
}

#endif // as_hpp__
