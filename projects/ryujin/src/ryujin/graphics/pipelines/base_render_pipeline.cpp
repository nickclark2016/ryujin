#include <ryujin/graphics/pipelines/base_render_pipeline.hpp>

#include <ryujin/graphics/render_manager.hpp>

#undef APIENTRY
#include <spdlog/spdlog.h>

namespace ryujin
{
    slot_map_key base_render_pipeline::add_render_target(const render_target_info& info)
    {
        auto tgt = build_render_target(info.name, info);

        tgt.colorTex = get_render_manager()->renderables().load_texture(fmt::v8::format("{}_color", info.name), tgt.color.img, tgt.color.view);
        tgt.depthTex = get_render_manager()->renderables().load_texture(fmt::v8::format("{}_depth", info.name), tgt.depth.img, tgt.depth.view);

        auto key = _targets.insert(tgt);
        _renderTargets[info.name] = key;

        return key;
    }

    void base_render_pipeline::remove_render_target(const slot_map_key& k)
    {
        auto tgt = _targets.try_get(k);
        if (tgt)
        {
            get_render_manager()->release(tgt->fbo);
            get_render_manager()->renderables().unload_texture(tgt->colorTex);
            get_render_manager()->renderables().unload_texture(tgt->depthTex);

            if (tgt->fbo == _activeRenderTarget.fbo)
            {
                _activeRenderTarget = _defaultRenderTarget;
            }
            _prevRenderTarget = _activeRenderTarget;

            _targets.erase(k);
        }
    }

    void base_render_pipeline::transition_render_targets(command_list& list)
    {
        if (_prevRenderTarget.fbo == nullptr)
        {
            return;
        }

        image_memory_barrier color = {
            .src = access_type::NONE,
            .dst = access_type::SHADER_READ,
            .oldLayout = image_layout::COLOR_ATTACHMENT_OPTIMAL,
            .newLayout = image_layout::SHADER_READ_ONLY_OPTIMAL,
            .srcQueue = list.queue_index(),
            .dstQueue = list.queue_index(),
            .img = _prevRenderTarget.color.img,
            .range = {
                .aspect = image_aspect::COLOR,
                .baseMipLevel = 0,
                .mipLevelCount = 1,
                .baseLayer = 0,
                .layerCount = 1
            }
        };

        image_memory_barrier depth = {
            .src = access_type::NONE,
            .dst = access_type::SHADER_READ,
            .oldLayout = image_layout::DEPTH_ATTACHMENT_OPTIMAL,
            .newLayout = image_layout::SHADER_READ_ONLY_OPTIMAL,
            .srcQueue = list.queue_index(),
            .dstQueue = list.queue_index(),
            .img = _prevRenderTarget.depth.img,
            .range = {
                .aspect = image_aspect::DEPTH,
                .baseMipLevel = 0,
                .mipLevelCount = 1,
                .baseLayer = 0,
                .layerCount = 1
            }
        };

        const image_memory_barrier barriers[] = {color, depth};

        list.barrier(pipeline_stage::TOP_OF_PIPE, pipeline_stage::FRAGMENT_SHADER, {}, {}, span(barriers));
    }

}