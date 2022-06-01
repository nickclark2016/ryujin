#ifndef types_hpp__
#define types_hpp__

#include "../core/as.hpp"
#include "../core/optional.hpp"
#include "../core/primitives.hpp"
#include "../core/span.hpp"
#include "../core/string.hpp"
#include "../core/variant.hpp"

#include <VkBootstrap.h>
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <cstddef>

namespace ryujin
{
    enum class access_type : u32
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
        DEPTH_STENCIL_ATTACHMENT_READ = 0x00200,
        DEPTH_STENCIL_ATTACHMENT_WRITE = 0x00400,
        TRANSFER_READ = 0x00800,
        TRANSFER_WRITE = 0x01000,
        HOST_READ = 0x02000,
        HOST_WRITE = 0x04000,
        MEMORY_READ = 0x08000,
        MEMORY_WRITE = 0x10000,
    };

    enum class address_mode : u32
    {
        REPEAT = 0,
        MIRRORED_REPEAT = 1,
        CLAMP_TO_EDGE = 2,
        BORDER = 3,
        MIRROR_CLAMP_TO_EDGE = 4
    };

    enum class attachment_load_op : u32
    {
        LOAD = 0,
        CLEAR = 1,
        DONT_CARE = 2
    };

    enum class attachment_store_op : u32
    {
        STORE = 0,
        DONT_CARE = 1
    };

    enum class blend_factor
    {
        ZERO = 0,
        ONE = 1,
        SRC_COLOR = 2,
        ONE_MINUS_SRC_COLOR = 3,
        DST_COLOR = 4,
        ONE_MINUS_DST_COLOR = 5,
        SRC_ALPHA = 6,
        ONE_MINUS_SRC_ALPHA = 7,
        DST_ALPHA = 8,
        ONE_MINUS_DST_ALPHA = 9,
        CONSTANT_COLOR = 10,
        ONE_MINUS_CONSTANT_COLOR = 11,
        CONSTANT_ALPHA = 12,
        ONE_MINUS_CONSTANT_ALPHA = 13,
        SRC_ALPHA_SATURATE = 14,
    };

    enum class blend_op
    {
        ADD = 0,
        SUBTRACT = 1,
        REVERSE_SUBTRACT = 2,
        MIN = 3,
        MAX = 4
    };

    enum class buffer_usage : u32
    {
        TRANSFER_SRC = 0x001,
        TRANSFER_DST = 0x002,
        UNIFORM_TEXEL = 0x004,
        STORAGE_TEXEL = 0x008,
        UNIFORM = 0x010,
        STORAGE = 0x020,
        INDEX = 0x040,
        VERTEX = 0x080,
        INDIRECT = 0x100,
    };

    enum class compare_op : u32
    {
        NEVER = 0,
        LESS = 1,
        EQUAL = 2,
        LESS_OR_EQUAL = 3,
        GREATER = 4,
        NOT_EQUAL = 5,
        GREATER_OR_EQUAL = 6,
        ALWAYS = 7
    };

    enum class cull_mode : u32
    {
        NONE = 0x00,
        FRONT = 0x01,
        BACK = 0x02
    };

    enum class data_format : u32
    {
        UNDEFINED = 0,
        R4G4_UNORM_PACK8 = 1,
        R4G4B4A4_UNORM_PACK16 = 2,
        B4G4R4A4_UNORM_PACK16 = 3,
        R5G6B5_UNORM_PACK16 = 4,
        B5G6R5_UNORM_PACK16 = 5,
        R5G5B5A1_UNORM_PACK16 = 6,
        B5G5R5A1_UNORM_PACK16 = 7,
        A1R5G5B5_UNORM_PACK16 = 8,
        R8_UNORM = 9,
        R8_SNORM = 10,
        R8_USCALED = 11,
        R8_SSCALED = 12,
        R8_UINT = 13,
        R8_SINT = 14,
        R8_SRGB = 15,
        R8G8_UNORM = 16,
        R8G8_SNORM = 17,
        R8G8_USCALED = 18,
        R8G8_SSCALED = 19,
        R8G8_UINT = 20,
        R8G8_SINT = 21,
        R8G8_SRGB = 22,
        R8G8B8_UNORM = 23,
        R8G8B8_SNORM = 24,
        R8G8B8_USCALED = 25,
        R8G8B8_SSCALED = 26,
        R8G8B8_UINT = 27,
        R8G8B8_SINT = 28,
        R8G8B8_SRGB = 29,
        B8G8R8_UNORM = 30,
        B8G8R8_SNORM = 31,
        B8G8R8_USCALED = 32,
        B8G8R8_SSCALED = 33,
        B8G8R8_UINT = 34,
        B8G8R8_SINT = 35,
        B8G8R8_SRGB = 36,
        R8G8B8A8_UNORM = 37,
        R8G8B8A8_SNORM = 38,
        R8G8B8A8_USCALED = 39,
        R8G8B8A8_SSCALED = 40,
        R8G8B8A8_UINT = 41,
        R8G8B8A8_SINT = 42,
        R8G8B8A8_SRGB = 43,
        B8G8R8A8_UNORM = 44,
        B8G8R8A8_SNORM = 45,
        B8G8R8A8_USCALED = 46,
        B8G8R8A8_SSCALED = 47,
        B8G8R8A8_UINT = 48,
        B8G8R8A8_SINT = 49,
        B8G8R8A8_SRGB = 50,
        A8B8G8R8_UNORM_PACK32 = 51,
        A8B8G8R8_SNORM_PACK32 = 52,
        A8B8G8R8_USCALED_PACK32 = 53,
        A8B8G8R8_SSCALED_PACK32 = 54,
        A8B8G8R8_UINT_PACK32 = 55,
        A8B8G8R8_SINT_PACK32 = 56,
        A8B8G8R8_SRGB_PACK32 = 57,
        A2R10G10B10_UNORM_PACK32 = 58,
        A2R10G10B10_SNORM_PACK32 = 59,
        A2R10G10B10_USCALED_PACK32 = 60,
        A2R10G10B10_SSCALED_PACK32 = 61,
        A2R10G10B10_UINT_PACK32 = 62,
        A2R10G10B10_SINT_PACK32 = 63,
        A2B10G10R10_UNORM_PACK32 = 64,
        A2B10G10R10_SNORM_PACK32 = 65,
        A2B10G10R10_USCALED_PACK32 = 66,
        A2B10G10R10_SSCALED_PACK32 = 67,
        A2B10G10R10_UINT_PACK32 = 68,
        A2B10G10R10_SINT_PACK32 = 69,
        R16_UNORM = 70,
        R16_SNORM = 71,
        R16_USCALED = 72,
        R16_SSCALED = 73,
        R16_UINT = 74,
        R16_SINT = 75,
        R16_SFLOAT = 76,
        R16G16_UNORM = 77,
        R16G16_SNORM = 78,
        R16G16_USCALED = 79,
        R16G16_SSCALED = 80,
        R16G16_UINT = 81,
        R16G16_SINT = 82,
        R16G16_SFLOAT = 83,
        R16G16B16_UNORM = 84,
        R16G16B16_SNORM = 85,
        R16G16B16_USCALED = 86,
        R16G16B16_SSCALED = 87,
        R16G16B16_UINT = 88,
        R16G16B16_SINT = 89,
        R16G16B16_SFLOAT = 90,
        R16G16B16A16_UNORM = 91,
        R16G16B16A16_SNORM = 92,
        R16G16B16A16_USCALED = 93,
        R16G16B16A16_SSCALED = 94,
        R16G16B16A16_UINT = 95,
        R16G16B16A16_SINT = 96,
        R16G16B16A16_SFLOAT = 97,
        R32_UINT = 98,
        R32_SINT = 99,
        R32_SFLOAT = 100,
        R32G32_UINT = 101,
        R32G32_SINT = 102,
        R32G32_SFLOAT = 103,
        R32G32B32_UINT = 104,
        R32G32B32_SINT = 105,
        R32G32B32_SFLOAT = 106,
        R32G32B32A32_UINT = 107,
        R32G32B32A32_SINT = 108,
        R32G32B32A32_SFLOAT = 109,
        R64_UINT = 110,
        R64_SINT = 111,
        R64_SFLOAT = 112,
        R64G64_UINT = 113,
        R64G64_SINT = 114,
        R64G64_SFLOAT = 115,
        R64G64B64_UINT = 116,
        R64G64B64_SINT = 117,
        R64G64B64_SFLOAT = 118,
        R64G64B64A64_UINT = 119,
        R64G64B64A64_SINT = 120,
        R64G64B64A64_SFLOAT = 121,
        B10G11R11_UFLOAT_PACK32 = 122,
        E5B9G9R9_UFLOAT_PACK32 = 123,
        D16_UNORM = 124,
        X8_D24_UNORM_PACK32 = 125,
        D32_SFLOAT = 126,
        S8_UINT = 127,
        D16_UNORM_S8_UINT = 128,
        D24_UNORM_S8_UINT = 129,
        D32_SFLOAT_S8_UINT = 130,
        BC1_RGB_UNORM_BLOCK = 131,
        BC1_RGB_SRGB_BLOCK = 132,
        BC1_RGBA_UNORM_BLOCK = 133,
        BC1_RGBA_SRGB_BLOCK = 134,
        BC2_UNORM_BLOCK = 135,
        BC2_SRGB_BLOCK = 136,
        BC3_UNORM_BLOCK = 137,
        BC3_SRGB_BLOCK = 138,
        BC4_UNORM_BLOCK = 139,
        BC4_SNORM_BLOCK = 140,
        BC5_UNORM_BLOCK = 141,
        BC5_SNORM_BLOCK = 142,
        BC6H_UFLOAT_BLOCK = 143,
        BC6H_SFLOAT_BLOCK = 144,
        BC7_UNORM_BLOCK = 145,
        BC7_SRGB_BLOCK = 146,
        ETC2_R8G8B8_UNORM_BLOCK = 147,
        ETC2_R8G8B8_SRGB_BLOCK = 148,
        ETC2_R8G8B8A1_UNORM_BLOCK = 149,
        ETC2_R8G8B8A1_SRGB_BLOCK = 150,
        ETC2_R8G8B8A8_UNORM_BLOCK = 151,
        ETC2_R8G8B8A8_SRGB_BLOCK = 152,
        EAC_R11_UNORM_BLOCK = 153,
        EAC_R11_SNORM_BLOCK = 154,
        EAC_R11G11_UNORM_BLOCK = 155,
        EAC_R11G11_SNORM_BLOCK = 156,
        ASTC_4x4_UNORM_BLOCK = 157,
        ASTC_4x4_SRGB_BLOCK = 158,
        ASTC_5x4_UNORM_BLOCK = 159,
        ASTC_5x4_SRGB_BLOCK = 160,
        ASTC_5x5_UNORM_BLOCK = 161,
        ASTC_5x5_SRGB_BLOCK = 162,
        ASTC_6x5_UNORM_BLOCK = 163,
        ASTC_6x5_SRGB_BLOCK = 164,
        ASTC_6x6_UNORM_BLOCK = 165,
        ASTC_6x6_SRGB_BLOCK = 166,
        ASTC_8x5_UNORM_BLOCK = 167,
        ASTC_8x5_SRGB_BLOCK = 168,
        ASTC_8x6_UNORM_BLOCK = 169,
        ASTC_8x6_SRGB_BLOCK = 170,
        ASTC_8x8_UNORM_BLOCK = 171,
        ASTC_8x8_SRGB_BLOCK = 172,
        ASTC_10x5_UNORM_BLOCK = 173,
        ASTC_10x5_SRGB_BLOCK = 174,
        ASTC_10x6_UNORM_BLOCK = 175,
        ASTC_10x6_SRGB_BLOCK = 176,
        ASTC_10x8_UNORM_BLOCK = 177,
        ASTC_10x8_SRGB_BLOCK = 178,
        ASTC_10x10_UNORM_BLOCK = 179,
        ASTC_10x10_SRGB_BLOCK = 180,
        ASTC_12x10_UNORM_BLOCK = 181,
        ASTC_12x10_SRGB_BLOCK = 182,
        ASTC_12x12_UNORM_BLOCK = 183,
        ASTC_12x12_SRGB_BLOCK = 184,
    };

    enum class descriptor_type : u32
    {
        SAMPLER = 0,
        COMBINED_IMAGE_SAMPLER = 1,
        SAMPLED_IMAGE = 2,
        STORAGE_IMAGE = 3,
        UNIFORM_TEXEL_BUFFER = 4,
        STORAGE_TEXEL_BUFFER = 5,
        UNIFORM_BUFFER = 6,
        STORAGE_BUFFER = 7,
        DYNAMIC_UNIFORM_BUFFER = 8,
        DYNAMIC_STORAGE_BUFFER = 9,
        INPUT_ATTACHMENT = 10
    };

    enum class dynamic_pipeline_state : u32
    {
        VIEWPORT = 0,
        SCISSOR = 1
    };

    enum class filter : u32
    {
        NEAREST = 0,
        LINEAR = 1
    };

    enum class logic_op : u32
    {
        CLEAR = 0,
        AND = 1,
        AND_REVERSE = 2,
        COPY = 3,
        AND_INVERTED = 4,
        NO_OP = 5,
        XOR = 6,
        OR = 7,
        NOR = 8,
        EQUIVALENT = 9,
        INVERT = 10,
        REVERSE = 11,
        COPY_INVERTED = 12,
        OR_INVERTED = 13,
        NAND = 14,
        SET = 15
    };

    enum class image_aspect : u32
    {
        COLOR = 0x01,
        DEPTH = 0x02,
        STENCIL = 0x04
    };

    enum class image_layout : u32
    {
        UNDEFINED = 0,
        GENERAL = 1,
        COLOR_ATTACHMENT_OPTIMAL = 2,
        DEPTH_STENCIL_ATTACHMENT_OPTIMAL = 3,
        DEPTH_STENCIL_READ_ONLY_OPTIMAL = 4,
        SHADER_READ_ONLY_OPTIMAL = 5,
        TRANSFER_SRC_OPTIMAL = 6,
        TRANSFER_DST_OPTIMAL = 7,
        PREINITIALIZED = 8,
        DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL = 1000117000,
        DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL = 1000117001,
        DEPTH_ATTACHMENT_OPTIMAL = 1000241000,
        DEPTH_READ_ONLY_OPTIMAL = 1000241001,
        STENCIL_ATTACHMENT_OPTIMAL = 1000241002,
        STENCIL_READ_ONLY_OPTIMAL = 1000241003,
        READ_ONLY_OPTIMAL = 1000314000,
        ATTACHMENT_OPTIMAL = 1000314001,
        PRESENT_SRC_KHR = 1000001002,
    };

    enum class image_usage : u32
    {
        TRANSFER_SRC = 0x001,
        TRANSFER_DST = 0x002,
        SAMPLED = 0x004,
        STORAGE = 0x008,
        COLOR_ATTACHMENT = 0x010,
        DEPTH_STENCIL_ATTACHMENT = 0x020,
        TRANSIENT_ATTACHMENT = 0x040,
        INPUT_ATTACHMENT = 0x080
    };

    enum class image_type : u32
    {
        TYPE_1D = 0,
        TYPE_2D = 1,
        TYPE_3D = 2
    };

    enum class image_view_type : u32
    {
        TYPE_1D,
        TYPE_2D,
        TYPE_3D,
        TYPE_CUBE,
        TYPE_1D_ARRAY,
        TYPE_2D_ARRAY,
        TYPE_CUBE_ARRAY
    };

    enum class input_binding_rate : u32
    {
        PER_VERTEX = 0,
        PER_INSTANCE = 1
    };

    enum class mipmap_mode : u32
    {
        NEAREST = 0,
        LINEAR = 1
    };

    enum class memory_property : u32
    {
        NONE = 0x0,
        DEVICE_LOCAL = 0x01,
        HOST_VISIBLE = 0x02,
        HOST_COHERENT = 0x04,
        HOST_CACHED = 0x04
    };

    enum class memory_usage : u32
    {
        PREFER_AUTO_SELECT = 7,
        PREFER_DEVICE = 8,
        PREFER_HOST = 9,
    };

    enum class pipeline_stage : u32
    {
        TOP_OF_PIPE = 0x00001,
        DRAW_INDIRECT = 0x00002,
        VERTEX_INPUT = 0x00004,
        VERTEX_SHADER = 0x00008,
        TESSELLATION_CONTROL_SHADER = 0x00010,
        TESSELLATION_EVALUATION_SHADER = 0x00020,
        GEOMETRY_SHADER = 0x00040,
        FRAGMENT_SHADER = 0x00080,
        EARLY_FRAGMENT_TESTS = 0x00100,
        LATE_FRAGMENT_TESTS = 0x00200,
        COLOR_ATTACHMENT_OUTPUT = 0x00400,
        COMPUTE_SHADER = 0x00800,
        TRANSFER = 0x01000,
        BOTTOM_OF_PIPE = 0x02000,
        HOST = 0x04000,
        ALL_GRAPHICS = 0x08000,
        ALL_COMMANDS = 0x10000,
    };

    enum class polygon_rasterization_mode : u32
    {
        FILL = 0,
        LINE = 1,
        POINT = 2
    };

    enum class primitive_topology : u32
    {
        POINT_LIST = 0,
        LINE_LIST = 1,
        LINE_STRIP = 2,
        TRIANGLE_LIST = 3,
        TRIANGLE_STRIP = 4,
        TRIANGLE_FAN = 5,
        LINE_LIST_WITH_ADJACENCY = 6,
        LINE_STRIP_WITH_ADJACENCY = 7,
        TRIANGLE_LIST_WITH_ADJACENCY = 8,
        TRIANGLE_STRIP_WITH_ADJACENCY = 9,
        PATCH_LIST = 10
    };

    enum class sample_count : u32
    {
        COUNT_1 = 0x01,
        COUNT_2 = 0x02,
        COUNT_4 = 0x04,
        COUNT_8 = 0x08,
        COUNT_16 = 0x10
    };

    enum class shader_stage : u32
    {
        VERTEX = 0x001,
        TESSELLATION_CONTROL = 0x002,
        TESSELLATION_EVALUATION = 0x004,
        GEOMETRY = 0x008,
        FRAGMENT = 0x010,
        COMPUTE = 0x020,
    };

    enum class stencil_op : u32
    {
        KEEP = 0,
        ZERO = 1,
        REPLACE = 2,
        INCREMENT_AND_CLAMP = 3,
        DECREMENT_AND_CLAMP = 4,
        INVERT = 5,
        INCREMENT_AND_WRAP = 6,
        DECREMENT_AND_WRAP = 7
    };

    enum class vertex_winding_order : u32
    {
        COUNTER_CLOCKWISE = 0,
        CLOCKWISE = 1
    };

    using clear_value = VkClearValue;
    using descriptor_pool = VkDescriptorPool;
    using descriptor_set = VkDescriptorSet;
    using descriptor_set_layout = VkDescriptorSetLayout;
    using fence = VkFence;
    using frame_buffer = VkFramebuffer;
    using image_sampler = VkSampler;
    using image_view = VkImageView;
    using pipeline = VkPipeline;
    using pipeline_layout = VkPipelineLayout;
    using render_pass = VkRenderPass;
    using semaphore = VkSemaphore;
    using shader_module = VkShaderModule;

    struct buffer
    {
        VkBuffer buffer;
        VmaAllocation allocation;
        VmaAllocationInfo info;
    };

    struct image
    {
        VkImage image;
        VmaAllocation allocation;
        VmaAllocationInfo info;
    };

    struct attachment_description
    {
        data_format format;
        sample_count samples;
        attachment_load_op load;
        attachment_store_op store;
        attachment_load_op stencilLoad;
        attachment_store_op stencilStore;
        image_layout initialLayout;
        image_layout finalLayout;
    };

    struct attachment_reference
    {
        u32 attachment;
        image_layout layout;
        image_aspect aspect;
    };

    struct subpass_description
    {
        span<attachment_reference> inputs;
        span<attachment_reference> colors;
        span<attachment_reference> resolves;
        optional<attachment_reference> depthStencil;
        span<u32> preserveIndices;
    };

    struct subpass_dependency
    {
        u32 srcSubpassIndex;
        u32 dstSubpassIndex;
        pipeline_stage srcStagesMask;
        pipeline_stage dstStagesMask;
        access_type srcAccessMask;
        access_type dstAccessMask;
    };

    struct render_pass_create_info
    {
        span<attachment_description> attachments;
        span<subpass_description> subpasses;
        span<subpass_dependency> dependencies;
        string name;
    };

    struct frame_buffer_attachment_image_info
    {
        image_usage usage;
        span<data_format> formats;
    };

    struct frame_buffer_attachment_create_info
    {
        span<frame_buffer_attachment_image_info> infos;
    };

    struct frame_buffer_create_info
    {
        variant<frame_buffer_attachment_create_info, span<image_view>> attachments;
        render_pass pass;
        u32 width;
        u32 height;
        u32 layers;
        string name;
    };

    struct shader_module_create_info
    {
        span<unsigned char> bytes;
    };

    struct shader_stage_info
    {
        shader_stage stage;
        shader_module mod;
    };

    struct vertex_input_binding
    {
        u32 binding;
        u32 stride;
        input_binding_rate rate;
    };

    struct vertex_input_attribute
    {
        u32 location;
        u32 binding;
        u32 offset;
        data_format format;
    };

    struct vertex_input_state_info
    {
        span<vertex_input_binding> bindings;
        span<vertex_input_attribute> attributes;
    };

    struct input_assembly_state_info
    {
        primitive_topology topology;
        bool restartEnabled;
    };

    struct viewport
    {
        float x;
        float y;
        float width;
        float height;
        float minDepth;
        float maxDepth;
    };

    struct scissor_region
    {
        i32 x;
        i32 y;
        u32 width;
        u32 height;
    };

    struct viewport_state_info
    {
        span<viewport> viewports;
        span<scissor_region> scissors;
    };

    struct depth_bias_state_info
    {
        float constant;
        float clamp;
        float slope;
    };

    struct rasterization_state_info
    {
        bool depthClampEnabled;
        bool rasterizerDiscardEnabled;
        polygon_rasterization_mode poly;
        cull_mode cull;
        vertex_winding_order front;
        optional<depth_bias_state_info> depthBias;
        float lineWidth;
    };

    struct multisample_state_info
    {
        sample_count samples;
        bool enableSampleShading;
        float minSampleShading;
        span<u32> sampleMask;
        bool alphaToCoverageEnabled;
        bool alphaToOneEnabled;
    };

    struct depth_test_state
    {
        bool writeEnabled;
        compare_op depthCompareOp;
    };

    struct depth_bounds_state
    {
        float minDepthBounds;
        float maxDepthBounds;
    };

    struct stencil_test_state
    {
        stencil_op fail;
        stencil_op pass;
        stencil_op depthFail;
        compare_op compare;
        u32 compareMask;
        u32 writeMask;
        u32 reference;
    };

    struct depth_stencil_state_info
    {
        optional<depth_test_state> depthTest;
        optional<depth_bounds_state> depthBoundsTest;
        optional<stencil_test_state> stencilTest;
    };

    struct component_attachment_blend_state
    {
        blend_factor src;
        blend_factor dst;
        blend_op op;
    };

    struct color_attachment_blend_state
    {
        bool enabled;
        component_attachment_blend_state rgb;
        component_attachment_blend_state alpha;
    };

    struct color_blend_state_info
    {
        optional<logic_op> logicOp;
        span<color_attachment_blend_state> attachments;
        float blendConstants[4];
    };

    struct graphics_pipeline_create_info
    {
        span<shader_stage_info> stages;
        vertex_input_state_info vertexInput;
        input_assembly_state_info inputAssembly;
        viewport_state_info viewport;
        rasterization_state_info rasterizationState;
        multisample_state_info multisampleState;
        optional<depth_stencil_state_info> depthStencilState;
        color_blend_state_info blendState;
        span<dynamic_pipeline_state> dynamicStates;
        pipeline_layout layout;
        render_pass pass;
        u32 subpass;
    };

    struct image_create_info
    {
        image_type type;
        data_format format;
        u32 width;
        u32 height;
        u32 depth;
        u32 mipLevels;
        u32 arrayLayers;
        sample_count samples;
        image_usage usage;
    };

    struct image_view_usage
    {
        image_usage usage;
    };

    struct image_subresource_range
    {
        image_aspect aspect;
        u32 baseMipLevel;
        u32 mipLevelCount;
        u32 baseLayer;
        u32 layerCount;
    };

    struct image_view_create_info
    {
        optional<image_view_usage> usage;

        image img;
        image_view_type type;
        data_format fmt;
        image_subresource_range subresource;
    };

    struct descriptor_set_layout_binding
    {
        u32 binding;
        descriptor_type type;
        u32 count;
        shader_stage stages;
    };

    struct descriptor_set_layout_create_info
    {
        span<descriptor_set_layout_binding> bindings;
    };

    struct push_constant_range
    {
        shader_stage stages;
        u32 offset;
        u32 size;
    };

    struct pipeline_layout_create_info
    {
        span<descriptor_set_layout> layouts;
        span<push_constant_range> pushConstants;
        string name;
    };

    struct buffer_create_info
    {
        sz size;
        buffer_usage usage;
    };

    struct allocation_create_info
    {
        memory_property required;
        memory_property preferred;
        memory_usage usage;
        bool hostSequentialWriteAccess;
        bool hostRandomAccess;
        bool persistentlyMapped;
    };

    struct sampler_create_info
    {
        filter min;
        filter mag;
        mipmap_mode mipmapMode;
        address_mode u;
        address_mode v;
        address_mode w;
        float mipLodBias;
        bool enableAnisotropy;
        float maxAnisotropy;
        optional<compare_op> compare;
        float minLod;
        float maxLod;
        bool unnormalizedCoordinates;
        string name;
    };

    struct descriptor_pool_size
    {
        descriptor_type type;
        u32 count;
    };

    struct descriptor_pool_create_info
    {
        u32 maxSetCount;
        span<descriptor_pool_size> sizes;
    };

    struct descriptor_set_allocate_info
    {
        descriptor_pool pool;
        descriptor_set_layout layout;
    };

    struct fence_create_info
    {
        bool signaled;
    };
    
    struct descriptor_buffer_info
    {
        buffer buf;
        sz offset;
        sz length;
    };

    struct descriptor_image_info
    {
        image_view view;
        image_sampler sam;
        image_layout layout;
    };

    struct descriptor_write_info
    {
        descriptor_set set;
        descriptor_type type;
        u32 binding;
        u32 element;
        variant<span<descriptor_buffer_info>, span<descriptor_image_info>> info;
    };

    struct render_pass_attachment_begin_info
    {
        span<image_view> views;
    };

    struct render_pass_begin_info
    {
        optional<render_pass_attachment_begin_info> attachmentBegin;
        render_pass pass;
        frame_buffer buffer;
        int32_t x;
        int32_t y;
        uint32_t width;
        uint32_t height;
        span<clear_value> clearValues;
    };

    struct buffer_copy_regions
    {
        sz srcOffset;
        sz dstOffset;
        sz size;
    };

    struct image_subresource_layers
    {
        image_aspect aspect;
        u32 mipLevel;
        u32 baseArrayLayer;
        u32 layerCount;
    };

    struct buffer_image_copy_regions
    {
        sz bufferOffset;
        sz rowLength;
        sz imageHeight;
        image_subresource_layers subresource;
        i32 x;
        i32 y;
        i32 z;
        u32 width;
        u32 height;
        u32 depth;
    };

    struct wait_info
    {
        semaphore sem;
        pipeline_stage stageMask;
    };

    struct submit_info
    {
        span<wait_info> wait;
        span<semaphore> signal;
    };

    struct memory_barrier
    {
        access_type src;
        access_type dst;
    };

    struct buffer_memory_barrier
    {
        access_type src;
        access_type dst;

        uint32_t srcQueue;
        uint32_t dstQueue;

        buffer buf;
        sz offset;
        sz size;
    };

    struct image_memory_barrier
    {
        access_type src;
        access_type dst;
        image_layout oldLayout;
        image_layout newLayout;

        uint32_t srcQueue;
        uint32_t dstQueue;

        image img;
        image_subresource_range range;
    };

    struct texture
    {
        image img;
        image_view view;
        image_sampler sampler;
    };

    constexpr bool operator==(const buffer& lhs, const buffer& rhs) noexcept
    {
        return lhs.buffer == rhs.buffer;
    }

    constexpr bool operator!=(const buffer& lhs, const buffer& rhs) noexcept
    {
        return lhs.buffer != rhs.buffer;
    }

    constexpr bool operator==(const image& lhs, const image& rhs) noexcept
    {
        return lhs.image == rhs.image;
    }

    constexpr bool operator!=(const image& lhs, const image& rhs) noexcept
    {
        return lhs.image != rhs.image;
    }

    constexpr access_type operator|(const access_type lhs, const access_type rhs)
    {
        return as<access_type>(as<u32>(lhs) | as<u32>(rhs));
    }

    constexpr buffer_usage operator|(const buffer_usage lhs, const buffer_usage rhs)
    {
        return as<buffer_usage>(as<u32>(lhs) | as<u32>(rhs));
    }

    constexpr image_aspect operator|(const image_aspect lhs, const image_aspect rhs)
    {
        return as<image_aspect>(as<u32>(lhs) | as<u32>(rhs));
    }

    constexpr image_usage operator|(const image_usage lhs, const image_usage rhs)
    {
        return as<image_usage>(as<u32>(lhs) | as<u32>(rhs));
    }

    constexpr pipeline_stage operator|(const pipeline_stage lhs, const pipeline_stage rhs)
    {
        return as<pipeline_stage>(as<u32>(lhs) | as<u32>(rhs));
    }

    constexpr shader_stage operator|(const shader_stage lhs, const shader_stage rhs)
    {
        return as<shader_stage>(as<u32>(lhs) | as<u32>(rhs));
    }

    constexpr cull_mode operator|(const cull_mode lhs, const cull_mode rhs)
    {
        return as<cull_mode>(as<u32>(lhs) | as<u32>(rhs));
    }

    constexpr memory_property operator|(const memory_property lhs, const memory_property rhs)
    {
        return as<memory_property>(as<u32>(lhs) | as<u32>(rhs));
    }
}

#endif // types_hpp__