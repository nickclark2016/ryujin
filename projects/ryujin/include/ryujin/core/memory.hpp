#ifndef memory_hpp__
#define memory_hpp__

#include "smart_pointers.hpp"
#include "utility.hpp"

#include <new>

namespace ryujin
{
    template <typename T, typename ... Args>
    inline constexpr T* construct_at(T* p, Args&& ... args) noexcept(noexcept(::new((void*)0) T(declval<Args>()...)))
    {
        return ::new((void*)p) T(ryujin::forward<Args>(args)...);
    }

    template <typename T>
    inline constexpr void destroy_at(T* p)
    {
        if constexpr (is_array_v<T>)
        {
            for (auto& x : *p)
            {
                destroy_at(&x);
            }
        }
        else
        {
            p->~T();
        }
    }
}

#endif // memory_hpp__
