#ifndef png_hpp__
#define png_hpp__

#include <ryujin/core/assets.hpp>
#include <ryujin/core/span.hpp>

namespace ryujin::assets
{
    texture_asset* load_png(span<std::byte> buffer);
    texture_asset* load_png(const std::string& path);
}

#endif // png_hpp__
