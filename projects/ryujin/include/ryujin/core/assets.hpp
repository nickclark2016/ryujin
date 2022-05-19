#ifndef asset_hpp__
#define asset_hpp__

#include "export.hpp"
#include "primitives.hpp"
#include "slot_map.hpp"
#include "smart_pointers.hpp"
#include "string.hpp"
#include "unordered_map.hpp"
#include "vector.hpp"

#include "../entities/transform_component.hpp"
#include "../math/mat4.hpp"
#include "../math/quat.hpp"
#include "../math/vec2.hpp"
#include "../math/vec3.hpp"

#include <cstddef>
#include <filesystem>

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

        RYUJIN_API u32 width() const noexcept;
        RYUJIN_API u32 height() const noexcept;
        RYUJIN_API u32 mip_count() const noexcept;
        RYUJIN_API u32 channel_count() const noexcept;
        RYUJIN_API data_type type() const noexcept;
        RYUJIN_API channel_swizzle swizzle() const noexcept;
        RYUJIN_API const mip_level* get_mip_level(const u32 mip = 0) const noexcept;

        RYUJIN_API texture_asset(const vector<mip_level>& mips, const u32 channels, const data_type type, const channel_swizzle swizzle);

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
        vec4<float> tangent;
        vec2<float> texCoord;
    };

    enum class alpha_mode
    {
        OPAQUE,
        BLENDED
    };

    struct material_asset
    {
        string name;
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
        string name;
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
        RYUJIN_API slot_map_key get_mesh_group() const;
        RYUJIN_API const string& name() const;

        RYUJIN_API transform_component transform() const noexcept;
        RYUJIN_API void transform(const transform_component& tx) noexcept;

        RYUJIN_API void add_child(model_asset& child);
        RYUJIN_API const vector<model_asset*> children() const noexcept;

        RYUJIN_API model_asset(const string& name, const slot_map_key mesh, mat4<float> transform, vec3<float> translate, quat<float> rotate, vec3<float> scale);
    private:
        string _name;
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
        RYUJIN_API const texture_asset* load_texture(const std::filesystem::path& path, const bool reload = false);
        RYUJIN_API const model_asset* load_model(const std::filesystem::path& path, const bool reload = false);
        RYUJIN_API const material_asset* load_material(const string& name, material_asset material);

        RYUJIN_API const slot_map_key load_mesh_group(const mesh_group& group);
        RYUJIN_API [[nodiscard]] const mesh_group* get_mesh_group(const slot_map_key& key) const noexcept;

    private:
        unordered_map<string, unique_ptr<texture_asset>> _textures;
        unordered_map<string, unique_ptr<model_asset>> _models;
        unordered_map<string, unique_ptr<material_asset>> _materials;
        
        slot_map<mesh_group> _meshes;
    };
}

#endif // asset_hpp__
