#ifndef allocator_hpp__
#define allocator_hpp__

#include "export.hpp"
#include "primitives.hpp"

#include <new> // TODO: Figure out how tf to replace this

namespace ryujin
{
    namespace detail
    {
        RYUJIN_API void* allocate_raw_bytes(const sz count, const sz bytesPerElem, const sz alignment);
        RYUJIN_API void deallocate_raw_bytes(void* data, const sz alignment);
    }

    template <typename T>
    class allocator
    {
    public:
        using value_type = T;
        using pointer = T*;
        using const_pointer = const T*;
        using reference = T&;
        using const_reference = const T&;
        using size_type = sz;
        using difference_type = ptr_diff;

        RYUJIN_API constexpr pointer allocate(size_type n);
        RYUJIN_API constexpr void deallocate(pointer p, size_type n);
    };
    
    template<typename T>
    inline constexpr typename allocator<T>::pointer allocator<T>::allocate(size_type n)
    {
        void* data = detail::allocate_raw_bytes(n, sizeof(T), alignof(T));
        return reinterpret_cast<T*>(data);
    }
    
    template<typename T>
    inline constexpr void allocator<T>::deallocate(pointer p, size_type n)
    {
        detail::deallocate_raw_bytes(p, alignof(T));
    }
}

#endif // allocator_hpp__
