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

        vec3<float> offset = vec3(0.0f); // do not manually manipulate
        vec3<float> deformScale = vec3(0.0f); // do not manually manipulate

        transform_component() = default;
        transform_component(const transform_component&) = default;
        transform_component(transform_component&&) noexcept = default;
        transform_component& operator=(const transform_component&) = default;
        transform_component& operator=(transform_component&&) noexcept = default;
    };

    inline void set_position(transform_component& tx, const vec3<float>& position)
    {
        // probably shouldn't do this, but full sending
        tx.position = position;
        tx.matrix = transform(tx.position + tx.offset, tx.rotation, tx.scale * tx.deformScale);
    }

    inline void set_rotation(transform_component& tx, const vec3<float>& rotation)
    {
        // probably shouldn't do this, but full sending
        tx.rotation = quat(rotation);
        tx.matrix = transform(tx.position + tx.offset, rotation, tx.scale * tx.deformScale);
    }

    inline void set_rotation(transform_component& tx, const quat<float>& rotation)
    {
        // probably shouldn't do this, but full sending
        tx.rotation = rotation;
        (tx.matrix) = transform(tx.position + tx.offset, tx.rotation, tx.scale * tx.deformScale);
    }

    inline void set_scale(transform_component& tx, const vec3<float>& scale)
    {
        // probably shouldn't do this, but full sending
        tx.scale = scale;
        tx.matrix = transform(tx.position + tx.offset, tx.rotation, tx.scale * tx.deformScale);
    }

    inline void set_transform(transform_component& tx, const vec3<float>& position, const quat<float>& rotation, const vec3<float>& scale)
    {
        // probably shouldn't do this, but full sending
        tx.position = position;
        tx.rotation = rotation;
        tx.scale = scale;
        tx.matrix = transform(tx.position + tx.offset, tx.rotation, tx.scale * tx.deformScale);
    }

    inline void set_transform(transform_component& tx, const vec3<float>& position, const vec3<float>& rotation, const vec3<float>& scale)
    {
        // probably shouldn't do this, but full sending
        tx.position = position;
        tx.rotation = quat(rotation);
        tx.scale = scale;
        tx.matrix = transform(tx.position + tx.offset, tx.rotation, tx.scale * tx.deformScale);
    }

    inline void apply_parent_transform(transform_component& tx, const transform_component& parent)
    {
        const auto merged = parent.matrix * tx.matrix;
        decompose(merged, tx.position, tx.rotation, tx.scale);
    }
} // namespace ryujin

#endif // transformation_component_hpp__
