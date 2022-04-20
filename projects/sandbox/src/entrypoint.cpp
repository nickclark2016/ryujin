#include <ryujin/core/engine.hpp>
#include <ryujin/graphics/render_system.hpp>
#include <ryujin/input/input.hpp>
#include <ryujin/graphics/pipelines/pbr_render_pipeline.hpp>

#include <cmath>
#include <iostream>

#include "free_look_camera.hpp"

using namespace ryujin;

class sandbox_application final : public base_application
{
    free_look_camera _camera;

public:
    sandbox_application() = default;

    void pre_init(engine_context& ctx) override
    {
        const window::create_info winInfo =
        {
            "Sandbox",
            1280,
            720
        };

        auto& win = ctx.add_window(winInfo);
        win->focus();
        win->capture_cursor();

        win->on_focus([&win](bool) {
                win->capture_cursor();
            });
    }

    void on_load(engine_context& ctx) override
    {
	    const auto cube = ctx.get_assets().load_model("data/models/cube/Cube.gltf");

	    const auto& manager = ctx.get_render_system().get_render_manager(0);
        manager->use_render_pipeline<pbr_render_pipeline>();

        auto& renderables = manager->renderables();

        renderables.load_to_entities(ctx.get_assets(), *cube);

        renderables.build_meshes();

        std::this_thread::sleep_for(std::chrono::seconds(1));
        _camera = free_look_camera(vec3(-5.0f, 5.0f, -5.0f), ctx.get_registry());
    }

    void on_exit(engine_context& ctx) override
    {
    }

    void on_render(engine_context& ctx) override
    {
    }

    void post_render(engine_context& ctx) override
    {
    }

    void on_frame(engine_context& ctx) override
    {
	    const auto in = input::get_input();
        if (!in) return;

        if (in->get_keys().get_state(keyboard::key::ESCAPE) == keyboard::state::PRESSED)
        {
            ctx.get_window("Sandbox")->release_cursor();
        }

        if(in->get_keys().get_state(keyboard::key::C) == keyboard::state::PRESSED)
        {
            ctx.get_window("Sandbox")->capture_cursor();
        }

        if (ctx.get_window("Sandbox")->is_cursor_captured())
        {
            _camera->on_update(1.0 / 60.0);
        }
    }
};

int main(int, char**)
{
	const auto engine = std::make_unique<engine_context>();
    std::unique_ptr<base_application> app(new sandbox_application());

    engine->execute(app);

    return 0;
}