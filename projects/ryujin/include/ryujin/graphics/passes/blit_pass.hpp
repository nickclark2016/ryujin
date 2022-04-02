#ifndef blit_pass_hpp__
#define blit_pass_hpp__

#include "../types.hpp"

namespace ryujin
{
    class render_manager;
    class graphics_command_list;

    class blit_pass
    {
    public:
        blit_pass(render_manager& manager);

        void set_input_texture(image_view view);
        void set_output_texture(image_view view, const std::uint32_t width, const std::uint32_t height);

        void record(graphics_command_list& commands);

    private:
        void build_render_pass();
        void build_pipeline_layout();
        void build_pipeline();
        void build_input_sampler();

        void rebuild_target(const std::uint32_t width, const std::uint32_t height, const bool force = false);

        std::uint32_t _targetWidth = 0, _targetHeight = 0;
        frame_buffer _target = {};

        render_manager& _manager;
        render_pass _pass = {};
        pipeline _shader = {};
        pipeline_layout _layout = {};
        image_sampler _inputSampler = {};
        image_view _inputImageView = {};
        image_view _imageTarget = {};

        descriptor_set_layout _imageInputSetLayout = {};

        descriptor_set _sets[2] = {};
        std::uint32_t _frame = 0;
    };
}

#endif // blit_pass_hpp__