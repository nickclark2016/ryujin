#ifndef entity_relationship_component_hpp__
#define entity_relationship_component_hpp__

#include "entity.hpp"

namespace ryujin
{
    template <typename EntityType>
    struct entity_relationship_component
    {
        static constexpr EntityType tombstone = entity_traits<EntityType::type>::from_type(~EntityType::type(0));

        EntityType parent;
        EntityType firstChild;
        EntityType nextSibling;
    };
}

#endif // entity_relationship_component_hpp__
