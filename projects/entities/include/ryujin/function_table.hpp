#ifndef function_table_hpp__
#define function_table_hpp__

#include <functional>
#include <new>

namespace ryujin
{
    struct function_table
    {
        void (*default_ctor)(void*);
        void (*dtor)(void*);
    };

    template <typename T>
    constexpr function_table construct_fn_table()
    {
        function_table table;
        table.default_ctor = [](void* ptr) { ::new(ptr) T(); };
        table.dtor = [](void* ptr) { reinterpret_cast<T*>(ptr)->~T(); };
        return table;
    }
} // namespace entities

#endif // function_table_hpp__
