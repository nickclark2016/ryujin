#include <ryujin/core/engine.hpp>
#include <ryujin/graphics/camera_component.hpp>
#include <ryujin/graphics/render_system.hpp>
#include <ryujin/input/input.hpp>
#include <ryujin/graphics/pipelines/pbr_render_pipeline.hpp>
#include <ryujin/math/transformations.hpp>

#include <iostream>

using namespace ryujin;

class sandbox_application final : public base_application
{
public:
    sandbox_application() = default;

    entity_handle<registry::entity_type> camera;

    void pre_init(engine_context& ctx) override
    {
        window::create_info winInfo =
        {
            "Sandbox",
            1280,
            720
        };

        ctx.add_window(winInfo)->focus();
    }

    void on_load(engine_context& ctx) override
    {
        auto cube = ctx.get_assets().load_model("data/models/cube/Cube.gltf");

        auto& manager = ctx.get_render_system().get_render_manager(0);
        manager->use_render_pipeline<pbr_render_pipeline>();

        auto& renderables = manager->renderables();

        // auto logoAsset = renderables.load_texture("logo", *tex);

        auto& cubeMeshGroup = ctx.get_assets().get_mesh_group(cube->get_mesh_group())->meshes[0];
        auto cubeMesh = renderables.load_mesh("cube_mesh", cubeMeshGroup);
        auto cubeMaterial = renderables.load_material("cube_material", *cubeMeshGroup.material);
        
        renderable_component cubeRenderableComponent
        {
            .material = cubeMaterial,
            .mesh = cubeMesh,
        };

        renderables.build_meshes();

        const vec3<float> rotation(3.1415f / 4.0f, 0.0f, 0.0f);

        auto entity = ctx.get_registry().allocate();
        entity.assign(cubeRenderableComponent);
        set_transform(entity.get<transform_component>(), vec3(-2.0f, 2.0f, 5.0f), rotation, vec3(1.0f));

        const mat4 transformation2 = transform(vec3(2.0f, 2.0f, 5.0f), rotation, vec3(1.0f));

        auto entity2 = ctx.get_registry().allocate();
        entity2.assign(cubeRenderableComponent);
        set_transform(entity2.get<transform_component>(), vec3(2.0f, 2.0f, 5.0f), rotation, vec3(1.0f));

        auto entity3 = ctx.get_registry().allocate();
        entity3.assign(cubeRenderableComponent);
        set_transform(entity3.get<transform_component>(), vec3(-2.0f, -2.0f, 5.0f), rotation, vec3(1.0f));

        const mat4 transformation4 = transform(vec3(2.0f, -2.0f, 5.0f), rotation, vec3(1.0f));

        auto entity4 = ctx.get_registry().allocate();
        entity4.assign(cubeRenderableComponent);
        set_transform(entity4.get<transform_component>(), vec3(2.0f, -2.0f, 5.0f), rotation, vec3(1.0f));

        camera = ctx.get_registry().allocate();
        camera.assign(camera_component{
            .near = 0.01f,
            .far = 1000.0f,
            .fov = 90.0f,
            .order = 1,
            .active = true
        });
        set_transform(camera.get<transform_component>(), vec3(0.0f, 0.0f, -1.0f), as_radians(vec3(0.0f, 15.0f, 0.0f)), vec3(1.0f));
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
        auto& tx = camera.get<transform_component>();

        auto in = input::get_input();
        if (!in) return;

        if (in->keys().get_state(keyboard::key::W) != keyboard::state::RELEASED)
        {
            set_position(tx, tx.position + vec3(0.0f, 0.0f, 0.01f));
        }
        else if (in->keys().get_state(keyboard::key::S) != keyboard::state::RELEASED)
        {
            set_position(tx, tx.position + vec3(0.0f, 0.0f, -0.01f));
        }
    }
};

int main(int argc, char** argv)
{
    auto engine = std::make_unique<engine_context>();
    std::unique_ptr<base_application> app(new sandbox_application());

    engine->execute(app);

    return 0;
}