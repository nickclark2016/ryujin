#ifndef asset_hpp__
#define asset_hpp__

#include "vector.hpp"

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
