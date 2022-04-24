#ifndef asset_model_hpp__
#define asset_model_hpp__

#include <ryujin/core/assets.hpp>

namespace ryujin
{
    class asset_manager;
}

namespace ryujin::assets
{
    vector<unique_ptr<model_asset>> load_model(const std::string& path, const std::string& ext, asset_manager* manager);
}

#endif // asset_model_hpp__
