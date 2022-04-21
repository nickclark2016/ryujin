#ifndef lighting_components_hpp__
#define lighting_components_hpp__

#include "../core/slot_map.hpp"
#include "../math/vec3.hpp"

namespace ryujin
{
    struct directional_light_component
    {
        slot_map_key shadowTarget = invalid_slot_map_key;
        vec3<float> color;
        float intensity;
    };
}

#endif // lighting_components_hpp__
