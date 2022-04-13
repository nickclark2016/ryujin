#ifndef asset_hpp__
#define asset_hpp__

#include "vector.hpp"

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
            std::uint32_t width;
            std::uint32_t height;
            vector<std::byte> bytes;
        };

        std::uint32_t width() const noexcept;
        std::uint32_t height() const noexcept;
        std::uint32_t mip_count() const noexcept;
        std::uint32_t channel_count() const noexcept;
        data_type type() const noexcept;
        channel_swizzle swizzle() const noexcept;
        const mip_level* get_mip_level(const std::uint32_t mip = 0) const noexcept;

        texture_asset(const vector<mip_level>& mips, const std::uint32_t channels, const data_type type, const channel_swizzle swizzle);

    private:
        vector<mip_level> _mips;
        std::uint32_t _channels = 0;
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

    struct mesh
    {
        vector<vertex> vertices;
        vector<std::uint32_t> indices;
    };

    enum class alpha_mode
    {
        OPAQUE
    };

    struct material_asset
    {
        std::string name;
        texture_asset* baseColorTexture;
        texture_asset* normalTexture;
        texture_asset* occlusionTexture;
        texture_asset* emissiveTexture;
        texture_asset* metallicRoughness;
        alpha_mode alpha;
    };

    class model_asset
    {
    public:
        const mesh& get_mesh() const;

    private:
        std::string _name;
        mesh _mesh;
        material_asset _material;
        mat4<float> _transform;
        vec3<float> _translation;
        vec3<float> _scale;
        quat<float> _rotation;
        vector<std::unique_ptr<model_asset>> _children;
    };

    class asset_manager
    {
    public:
        const texture_asset* load_texture(const std::filesystem::path& path, const bool reload = false);

        const std::unordered_map<std::string, std::unique_ptr<texture_asset>>& textures() const noexcept;

    private:
        std::unordered_map<std::string, std::unique_ptr<texture_asset>> _textures;
    };
}

#endif // asset_hpp__
