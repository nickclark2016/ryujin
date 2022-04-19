#ifndef naive_translucent_pbr_pass__
#define naive_translucent_pbr_pass__

#include "../types.hpp"

namespace ryujin
{
    class render_manager;
    class graphics_command_list;

    class naive_translucent_pbr_pass
    {
    public:
        naive_translucent_pbr_pass(render_manager& manager, pipeline_layout layout, render_pass pass, std::uint32_t width, std::uint32_t height);

        void render(graphics_command_list& cmd, buffer& indirect, buffer& count, std::size_t indirectOffset, std::size_t countOffset, std::size_t numBufferGroups);
    private:
        void init_graphics_pipeline(std::uint32_t width, std::uint32_t height, render_pass pass);

        render_manager& _manager;
        pipeline_layout _layout = {};
        pipeline _shader = {};
    };
}

#endif // naive_translucent_pbr_pass__
