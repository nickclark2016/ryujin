#include "image.hpp"

#include <ryujin/primitives.hpp>

#include <stb/stb_image.h>

#undef APIENTRY
#include <spdlog/spdlog.h>

#include <cstdint>

namespace ryujin::assets
{
    static constexpr auto HDR_IMAGE_EXT = ".hdr";

    texture_asset* load_image(const string& path, const string& ext)
    {
        const char* cpath = path.c_str();
        int x, y, channels;
        const auto infoResult = stbi_info(cpath, &x, &y, &channels);
        if (infoResult == 0)
        {
            spdlog::error("Failed to load image {} - {}", path.c_str(), stbi_failure_reason());
            return nullptr;
        }

        texture_asset::mip_level level = {
            .width = as<u32>(x),
            .height = as<u32>(y),
        };

        texture_asset::channel_swizzle swizzle {
            .first = texture_asset::channel::RED,
            .second = texture_asset::channel::GREEN,
            .third = texture_asset::channel::BLUE
        };

        texture_asset::data_type type;

        if (ext == HDR_IMAGE_EXT)
        {
            type = texture_asset::data_type::FLOAT;
            swizzle.fourth = texture_asset::channel::EXPONENT;
            float* data = stbi_loadf(cpath, &x, &y, &channels, 4);
            const sz sz = x * y * sizeof(float) * 4;
            level.bytes.resize(sz);
            ryujin::memcpy(level.bytes.data(), data, sz);
            stbi_image_free(data);
        }
        else
        {
            swizzle.fourth = texture_asset::channel::ALPHA;
            const auto is16bit = stbi_is_16_bit(cpath);
            if (is16bit)
            {
                type = texture_asset::data_type::USHORT;
                stbi_us* data = stbi_load_16(cpath, &x, &y, &channels, 4);
                const sz sz = x * y * sizeof(stbi_us) * 4;
                level.bytes.resize(sz);
                ryujin::memcpy(level.bytes.data(), data, sz);
                stbi_image_free(data);
            }
            else
            {
                type = texture_asset::data_type::UCHAR;
                stbi_uc* data = stbi_load(cpath, &x, &y, &channels, 4);
                const sz sz = x * y * sizeof(stbi_uc) * 4;
                level.bytes.resize(sz);
                ryujin::memcpy(level.bytes.data(), data, sz);
                stbi_image_free(data);
            }
        }

        vector<texture_asset::mip_level> mips;
        mips.push_back(level);
        texture_asset* asset = new texture_asset(mips, 4, type, swizzle);

        return asset;
    }
}
