#include <ryujin/core/utility.hpp>

#include <cstring>

#if defined(RYUJIN_MEMSET_DEFINED)
void* ryujin::memset(void* dst, int val, sz len)
{
    return std::memset(dst, val, len);
}
#endif

#if defined(RYUJIN_MEMCPY_DEFINED)
void* ryujin::memcpy(void* dst, const void* src, sz len)
{
    return std::memcpy(dst, src, len);
}
#endif