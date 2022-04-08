#ifndef renderable_hpp__
#define renderable_hpp__

#include "types.hpp"

#include "../core/assets.hpp"
#include "../core/slot_map.hpp"
#include "../entities/entity.hpp"

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

    struct mesh
    {
        slot_map_key bufferGroupId;
        std::uint32_t vertexOffset;
        std::uint32_t indexOffset;
        std::uint32_t indexCount;
    };

    struct renderable
    {
        slot_map_key material;
        slot_map_key mesh;
    };

    class renderable_manager
    {
    public:
        explicit renderable_manager(render_manager* manager);
        slot_map_key load_texture(const std::string& name, const texture_asset& asset);
        std::optional<texture> try_fetch_texture(const std::string& name);
        std::optional<texture> try_fetch_texture(const slot_map_key& key);

    private:
        render_manager* _manager;

        std::unordered_map<std::string, slot_map_key> _textureNameLut;
        slot_map<texture> _textures;

        image_sampler _defaultSampler = {};
    };
}

#endif // renderable_hpp__
