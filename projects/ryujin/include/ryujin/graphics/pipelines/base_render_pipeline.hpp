#ifndef base_render_pipeline_hpp__
#define base_render_pipeline_hpp__

#include "../types.hpp"

#include "../../core/export.hpp"
#include "../../core/primitives.hpp"
#include "../../core/slot_map.hpp"
#include "../../core/string.hpp"
#include "../../core/unordered_map.hpp"

namespace ryujin
{
    class command_list;
    class render_manager;

    class base_render_pipeline
    {
    public:
        struct render_target_info
        {
            u32 width;
            u32 height;
            string name;
        };

        virtual ~base_render_pipeline() = default;

        inline void set_render_manager(render_manager* manager)
        {
            _manager = manager;
            initialize();
            _activeRenderTarget = _defaultRenderTarget;
        }

        RYUJIN_API slot_map_key add_render_target(const render_target_info& info);
        RYUJIN_API void remove_render_target(const slot_map_key& k);

        inline frame_buffer get_render_target(const slot_map_key& k)
        {
            auto target = _targets.try_get(k);
            return target ? target->fbo : frame_buffer{};
        }

        inline void get_render_target_dimensions(const slot_map_key& k, u32& width, u32& height)
        {
            auto target = _targets.try_get(k);
            if (target)
            {
                width = target->width;
                height = target->height;
            }
            else
            {
                width = 0;
                height = 0;
            }
        }

        inline virtual void pre_render() {};
        virtual void render() = 0;

    protected:
        virtual void initialize() = 0;
        
        inline render_manager* get_render_manager() noexcept
        {
            return _manager;
        }

        struct render_texture
        {
            image img;
            image_view view;
        };

        struct render_target
        {
            frame_buffer fbo;
            render_texture color;
            render_texture depth;
            slot_map_key colorTex;
            slot_map_key depthTex;
            u32 width;
            u32 height;
        };

        virtual render_target build_render_target(const string& name, const render_target_info& info) = 0;
        
        inline void set_active_render_target(const slot_map_key& k)
        {
            auto val = _targets.try_get(k);
            if (val)
            {
                _activeRenderTarget = *val;
            }
            else
            {
                _activeRenderTarget = _defaultRenderTarget;
            }
        }

        render_target _defaultRenderTarget;
        unordered_map<string, slot_map_key> _renderTargets;
        slot_map<render_target> _targets;

        render_target _activeRenderTarget = {};
        render_target _prevRenderTarget = {};

        void transition_render_targets(command_list& list);

    private:
        render_manager* _manager = nullptr;
    };
}

#endif