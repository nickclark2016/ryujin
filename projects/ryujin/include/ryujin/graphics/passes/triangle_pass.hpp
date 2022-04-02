#ifndef triangle_pass_hpp__
#define triangle_pass_hpp__

#include "../types.hpp"

namespace ryujin
{
    class render_manager;
    class graphics_command_list;

    class triangle_pass
    {
    public:
        triangle_pass(render_manager& manager, const render_pass& pass);

        void record(graphics_command_list& commands);

    private:
        void build_pipeline_layout();
        void build_pipeline(const render_pass& pass);

        std::uint32_t _targetWidth = 0, _targetHeight = 0;
        frame_buffer _target = {};

        render_manager& _manager;
        pipeline _shader = {};
        pipeline_layout _layout = {};
    };
}

#endif // triangle_pass_hpp__
