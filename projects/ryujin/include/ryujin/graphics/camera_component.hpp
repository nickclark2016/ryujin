#ifndef camera_component_hpp__
#define camera_component_hpp__

#include "../math/vec3.hpp"

namespace ryujin
{
    struct camera_component
    {
        vec3<float> position;
        vec3<float> direction;
        vec3<float> up;
    };
}

#endif // camera_component_hpp__
