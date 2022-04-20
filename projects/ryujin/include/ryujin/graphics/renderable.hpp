#ifndef renderable_hpp__
#define renderable_hpp__

#include "types.hpp"

#include "../core/assets.hpp"
#include "../core/primitives.hpp"
#include "../core/slot_map.hpp"
#include "../core/vector.hpp"
#include "../entities/registry.hpp"

#include <map>
#include <memory>
#include <optional>
#include <unordered_map>

namespace ryujin
{
    class render_manager;

    enum class material_type
    {
        OPAQUE,
        TRANSLUCENT
    };

    struct material
    {
        slot_map_key albedo;
        slot_map_key normal;
        slot_map_key metallicRoughness;
        slot_map_key emissive;
        slot_map_key ao;
        material_type type;
    };

    struct renderable_mesh
    {
        u32 bufferGroupId;
        u32 vertexOffset;
        u32 indexOffset;
        u32 indexCount;
    };

    struct renderable_component
    {
        slot_map_key material = slot_map<ryujin::material>::invalid;
        slot_map_key mesh = slot_map<ryujin::renderable_mesh>::invalid;
    };

    struct gpu_instance_data
    {
        mat4<float> transform;
        u32 material;
        u32 parent;
        u32 pad0, pad1;
    };

    struct gpu_material_data
    {
        u32 albedo;
        u32 normal;
        u32 metallicRoughness;
        u32 emissive;
        u32 ambientOcclusion;
    };

    struct gpu_indirect_call
    {
        u32 indexCount;
        u32 instanceCount;
        u32 firstIndex;
        i32 vertexOffset;
        u32 firstInstance;
    };

    class renderable_manager
    {
    public:
        using entity_type = typename registry::entity_type;

        struct buffer_group
        {
            buffer positions;
            buffer interleaved;
            buffer indices;
        };

        struct instance_write_info
        {
            sz opaqueCount;
            sz translucentCount;
        };

        struct draw_call_write_info
        {
            sz meshGroupCount;
            sz drawCallCount;
        };

        explicit renderable_manager(render_manager* manager, registry* reg);
        slot_map_key load_texture(const std::string& name, const texture_asset& asset);
        slot_map_key load_texture(const std::string& name, const image img, const image_view view);
        std::optional<texture> try_fetch_texture(const std::string& name);
        std::optional<texture> try_fetch_texture(const slot_map_key& key);
        void unload_texture(const slot_map_key& key);

        slot_map_key load_material(const std::string& name, const material_asset& asset);

        slot_map_key load_mesh(const std::string& name, const mesh& m);
        void build_meshes();

        instance_write_info write_instances(buffer& buf, const sz offset);
        sz write_materials(buffer& buf, const sz offset);
        draw_call_write_info write_draw_calls(buffer& indirectBuffer, buffer& drawCountBuffer, const sz offset, const material_type type);
        sz write_textures(texture* buf, sz offset);

        const buffer_group& get_buffer_group(const sz idx) const noexcept;

        entity_handle<registry::entity_type> main_camera() noexcept;

        void get_active_cameras(vector<entity_handle<entity_type>>& entities);

        // TODO: Reference count textures used in materials, unload texture on reference count decrement to zero

    private:
        void register_entity(entity_type ent);
        void unregister_entity(entity_type ent);
        void update_entity(entity_type ent);

        void register_camera(entity_type ent);
        void unregister_camera(entity_type ent);
        
        registry* _registry;
        render_manager* _manager;

        std::unordered_map<std::string, slot_map_key> _textureNameLut;
        slot_map<texture> _textures;

        struct mesh_group
        {
            struct position_t { u16 x, y, z; };
            struct encoded_tbn_t { u16 x, y, z, w; };
            struct texcoord_t { u16 u, v; };
            struct interleaved_t
            {
                encoded_tbn_t tbn;
                texcoord_t texcoord0;
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

        // mesh group id -> collection of meshes -> vector of entities with that mesh
        vector<std::unordered_map<u32, std::unordered_map<slot_map_key, vector<entity_type>, slot_map_key_hash>>> _entities;
        bool _entitiesDirty;

        vector<vector<gpu_indirect_call>> _drawCallCache;
        vector<vector<u32>> _drawCountCache;
        vector<draw_call_write_info> _groupsWrittenCount;
        std::map<u32, vector<entity_type>> _cameras;
    };
}

#endif // renderable_hpp__
