#ifndef render_system_hpp__
#define render_system_hpp__

#include "render_manager.hpp"
#include "window.hpp"

#include "../core/primitives.hpp"
#include "../entities/registry.hpp"

#include <VkBootstrap.h>
#include <vma/vk_mem_alloc.h>

#include <memory>
#include <vector>

namespace ryujin
{
    class engine_context;

    class render_system final
    {
    public:
        render_system();
        ~render_system();

        void on_init(engine_context& ctx);
        void render_prework(engine_context& ctx);

        void on_pre_render(engine_context& ctx);
        void on_render(engine_context& ctx);
        void on_post_render(engine_context& ctx);

        std::unique_ptr<render_manager>& get_render_manager(sz idx) noexcept;
        sz render_manager_count() const noexcept;
    private:
        std::vector<std::unique_ptr<render_manager>> _managers;

        vkb::Instance _instance;
        vkb::Device _device;
        VmaAllocator _allocator;

        bool _shouldNameObjects = false;
    };
}

#endif // render_system_hpp__
