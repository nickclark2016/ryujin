#ifndef renderable_hpp__
#define renderable_hpp__

#include "types.hpp"

#include "../core/assets.hpp"
#include "../core/slot_map.hpp"
#include "../core/vector.hpp"
#include "../entities/registry.hpp"

#include <memory>
#include <optional>

namespace ryujin
{
    class render_manager;

    struct material
    {
        slot_map_key albedo;
        slot_map_key normal;
        slot_map_key metallicRoughness;
        slot_map_key emissive;
        slot_map_key ao;
    };

    struct renderable_mesh
    {
        std::uint32_t bufferGroupId;
        std::uint32_t vertexOffset;
        std::uint32_t indexOffset;
        std::uint32_t indexCount;
    };

    struct renderable_component
    {
        slot_map_key material = slot_map<ryujin::material>::invalid;
        slot_map_key mesh = slot_map<ryujin::renderable_mesh>::invalid;
    };

    struct gpu_instance_data
    {
        mat4<float> transform;
        std::uint32_t material;
        std::uint32_t parent;
        std::uint32_t pad0, pad1;
    };

    struct gpu_material_data
    {
        std::uint32_t albedo;
        std::uint32_t normal;
        std::uint32_t metallicRoughness;
        std::uint32_t emissive;
        std::uint32_t ambientOcclusion;
    };

    struct gpu_indirect_call
    {
        std::uint32_t indexCount;
        std::uint32_t instanceCount;
        std::uint32_t firstIndex;
        std::int32_t vertexOffset;
        std::uint32_t firstInstance;
    };

    class renderable_manager
    {
    public:
        struct buffer_group
        {
            buffer positions;
            buffer interleaved;
            buffer indices;
        };

        explicit renderable_manager(render_manager* manager, registry* reg);
        slot_map_key load_texture(const std::string& name, const texture_asset& asset);
        std::optional<texture> try_fetch_texture(const std::string& name);
        std::optional<texture> try_fetch_texture(const slot_map_key& key);

        slot_map_key load_material(const std::string& name, const material_asset& asset);

        slot_map_key load_mesh(const std::string& name, const mesh& m);
        void build_meshes();

        std::size_t write_instances(buffer& buf, const std::size_t offset);
        std::size_t write_materials(buffer& buf, const std::size_t offset);
        std::size_t write_draw_calls(buffer& indirectBuffer, buffer& drawCountBuffer, const std::size_t offset);
        std::size_t write_textures(texture* buf, std::size_t offset);

        const buffer_group& get_buffer_group(const std::size_t idx) const noexcept;

    private:
        using entity_type = typename registry::entity_type;

        void register_entity(entity_type ent);
        void unregister_entity(entity_type ent);
        void update_entity(entity_type ent);
        
        registry* _registry;
        render_manager* _manager;

        std::unordered_map<std::string, slot_map_key> _textureNameLut;
        slot_map<texture> _textures;

        struct mesh_group
        {
            struct position_t { float x, y, z; };
            struct texcoord_t { float u, v; };
            struct normal_t { float x, y, z; };
            struct tangent_t { float x, y, z, w; };
            struct interleaved_t
            {
                texcoord_t texcoord0;
                normal_t normal;
                tangent_t tangent;
            };

            vector<position_t> positions;
            vector<interleaved_t> interleavedValues;
            vector<uint32_t> indices;

            void clear();
        };

        vector<buffer_group> _bakedBufferGroups;
        mesh_group _activeMeshGroup;

        slot_map<renderable_mesh> _meshes;
        slot_map<material> _materials;

        image_sampler _defaultSampler = {};

        // mesh -> vector of entities with that mesh
        std::unordered_map<std::uint32_t, std::unordered_map<slot_map_key, vector<entity_type>, slot_map_key_hash>> _entities;
    };
}

#endif // renderable_hpp__
