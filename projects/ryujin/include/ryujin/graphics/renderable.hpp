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
        texture albedo;
        texture normal;
        texture metallic;
        texture roughness;
        texture ao;
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
        slot_map_key material;
        slot_map_key mesh;
    };

    class renderable_manager
    {
    public:
        explicit renderable_manager(render_manager* manager, registry* reg);
        slot_map_key load_texture(const std::string& name, const texture_asset& asset);
        std::optional<texture> try_fetch_texture(const std::string& name);
        std::optional<texture> try_fetch_texture(const slot_map_key& key);

        renderable_mesh load_mesh(const std::string& name, const mesh& m);
        void build_meshes();

    private:
        using entity_type = typename registry::entity_type;

        void register_entity(entity_type ent);
        void unregister_entity(entity_type ent);
        void update_entity(entity_type ent);
        
        registry* _registry;
        render_manager* _manager;

        std::unordered_map<std::string, slot_map_key> _textureNameLut;
        slot_map<texture> _textures;

        struct buffer_group
        {
            buffer positions;
            buffer interleaved;
            buffer indices;
        };

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

        image_sampler _defaultSampler = {};

        // mesh -> vector of entities with that mesh
        std::unordered_map<slot_map_key, vector<entity_type>, slot_map_key_hash> _entities;
    };
}

#endif // renderable_hpp__
