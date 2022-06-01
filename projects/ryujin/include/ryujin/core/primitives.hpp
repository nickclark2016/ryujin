#ifndef primitives_hpp__
#define primitives_hpp__

namespace ryujin
{
    using u8 = unsigned char;
    using u16 = unsigned short;
    using u32 = unsigned int;
    using u64 = unsigned long long;
    using i8 = char;
    using i16 = short;
    using i32 = int;
    using i64 = long;
    using f32 = float;
    using f64 = double;
    using sz = decltype(sizeof(void*));
    using ptr_diff = decltype((char*)(0) - (char*)(0));

    enum class byte : unsigned char {};
}

#endif // primitives_hpp__
