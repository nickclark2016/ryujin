#include <ryujin/core/engine.hpp>
#include <ryujin/graphics/render_system.hpp>

using namespace ryujin;

class sandbox_application final : public base_application
{
public:
    sandbox_application()
    {
    }

    void on_load(engine_context& ctx) override
    {
        window::create_info winInfo =
        {
            "Sandbox",
            1280,
            720
        };

        ctx.add_window(winInfo);
        ctx.get_assets().load_texture("data/textures/logo512.png");
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