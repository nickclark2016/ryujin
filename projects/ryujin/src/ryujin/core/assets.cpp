#include <ryujin/core/assets.hpp>

#include <ryujin/core/as.hpp>

#undef APIENTRY
#include <spdlog/spdlog.h>

#include "assets/image.hpp"
#include "assets/model.hpp"

#include <cctype>
#include <string>

namespace ryujin
{
    static std::string lower_extension(const std::filesystem::path& path)
    {
        std::string extension = path.extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c){ return std::tolower(c); });
        return extension;
    } 

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

    const std::string& model_asset::name() const
    {
        return _name;
    }

    const texture_asset* asset_manager::load_texture(const std::filesystem::path& path, const bool reload)
    {
        const auto key = path.string();
        if (_textures.find(key) != _textures.end() && !reload)
        {
            spdlog::info("Asset cache hit for {}", key);
            return _textures[key].get();
        }

        spdlog::trace("Loading texture asset {}", key);
        texture_asset* asset = assets::load_image(key, lower_extension(path));
        if (asset == nullptr)
        {
            spdlog::error("Failed to load {}.", key);
            return nullptr;
        }

        spdlog::info("Successfully loaded {} as texture asset. Inserting into texture asset cache.", key);

        auto tries = 0;
        auto pathStem = path.stem().string();
        auto insertionKey = pathStem;
        while (_textures.contains(insertionKey))
        {
            insertionKey = fmt::v8::format("{}_{}", pathStem, tries);
            tries++;
        }

        _textures[insertionKey] = std::unique_ptr<texture_asset>(asset);

        return asset;
    }

    const model_asset* asset_manager::load_model(const std::filesystem::path& path, const bool reload)
    {
        const auto key = path.string();
        if (_textures.find(key) != _textures.end() && !reload)
        {
            spdlog::info("Asset cache hit for {}", key);
            return _models[key].get();
        }

        spdlog::trace("Loading model asset {}", key);
        auto assets = assets::load_model(key, lower_extension(path), this);
        if (assets.empty())
        {
            spdlog::error("Failed to load {}.", key);
            return nullptr;
        }

        spdlog::info("Successfully loaded {} as model asset. Inserting into model asset cache.", key);
        for (auto& asset : assets)
        {
            auto name = asset->name();

            auto tries = 0;
            auto pathStem = path.stem().string();
            const auto base = name.empty() ? pathStem : name;
            auto insertionKey = base;
            while (_models.contains(insertionKey))
            {
                insertionKey = fmt::v8::format("{}_{}", base, tries);
                tries++;
            }
            _models[insertionKey] = std::move(asset);
        }

        return assets[0].get();
    }

    const material_asset* asset_manager::load_material(const std::string& name, material_asset material)
    {
        auto tries = 0;
        const auto base = name;
        auto insertionKey = base;
        while (_materials.contains(insertionKey))
        {
            insertionKey = fmt::v8::format("{}_{}", base, tries);
            tries++;
        }
        material.name = insertionKey;
        _materials[insertionKey] = std::make_unique<material_asset>(std::move(material));
        return _materials[insertionKey].get();
    }

    const slot_map_key asset_manager::load_mesh_group(const mesh_group& group)
    {
        return _meshes.insert(group);
    }
}