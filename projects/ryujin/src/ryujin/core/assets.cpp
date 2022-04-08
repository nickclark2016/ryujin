#include <ryujin/core/assets.hpp>

#include <ryujin/core/as.hpp>

#undef APIENTRY
#include <spdlog/spdlog.h>

#include "assets/png.hpp"

namespace ryujin
{   
    std::uint32_t texture_asset::width() const noexcept
    {
        return get_mip_level()->width;
    }

    std::uint32_t texture_asset::height() const noexcept
    {
        return get_mip_level()->height;
    }

    std::uint32_t texture_asset::mip_count() const noexcept
    {
        return as<std::uint32_t>(_mips.size());
    }

    std::uint32_t texture_asset::channel_count() const noexcept
    {
        return _channels;
    }

    const texture_asset::mip_level* texture_asset::get_mip_level(const std::uint32_t mip) const noexcept
    {
        if (mip < _mips.size())
        {
            auto mips = _mips.data();
            return mips + mip;
        }
        return nullptr;
    }

    texture_asset::data_type texture_asset::type() const noexcept
    {
        return _type;
    }

    texture_asset::channel_swizzle texture_asset::swizzle() const noexcept
    {
        return _swizzle;
    }

    texture_asset::texture_asset(const vector<mip_level>& mips, const std::uint32_t channels, const data_type type, const channel_swizzle swizzle)
        : _mips(mips), _channels(channels), _type(type), _swizzle(swizzle)
    {
    }

    const texture_asset* asset_manager::load_texture(const std::filesystem::path& path, const bool reload)
    {
        const auto key = path.string();
        if (_textures.find(key) != _textures.end() && !reload)
        {
            spdlog::info("Asset cache hit for {}", key);
            return _textures[key].get();
        }

        if (path.has_extension() && path.extension() == ".png")
        {
            spdlog::trace("Loading texture asset {}", key);
            texture_asset* asset = assets::load_png(key);
            if (asset == nullptr)
            {
                spdlog::error("Failed to load {}.", key);
                return nullptr;
            }

            spdlog::info("Successfully loaded {} as texture asset. Inserting into texture asset cache.", key);
            _textures[key] = std::unique_ptr<texture_asset>(asset);

            return asset;
        }
        else if (path.has_extension())
        {
            spdlog::warn("No texture loader for {} files.", path.extension().string());
        }
        return nullptr;
    }

    const std::unordered_map<std::string, std::unique_ptr<texture_asset>>& asset_manager::textures() const noexcept
    {
        return _textures;
    }
}