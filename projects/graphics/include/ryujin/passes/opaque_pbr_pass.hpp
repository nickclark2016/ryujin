#ifndef opaque_pbr_pass__
#define opaque_pbr_pass__

#include "../types.hpp"

#include <ryujin/export.hpp>
#include <ryujin/primitives.hpp>

namespace ryujin
{
    class render_manager;
    class graphics_command_list;

    class opaque_pbr_pass
    {
    public:
        RYUJIN_API opaque_pbr_pass(render_manager& manager, pipeline_layout layout, render_pass pass, u32 width, u32 height);

        RYUJIN_API void render(graphics_command_list& cmd, buffer& indirect, buffer& count, sz indirectOffset, sz countOffset, sz numBufferGroups);
    private:
        void init_graphics_pipeline(u32 width, u32 height, render_pass pass);

        render_manager& _manager;
        pipeline_layout _layout = {};
        pipeline _shader = {};
    };
}

#endif // opaque_pbr_pass__
