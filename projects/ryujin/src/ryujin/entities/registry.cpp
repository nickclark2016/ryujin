#include <ryujin/entities/registry.hpp>

namespace ryujin
{
    namespace detail
    {
        std::size_t component_identifier_utility::id = 0;
    }

    template class base_registry<std::conditional_t<sizeof(size_t) == 8, entity<std::uint64_t>, entity<std::uint32_t>>>;
}