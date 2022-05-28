#include <ryujin/core/engine.hpp>
#include <ryujin/graphics/render_system.hpp>
#include <ryujin/input/input.hpp>
#include <ryujin/graphics/pipelines/pbr_render_pipeline.hpp>

#include <ryujin/core/string.hpp>

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
        //win->capture_cursor();

        win->on_focus([&win](bool) {
                win->capture_cursor();
            });
    }

    entity_handle<registry::entity_type> cubeEnt;
    vec3<float> cubeRotation = { 0.0f, 45.0f, 0.0f };

    void on_load(engine_context& ctx) override
    {
	    const auto cube = ctx.get_assets().load_model("data/models/cube/Cube.gltf");

	    const auto& manager = ctx.get_render_system().get_render_manager(0);
        manager->use_render_pipeline<pbr_render_pipeline>();

        auto& renderables = manager->renderables();

        cubeEnt = renderables.load_to_entities(ctx.get_assets(), *cube); // actually get first child
        auto& hierarchy = cubeEnt.get<entity_relationship_component<registry::entity_type>>();
        cubeEnt = entity_handle(hierarchy.firstChild, &ctx.get_registry());

        auto& cubeTx = cubeEnt.get<transform_component>();
        set_position(cubeTx, vec3(0.0f, 0.0f, 0.0f));

        for (size_t i = 1; i < 4096; ++i)
        {
            auto e = renderables.load_to_entities(ctx.get_assets(), *cube); // actually get first child
            auto& hierarchy = e.get<entity_relationship_component<registry::entity_type>>();
            e = entity_handle(hierarchy.firstChild, &ctx.get_registry());

            auto& eTx = e.get<transform_component>();
            set_position(eTx, vec3(i * 4.0f, 0.0f, 0.0f));
        }

        _camera = free_look_camera(vec3(0.0f, 1.0f, -10.0f), ctx.get_registry());

        renderables.build_meshes();
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
        auto& cubeTx = cubeEnt.get<transform_component>();
        cubeRotation.x = std::fmod(cubeRotation.x + as<float>(2.0 * ctx.deltaTime()), 360.0f);
        set_rotation(cubeTx, as_radians(cubeRotation));

	    const auto in = input::get_input();
        if (!in) return;

        if (in->get_keys().get_state(keyboard::key::ESCAPE) == keyboard::state::PRESSED)
        {
            ctx.get_window("Sandbox")->release_cursor();
        }

        if (in->get_mouse().get_state(mouse::button::LEFT) == mouse::state::PRESSED)
        {
            ctx.get_window("Sandbox")->capture_cursor();
        }

        if (ctx.get_window("Sandbox")->is_cursor_captured())
        {
            _camera.on_update(ctx.deltaTime());
        }
    }
};

int main(int, char**)
{
	const auto engine = std::make_unique<engine_context>();
    ryujin::unique_ptr<base_application> app(new sandbox_application());

    engine->execute(app);

    return 0;
}