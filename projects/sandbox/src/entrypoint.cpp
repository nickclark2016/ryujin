#include <ryujin/input.hpp>
#include <ryujin/render_graph.hpp>
#include <ryujin/window.hpp>

#include <cmath>
#include <iostream>

using namespace ryujin;

int main(int, char**)
{
    auto winRes = window::create(window::create_info{
            .name = "Render Graph Sandbox",
            .width = 1920,
            .height = 1080
        });

    if (!winRes)
    {
        return 1;
    }

    auto& win = *winRes;

    render_graph g(win);

    auto colorTarget = g.add_render_target(render_target_builder("scene_color")
        .format(data_format::R8G8B8A8_SRGB)
        .width(1920)
        .height(1080)
        .use_as_color_attachment()
        .use_as_input_attachment());

    auto resolvePass = g.add_pass(pass_builder("resolve_pass")
            .add_color_target(render_target_usage{
                .tgt = colorTarget,
                .load = attachment_load_op::CLEAR,
                .store = attachment_store_op::STORE,
                .clear = {.color = {.float32 { 0.0f, 0.0f, 0.0f, 1.0f }}},
            })
            .on_pass_execute([](command_buffer& cmds) {
                
            }));

    while (!win->should_close())
    {
        input::poll();

        g.execute();
    }

    return 0;
}