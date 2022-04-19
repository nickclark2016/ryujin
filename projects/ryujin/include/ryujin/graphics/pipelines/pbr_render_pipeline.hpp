#ifndef simple_render_pipeline_hpp__
#define simple_render_pipeline_hpp__

#include "base_render_pipeline.hpp"

#include "../../core/primitives.hpp"
#include "../../core/vector.hpp"
#include "../../entities/registry.hpp"
#include "../../math/mat4.hpp"
#include "../../math/vec3.hpp"
#include "../passes/blit_pass.hpp"
#include "../passes/naive_translucent_pbr_pass.hpp"
#include "../passes/opaque_pbr_pass.hpp"
#include "../render_manager.hpp"
#include "../types.hpp"

#include <memory>

namespace ryujin
{
    class pbr_render_pipeline final : public base_render_pipeline
    {
        static constexpr data_format _colorAttachmentFmt = data_format::R8G8B8A8_SRGB;
        static constexpr data_format _depthAttachmentFmt = data_format::D32_SFLOAT;
        static constexpr u32 _targetWidth = 1920;
        static constexpr u32 _targetHeight = 1080;
        static constexpr u32 _maxInstances = 1024 * 512;
        static constexpr u32 _maxMaterials = 1024 * 64;
        static constexpr u32 _maxDrawCalls = 512;
        static constexpr u32 _maxTextures = 256;
        static constexpr u32 _maxPointLightCount = 512;
        static constexpr u32 _maxCameras = 32;

    public:
        void pre_render() override;
        void render() override;
    protected:
        void initialize() override;
        base_render_pipeline::render_target build_render_target(const std::string& name, const base_render_pipeline::render_target_info& info) override;
    private:
        struct alignas(256) scene_camera
        {
            mat4<float> view;
            mat4<float> proj;
            mat4<float> viewProj; // proj * view
            vec3<float> position;
            vec3<float> orientation;
        };

        struct point_light
        {
            vec3<float> position;
            vec3<float> color;
        };

        struct scene_lighting
        {
            point_light lights[_maxPointLightCount];
            u32 pointLightCount = 0;
        };

        struct alignas(256) scene_data
        {
            scene_lighting lighting = {};
            u32 texturesLoaded = 0;
        };

        void init_scene_pass();
        void init_scene_render_target();
        void init_scene_layout();
        void initialize_buffers();
        void initialize_textures();

        descriptor_set_layout _sceneWideLayout = {};
        descriptor_set_layout _drawableLayout = {};
        pipeline_layout _sceneLayout = {};
        render_pass _scenePass = {};

        std::unique_ptr<blit_pass> _blit;
        std::unique_ptr<opaque_pbr_pass> _opaque;
        std::unique_ptr<naive_translucent_pbr_pass> _naiveTranslucent;

        buffer _indirectCommands = {};
        buffer _translucentIndirectCommands = {};
        buffer _indirectCount = {};
        buffer _translucentIndirectCount = {};
        buffer _materials = {};
        buffer _instanceData = {};
        buffer _cameraData = {};
        buffer _sceneData = {};
        vector<texture> _textures;
        texture _invalidTexture = {};

        vector<descriptor_image_info> _textureWriteScratchBuffer;
        vector<entity_handle<registry::entity_type>> _activeCams;

        renderable_manager::draw_call_write_info _numBufferGroupsToDraw = {};
        renderable_manager::draw_call_write_info _numTranslucentBufferGroupsToDraw = {};
        renderable_manager::instance_write_info _instanceCounts;

        scene_data _hostSceneData = {};
    };
}

#endif // pbr_render_pipeline__