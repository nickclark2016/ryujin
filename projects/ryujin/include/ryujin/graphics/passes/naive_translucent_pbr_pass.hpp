#ifndef naive_translucent_pbr_pass__
#define naive_translucent_pbr_pass__

#include "../types.hpp"

#include "../../core/primitives.hpp"

namespace ryujin
{
    class render_manager;
    class graphics_command_list;

    class naive_translucent_pbr_pass
    {
    public:
        naive_translucent_pbr_pass(render_manager& manager, pipeline_layout layout, render_pass pass, u32 width, u32 height);

        void render(graphics_command_list& cmd, buffer& indirect, buffer& count, sz indirectOffset, sz countOffset, sz numBufferGroups);
    private:
        void init_graphics_pipeline(u32 width, u32 height, render_pass pass);

        render_manager& _manager;
        pipeline_layout _layout = {};
        pipeline _shader = {};
    };
}

#endif // naive_translucent_pbr_pass__
