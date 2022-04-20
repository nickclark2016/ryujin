#ifndef asset_hpp__
#define asset_hpp__

#include "primitives.hpp"
#include "slot_map.hpp"
#include "vector.hpp"

#include "../entities/transform_component.hpp"
#include "../math/mat4.hpp"
#include "../math/quat.hpp"
#include "../math/vec2.hpp"
#include "../math/vec3.hpp"
#include "../math/vec4.hpp"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <unordered_map>

namespace ryujin
{
    class texture_asset
    {
    public:
        enum class data_type
        {
            UCHAR,
            USHORT,
            FLOAT
        };

        enum class channel
        {
            RED,
            GREEN,
            BLUE,
            ALPHA,
            EXPONENT,
            EMPTY
        };

        struct channel_swizzle
        {
            channel first;
            channel second;
            channel third;
            channel fourth;
        };

        struct mip_level
        {
            u32 width;
            u32 height;
            vector<std::byte> bytes;
        };

        u32 width() const noexcept;
        u32 height() const noexcept;
        u32 mip_count() const noexcept;
        u32 channel_count() const noexcept;
        data_type type() const noexcept;
        channel_swizzle swizzle() const noexcept;
        const mip_level* get_mip_level(const u32 mip = 0) const noexcept;

        texture_asset(const vector<mip_level>& mips, const u32 channels, const data_type type, const channel_swizzle swizzle);

    private:
        vector<mip_level> _mips;
        u32 _channels = 0;
        data_type _type = {};
        channel_swizzle _swizzle;
    };

    struct vertex
    {
        vec3<float> position;
        vec3<float> normal;
        vec3<float> tangent;
        vec2<float> texCoord;
    };

    enum class alpha_mode
    {
        OPAQUE,
        BLENDED
    };

    struct material_asset
    {
        std::string name;
        const texture_asset* baseColorTexture;
        const texture_asset* normalTexture;
        const texture_asset* occlusionTexture;
        const texture_asset* emissiveTexture;
        const texture_asset* metallicRoughness;
        alpha_mode alpha;
    };

    struct mesh
    {
        vector<vertex> vertices;
        vector<u32> indices;
        std::string name;
        const material_asset* material;
        vec3<float> position;
        vec3<float> scale;
    };

    struct mesh_group
    {
        u32 id;
        vector<mesh> meshes;
    };

    class model_asset
    {
    public:
        slot_map_key get_mesh_group() const;
        const std::string& name() const;

        transform_component transform() const noexcept;
        void transform(const transform_component& tx) noexcept;

        void add_child(model_asset& child);
        const vector<model_asset*> children() const noexcept;

        model_asset(const std::string& name, const slot_map_key mesh, mat4<float> transform, vec3<float> translate, quat<float> rotate, vec3<float> scale);
    private:
        std::string _name;
        slot_map_key _mesh = slot_map<mesh_group>::invalid;
        mat4<float> _transform;
        vec3<float> _translation;
        vec3<float> _scale;
        quat<float> _rotation;
        vector<model_asset*> _children;
        model_asset* _parent = nullptr;
    };

    class asset_manager
    {
    public:
        const texture_asset* load_texture(const std::filesystem::path& path, const bool reload = false);
        const model_asset* load_model(const std::filesystem::path& path, const bool reload = false);
        const material_asset* load_material(const std::string& name, material_asset material);

        const slot_map_key load_mesh_group(const mesh_group& group);
        const mesh_group* get_mesh_group(const slot_map_key& key) const noexcept;


    private:
        std::unordered_map<std::string, std::unique_ptr<texture_asset>> _textures;
        std::unordered_map<std::string, std::unique_ptr<model_asset>> _models;
        std::unordered_map<std::string, std::unique_ptr<material_asset>> _materials;
        
        slot_map<mesh_group> _meshes;
    };
}

#endif // asset_hpp__
