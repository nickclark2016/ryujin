#include <ryujin/entities/sparse_set.hpp>

#include <ryujin/entities/entity.hpp>

namespace ryujin
{
    template class sparse_set<std::conditional_t<sizeof(size_t) == 8, entity<u64>, entity<u32>>, 512>;
    template class sparse_set<std::conditional_t<sizeof(size_t) == 8, entity<u64>, entity<u32>>, 1024>;
    template class sparse_set<std::conditional_t<sizeof(size_t) == 8, entity<u64>, entity<u32>>, 2048>;
}