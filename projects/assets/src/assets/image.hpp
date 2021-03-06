#ifndef asset_image_hpp__
#define asset_image_hpp__

#include <ryujin/assets.hpp>
#include <ryujin/string.hpp>

namespace ryujin::assets
{
    texture_asset* load_image(const string& path, const string& ext);
}

#endif // asset_image_hpp__
