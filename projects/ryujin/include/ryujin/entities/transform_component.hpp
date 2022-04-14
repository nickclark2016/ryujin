#ifndef transformation_component_hpp__
#define transformation_component_hpp__

#include "../math/quat.hpp"
#include "../math/vec3.hpp"
#include "../math/mat4.hpp"

namespace ryujin
{
    struct transform_component
    {
        // TODO: figure out how tf to do properties across compilers
        mat4<float> matrix = mat4(1.0f);
        vec3<float> position = vec3(0.0f);
        quat<float> rotation = quat<float>();
        vec3<float> scale = vec3<float>(1.0f);
    };
} // namespace ryujin

#endif // transformation_component_hpp__
