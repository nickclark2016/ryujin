#ifndef simple_render_pipeline_hpp__
#define simple_render_pipeline_hpp__

#include "base_render_pipeline.hpp"

#include "../passes/blit_pass.hpp"
#include "../passes/triangle_pass.hpp"
#include "../types.hpp"

#include <memory>

namespace ryujin::detail
{
    class simple_render_pipeline final : public base_render_pipeline
    {
    public:
        void pre_render() override;
        void render() override;
    protected:
        void initialize() override;
    private:
        void init_simple_pass();
        void init_render_target();
        void init_graphics_pipeline();

        frame_buffer _renderTarget{};
        image _target;
        image_view _targetView;
        data_format _targetFormat = data_format::R8G8B8A8_SRGB;

        render_pass _simplePass{};

        shader_module _simpleVertexShader;
        shader_module _simpleFragmentShader;

        std::unique_ptr<blit_pass> _blit;
        std::unique_ptr<triangle_pass> _triangle;
    };
}

#endif // simple_render_pipeline_hpp__
