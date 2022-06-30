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

    auto graph = render_graph::create_render_graph(*win);

    const attachment_reference triangleColorRef[] = { { .index = 0, .layout = image_layout::COLOR} };

    const subpass_create_info subpasses[] = {
        {
            .type = operation_type::GRAPHICS,
            .colors = { triangleColorRef },
        }
    };

    const subpass_dependency_create_info dependencies[] = {
        {
            .srcSubpassIndex = subpass_dependency_create_info::external_dependency_index,
            .dstSubpassIndex = 0,
            .srcStageMask = pipeline_stage::COLOR_ATTACHMENT_WRITE,
            .dstStageMask = pipeline_stage::COLOR_ATTACHMENT_WRITE,
            .srcAccessMask = memory_access::NONE,
            .dstAccessMask = memory_access::COLOR_ATTACHMENT_WRITE
        },
        {
            .srcSubpassIndex = 0,
            .dstSubpassIndex = subpass_dependency_create_info::external_dependency_index,
            .srcStageMask = pipeline_stage::COLOR_ATTACHMENT_WRITE,
            .dstStageMask = pipeline_stage::BOTTOM,
            .srcAccessMask = memory_access::COLOR_ATTACHMENT_WRITE,
            .dstAccessMask = memory_access::NONE
        }
    };

    const render_pass_framebuffer_attachment_info attachments[] = {
        {
            .fmt = graph->swapchain_image_format(),
            .samples = samples_per_pixel::COUNT_1,
            .load = framebuffer_attachment_load_op::CLEAR,
            .store = framebuffer_attachment_store_op::STORE,
            .input = image_layout::UNDEFINED,
            .output = image_layout::SWAPCHAIN
        }
    };

    auto trianglePass = graph->create_render_pass({
            .name = "triangle",
            .attachments = attachments,
            .subpasses = subpasses,
            .dependencies = dependencies
        });

    while (!win->should_close())
    {
        input::poll();

        graph->execute();
    }

    return 0;
}