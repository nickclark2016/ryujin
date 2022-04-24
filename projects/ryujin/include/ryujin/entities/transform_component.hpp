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
        mat4<float> matrix = mat4(1.0f);
        vec3<float> position = vec3(0.0f);
        quat<float> rotation = quat<float>();
        vec3<float> scale = vec3<float>(1.0f);

        transform_component() = default;
        transform_component(const transform_component&) = default;
        
        transform_component(transform_component&& comp) noexcept
            : matrix(std::move((comp.matrix))), position(std::move(const_cast<vec3<float>&>(comp.position))),
                rotation(std::move(const_cast<quat<float>&>(comp.rotation))), scale(std::move(const_cast<vec3<float>&>(comp.scale)))
        {
        }

        transform_component& operator=(const transform_component& rhs) noexcept
        {
            matrix = rhs.matrix;
            position = rhs.position;
            rotation = rhs.rotation;
            scale = rhs.scale;
            return *this;
        }

        transform_component& operator=(transform_component&& rhs) noexcept
        {
            matrix = std::move(const_cast<mat4<float>&&>(rhs.matrix));
            position = std::move(const_cast<vec3<float>&&>(rhs.position));
            rotation = std::move(const_cast<quat<float>&&>(rhs.rotation));
            scale = std::move(const_cast<vec3<float>&&>(rhs.scale));
            return *this;
        }
    };

    inline void set_position(transform_component& tx, const vec3<float>& position)
    {
        // probably shouldn't do this, but full sending
        tx.position = position;
        tx.matrix = transform(tx.position, tx.rotation, tx.scale);
    }

    inline void set_rotation(transform_component& tx, const vec3<float>& rotation)
    {
        // probably shouldn't do this, but full sending
        tx.rotation = quat(rotation);
        tx.matrix = transform(tx.position, rotation, tx.scale);
    }

    inline void set_rotation(transform_component& tx, const quat<float>& rotation)
    {
        // probably shouldn't do this, but full sending
        tx.rotation = rotation;
        (tx.matrix) = transform(tx.position, tx.rotation, tx.scale);
    }

    inline void set_scale(transform_component& tx, const vec3<float>& scale)
    {
        // probably shouldn't do this, but full sending
        tx.scale = scale;
        tx.matrix = transform(tx.position, tx.rotation, tx.scale);
    }

    inline void set_transform(transform_component& tx, const vec3<float>& position, const quat<float>& rotation, const vec3<float>& scale)
    {
        // probably shouldn't do this, but full sending
        tx.position = position;
        tx.rotation = rotation;
        tx.scale = scale;
        tx.matrix = transform(tx.position, tx.rotation, tx.scale);
    }

    inline void set_transform(transform_component& tx, const vec3<float>& position, const vec3<float>& rotation, const vec3<float>& scale)
    {
        // probably shouldn't do this, but full sending
        tx.position = position;
        tx.rotation = quat(rotation);
        tx.scale = scale;
        tx.matrix = transform(tx.position, tx.rotation, tx.scale);
    }

    inline void apply_parent_transform(transform_component& tx, const transform_component& parent)
    {
        const auto merged = parent.matrix * tx.matrix;
        decompose(merged, tx.position, tx.rotation, tx.scale);
    }
} // namespace ryujin

#endif // transformation_component_hpp__
