#ifndef lighting_components_hpp__
#define lighting_components_hpp__

#include "../math/vec3.hpp"

namespace ryujin
{
    struct directional_light
    {
        bool castShadows;
        vec3<float> color;
        float intensity;
    };
}

#endif // lighting_components_hpp__
