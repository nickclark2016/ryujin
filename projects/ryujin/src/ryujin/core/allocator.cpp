#include <ryujin/core/allocator.hpp>

#include <new>

namespace ryujin::detail
{
    void* allocate_raw_bytes(const sz count, const sz bytesPerElem, const sz alignment)
    {
        return operator new(count * bytesPerElem, std::align_val_t(alignment));
    }

    void deallocate_raw_bytes(void* data, const sz alignment)
    {
        operator delete(data, std::align_val_t(alignment));
    }
}