#ifndef render_graph_hpp__
#define render_graph_hpp__

#include <ryujin/memory.hpp>
#include <ryujin/optional.hpp>
#include <ryujin/primitives.hpp>
#include <ryujin/slot_map.hpp>
#include <ryujin/span.hpp>
#include <ryujin/window.hpp>

namespace ryujin
{
    enum class render_target_format : u32
    {
        RGBA8_SRGB = 43,
        RGBA16_FLOAT = 97,
        DEPTH32_FLOAT = 126,
        DEPTH16_UNORM_SAMPLES8_UINT = 128,
        DEPTH24_UNORM_SAMPLE8_UINT = 129,
        DEPTH32_FLOAT_SAMPLES8_UINT = 130,
    };

    enum class samples_per_pixel : u32
    {
        COUNT_1 = 0x1,
        COUNT_2 = 0x2,
        COUNT_4 = 0x4,
        COUNT_8 = 0x8,
        COUNT_16 = 0x16
    };

    enum class framebuffer_attachment_load_op : u32
    {
        LOAD = 0,
        CLEAR = 1,
        DONT_CARE = 2
    };

    enum class framebuffer_attachment_store_op : u32
    {
        STORE = 0,
        DONT_CARE = 1
    };

    enum class image_layout : u32
    {
        UNDEFINED = 0,
        COLOR = 2,
        DEPTH_STENCIL = 3,
        DEPTH_STENCIL_READ_ONLY = 4,
        SHADER_READ_ONLY = 5,
        TRANSFER_SRC = 6,
        TRANSFER_DST = 7,
        SWAPCHAIN = 1000001002
    };

    enum class operation_type : u32
    {
        GRAPHICS = 0,
        COMPUTE = 1
    };

    enum class pipeline_stage : u32
    {
        TOP = 0x0001,
        DRAW_INDIRECT = 0x0002,
        VERTEX_INPUT = 0x0004,
        VERTEX_SHADER = 0x0008,
        TESSELLATION_CONTROL_SHADER = 0x0010,
        TESSELLATION_EVALUATION_SHADER = 0x0020,
        GEOMETRY_SHADER = 0x0040,
        FRAGMENT_SHADER = 0x0080,
        EARLY_FRAGMENT_TESTS = 0x0100,
        LATE_FRAGMNET_TESTS = 0x0200,
        COLOR_ATTACHMENT_WRITE = 0x0400,
        COMPUTE_SHADER = 0x0800,
        TRANSFER = 0x1000,
        BOTTOM = 0x02000
    };

    enum class memory_access : u32
    {
        NONE = 0,
        INDIRECT_COMMAND_READ = 0x00001,
        INDEX_READ = 0x00002,
        VERTEX_ATTRIBUTE_READ = 0x00004,
        UNIFORM_READ = 0x00008,
        INPUT_ATTACHMENT_READ = 0x00010,
        SHADER_READ = 0x00020,
        SHADER_WRITE = 0x00040,
        COLOR_ATTACHMENT_READ = 0x00080,
        COLOR_ATTACHMENT_WRITE = 0x00100,
        DEPTH_STENCIL_READ = 0x00200,
        DEPTH_STENCIL_WRITE = 0x00400,
        TRANSFER_READ = 0x00800,
        TRANSFER_WRITE = 0x01000,
        HOST_READ = 0x02000,
        HOST_WRITE = 0x04000,
        MEMORY_READ = 0x08000,
        MEMORY_WRITE = 0x10000
    };

    class commands;
    class render_graph;
    class render_pass;
    class render_target;
    class shader;

    struct render_pass_dependency_info;

    struct render_pass_framebuffer_attachment_info
    {
        render_target_format fmt;
        samples_per_pixel samples;
        framebuffer_attachment_load_op load;
        framebuffer_attachment_store_op store;
        framebuffer_attachment_load_op stencilLoad = framebuffer_attachment_load_op::DONT_CARE;
        framebuffer_attachment_store_op stencilStore = framebuffer_attachment_store_op::DONT_CARE;
        image_layout input;
        image_layout output;
    };

    struct attachment_reference
    {
        u32 index;
        image_layout layout;
    };

    struct subpass_create_info
    {
        operation_type type;
        span<attachment_reference> inputs;
        span<attachment_reference> colors;
        span<attachment_reference> depth;
    };

    struct subpass_dependency_create_info
    {
        static constexpr u32 external_dependency_index = 0xFFFFFFFF;

        u32 srcSubpassIndex;
        u32 dstSubpassIndex;
        span<pipeline_stage> srcStageMask;
        span<pipeline_stage> dstStageMask;
        span<memory_access> srcAccessMask;
        span<memory_access> dstAccessMask;
    };

    struct render_pass_create_info
    {
        string name;
        span<render_pass_framebuffer_attachment_info> attachments;
        span<subpass_create_info> subpasses;
        span<subpass_dependency_create_info> dependencies;
    };

    class render_pass
    {
    public:
        virtual void execute(commands& cmds) = 0;
    };

    class render_graph
    {
    public:
        using handle = slot_map_key;
        
        virtual ~render_graph() = default;
        virtual void execute() = 0;

        virtual handle create_render_pass(render_pass_create_info&& info) = 0;
        virtual void on_render_pass_execute(handle h, move_only_function<void(commands&)>&& cmds) = 0;
        virtual void add_render_pass_dependency(const render_pass_dependency_info& info) = 0;
        virtual render_target_format swapchain_image_format() const noexcept = 0;

        static unique_ptr<render_graph> create_render_graph(const window& win);
    };

    class commands
    {};

    struct render_pass_dependency_info
    {
        render_graph::handle src;
        render_graph::handle dst;
    };
}

#endif // render_graph_hpp__
