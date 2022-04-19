#include <ryujin/core/assets.hpp>

#include <ryujin/core/as.hpp>
#include <ryujin/core/primitives.hpp>

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

    u32 texture_asset::width() const noexcept
    {
        return get_mip_level()->width;
    }

    u32 texture_asset::height() const noexcept
    {
        return get_mip_level()->height;
    }

    u32 texture_asset::mip_count() const noexcept
    {
        return as<u32>(_mips.size());
    }

    u32 texture_asset::channel_count() const noexcept
    {
        return _channels;
    }

    const texture_asset::mip_level* texture_asset::get_mip_level(const u32 mip) const noexcept
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

    texture_asset::texture_asset(const vector<mip_level>& mips, const u32 channels, const data_type type, const channel_swizzle swizzle)
        : _mips(mips), _channels(channels), _type(type), _swizzle(swizzle)
    {
    }

    slot_map_key model_asset::get_mesh_group() const
    {
        return _mesh;
    }

    const std::string& model_asset::name() const
    {
        return _name;
    }

    void model_asset::add_child(model_asset& child)
    {
        _children.push_back(&child);
        child._parent = this;
    }

    model_asset::model_asset(const std::string& name, const slot_map_key mesh, mat4<float> transform, vec3<float> translate, quat<float> rotate, vec3<float> scale)
        : _name(name), _mesh(mesh), _transform(transform), _translation(translate), _scale(scale), _rotation(rotate)
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
        auto res = assets[0].get();
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

        return res;
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
    
    mesh_group* asset_manager::get_mesh_group(const slot_map_key& key) noexcept
    {
        return _meshes.try_get(key);
    }
}