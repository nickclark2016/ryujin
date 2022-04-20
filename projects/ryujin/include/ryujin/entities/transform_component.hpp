#ifndef transformation_component_hpp__
#define transformation_component_hpp__

#include "../math/quat.hpp"
#include "../math/vec3.hpp"
#include "../math/mat4.hpp"
#include "../math/transformations.hpp"

namespace ryujin
{
    struct transform_component
    {
        const mat4<float> matrix = mat4(1.0f);
        const vec3<float> position = vec3(0.0f);
        const quat<float> rotation = quat<float>();
        const vec3<float> scale = vec3<float>(1.0f);

        transform_component() = default;
        transform_component(const transform_component&) = default;
        
        transform_component(transform_component&& comp) noexcept
            : matrix(std::move(const_cast<mat4<float>&>(comp.matrix))), position(std::move(const_cast<vec3<float>&>(comp.position))),
                rotation(std::move(const_cast<quat<float>&>(comp.rotation))), scale(std::move(const_cast<vec3<float>&>(comp.scale)))
        {
        }

        transform_component& operator=(const transform_component& rhs) noexcept
        {
            const_cast<mat4<float>&>(matrix) = rhs.matrix;
            const_cast<vec3<float>&>(position) = rhs.position;
            const_cast<quat<float>&>(rotation) = rhs.rotation;
            const_cast<vec3<float>&>(scale) = rhs.scale;
            return *this;
        }

        transform_component& operator=(transform_component&& rhs) noexcept
        {
            const_cast<mat4<float>&>(matrix) = std::move(const_cast<mat4<float>&>(rhs.matrix));
            const_cast<vec3<float>&>(position) = std::move(const_cast<vec3<float>&>(rhs.position));
            const_cast<quat<float>&>(rotation) = std::move(const_cast<quat<float>&>(rhs.rotation));
            const_cast<vec3<float>&>(scale) = std::move(const_cast<vec3<float>&>(rhs.scale));
            return *this;
        }
    };

    inline void set_position(transform_component& tx, const vec3<float>& position)
    {
        // probably shouldn't do this, but full sending
        const_cast<vec3<float>&>(tx.position) = position;
        const_cast<mat4<float>&>(tx.matrix) = transform(tx.position, tx.rotation, tx.scale);
    }

    inline void set_rotation(transform_component& tx, const vec3<float>& rotation)
    {
        // probably shouldn't do this, but full sending
        const_cast<quat<float>&>(tx.rotation) = quat(rotation);
        const_cast<mat4<float>&>(tx.matrix) = transform(tx.position, tx.rotation, tx.scale);
    }

    inline void set_rotation(transform_component& tx, const quat<float>& rotation)
    {
        // probably shouldn't do this, but full sending
        const_cast<quat<float>&>(tx.rotation) = rotation;
        const_cast<mat4<float>&>(tx.matrix) = transform(tx.position, tx.rotation, tx.scale);
    }

    inline void set_scale(transform_component& tx, const vec3<float>& scale)
    {
        // probably shouldn't do this, but full sending
        const_cast<vec3<float>&>(tx.scale) = scale;
        const_cast<mat4<float>&>(tx.matrix) = transform(tx.position, tx.rotation, tx.scale);
    }

    inline void set_transform(transform_component& tx, const vec3<float>& position, const quat<float>& rotation, const vec3<float>& scale)
    {
        // probably shouldn't do this, but full sending
        const_cast<vec3<float>&>(tx.position) = position;
        const_cast<quat<float>&>(tx.rotation) = rotation;
        const_cast<vec3<float>&>(tx.scale) = scale;
        const_cast<mat4<float>&>(tx.matrix) = transform(tx.position, tx.rotation, tx.scale);
    }

    inline void set_transform(transform_component& tx, const vec3<float>& position, const vec3<float>& rotation, const vec3<float>& scale)
    {
        // probably shouldn't do this, but full sending
        const_cast<vec3<float>&>(tx.position) = position;
        const_cast<quat<float>&>(tx.rotation) = quat(rotation);
        const_cast<vec3<float>&>(tx.scale) = scale;
        const_cast<mat4<float>&>(tx.matrix) = transform(tx.position, tx.rotation, tx.scale);
    }
} // namespace ryujin

#endif // transformation_component_hpp__
