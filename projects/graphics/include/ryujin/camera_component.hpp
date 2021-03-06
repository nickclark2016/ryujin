#ifndef camera_component_hpp__
#define camera_component_hpp__

#include <ryujin/primitives.hpp>
#include <ryujin/slot_map.hpp>
#include <ryujin/vec3.hpp>

namespace ryujin
{
    struct camera_component
    {
        float near;
        float far;
        float fov;
        slot_map_key target = invalid_slot_map_key;
        u32 order; // higher == later render
        bool active;
    };
}

#endif // camera_component_hpp__
