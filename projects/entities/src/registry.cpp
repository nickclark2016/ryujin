#include <ryujin/registry.hpp>

#include <ryujin/primitives.hpp>

namespace ryujin
{
    namespace detail
    {
        sz component_identifier_utility::id = 0;
    }

    template class base_registry<std::conditional_t<sizeof(void*) == 8, entity<u64>, entity<u32>>>;
}