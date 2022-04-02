#include <ryujin/entities/sparse_set.hpp>

#include <ryujin/entities/entity.hpp>

namespace ryujin
{
    template class sparse_set<std::conditional_t<sizeof(size_t) == 8, entity<std::uint64_t>, entity<std::uint32_t>>, 512>;
    template class sparse_set<std::conditional_t<sizeof(size_t) == 8, entity<std::uint64_t>, entity<std::uint32_t>>, 1024>;
    template class sparse_set<std::conditional_t<sizeof(size_t) == 8, entity<std::uint64_t>, entity<std::uint32_t>>, 2048>;
}