#include <ryujin/core/engine.hpp>
#include <ryujin/graphics/render_system.hpp>
#include <ryujin/graphics/pipelines/pbr_render_pipeline.hpp>
#include <ryujin/math/transformations.hpp>

using namespace ryujin;

class sandbox_application final : public base_application
{
public:
    sandbox_application()
    {
    }

    void pre_init(engine_context& ctx) override
    {
        window::create_info winInfo =
        {
            "Sandbox",
            1280,
            720
        };

        ctx.add_window(winInfo);
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

        const mat4 transformation = transform(vec3(-2.0f, 2.0f, 5.0f), rotation, vec3(1.0f));

        auto entity = ctx.get_registry().allocate();
        entity.assign(cubeRenderableComponent);
        entity.get<transform_component>().matrix = transformation;

        const mat4 transformation2 = transform(vec3(2.0f, 2.0f, 5.0f), rotation, vec3(1.0f));

        auto entity2 = ctx.get_registry().allocate();
        entity2.assign(cubeRenderableComponent);
        entity2.get<transform_component>().matrix = transformation2;

        const mat4 transformation3 = transform(vec3(-2.0f, -2.0f, 5.0f), rotation, vec3(1.0f));

        auto entity3 = ctx.get_registry().allocate();
        entity3.assign(cubeRenderableComponent);
        entity3.get<transform_component>().matrix = transformation3;

        const mat4 transformation4 = transform(vec3(2.0f, -2.0f, 5.0f), rotation, vec3(1.0f));

        auto entity4 = ctx.get_registry().allocate();
        entity4.assign(cubeRenderableComponent);
        entity4.get<transform_component>().matrix = transformation4;
    }

    void on_exit(engine_context& ctx) override
    {

    }

    void on_frame(engine_context& ctx) override
    {

    }
};

int main(int argc, char** argv)
{
    auto engine = std::make_unique<engine_context>();
    std::unique_ptr<base_application> app(new sandbox_application());

    engine->execute(app);

    return 0;
}