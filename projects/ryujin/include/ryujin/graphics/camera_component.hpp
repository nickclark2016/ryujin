#ifndef camera_component_hpp__
#define camera_component_hpp__

#include "../core/slot_map.hpp"
#include "../math/vec3.hpp"

namespace ryujin
{
    struct camera_component
    {
        float near;
        float far;
        float verticalFov;
        slot_map_key target = invalid_slot_map_key;
        std::uint32_t order; // higher == later render
        bool active;
    };
}

#endif // camera_component_hpp__
