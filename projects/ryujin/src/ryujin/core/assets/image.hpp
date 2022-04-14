#ifndef asset_image_hpp__
#define asset_image_hpp__

#include <ryujin/core/assets.hpp>

namespace ryujin::assets
{
    texture_asset* load_image(const std::string& path, const std::string& ext);
}

#endif // asset_image_hpp__
