#include <ryujin/graphics/passes/naive_translucent_pbr_pass.hpp>

#include <ryujin/core/files.hpp>
#include <ryujin/graphics/render_manager.hpp>

namespace ryujin
{
    naive_translucent_pbr_pass::naive_translucent_pbr_pass(render_manager& manager, pipeline_layout layout, render_pass pass, std::uint32_t width, std::uint32_t height)
        : _manager(manager), _layout(layout)
    {
        init_graphics_pipeline(width, height, pass);
    }

    void naive_translucent_pbr_pass::render(graphics_command_list& cmd, buffer& indirect, buffer& count, std::size_t indirectOffset, std::size_t countOffset, std::size_t numBufferGroups)
    {
        cmd.bind_graphics_pipeline(_shader);

        const auto countPtr = reinterpret_cast<std::uint32_t*>(count.info.pMappedData) + countOffset;
        const auto indirectBaseAddr = countOffset * sizeof(gpu_indirect_call);
        auto indirectAddrOffset = 0;
        const auto countBaseAddr = indirectOffset * sizeof(std::uint32_t);
        auto countAddrOffset = 0;

        for (std::size_t bufferGroupId = 0; bufferGroupId < numBufferGroups; ++bufferGroupId)
        {
            // bind mesh group
            const auto& meshGroup = _manager.renderables().get_buffer_group(bufferGroupId);
            const buffer bufs[] = { meshGroup.positions, meshGroup.interleaved };
            cmd.bind_vertex_buffers(0, span(bufs));
            cmd.bind_index_buffer(meshGroup.indices);

            // draw
            const auto indirectOffset = indirectBaseAddr + indirectAddrOffset * sizeof(gpu_indirect_call);
            const auto countOffset = countBaseAddr + countAddrOffset * sizeof(uint32_t);
            cmd.draw_indexed_indirect(indirect, indirectOffset, count, countOffset, 512, sizeof(gpu_indirect_call));

            auto drawCallCount = countPtr[countAddrOffset];
            ++countAddrOffset;
            indirectAddrOffset += drawCallCount;
        }
    }

    void naive_translucent_pbr_pass::init_graphics_pipeline(std::uint32_t width, std::uint32_t height, render_pass pass)
    {
        auto vertexSource = files::load_binary("data/shaders/pbr/shader.vert.spv");
        auto fragmentSource = files::load_binary("data/shaders/pbr/shader.opaque.frag.spv");

        if (!vertexSource || !fragmentSource)
        {
            return;
        }

        shader_module_create_info vertexModuleInfo = {
            .bytes = span(reinterpret_cast<unsigned char*>(vertexSource->bytes.get()), vertexSource->length)
        };

        shader_module_create_info fragmentModuleInfo = {
            .bytes = span(reinterpret_cast<unsigned char*>(fragmentSource->bytes.get()), fragmentSource->length)
        };

        auto vertexModule = _manager.create(vertexModuleInfo);
        auto fragmentModule = _manager.create(fragmentModuleInfo);

        const shader_stage_info vertexStage = {
            .stage = shader_stage::VERTEX,
            .mod = *vertexModule
        };

        const shader_stage_info fragmentStage = {
            .stage = shader_stage::FRAGMENT,
            .mod = *fragmentModule
        };

        const shader_stage_info stages[] = { vertexStage, fragmentStage };

        const viewport vp = {
            .x = 0.0f,
            .y = 0.0f,
            .width = as<float>(width),
            .height = as<float>(height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };

        const scissor_region sc = {
            .x = 0,
            .y = 0,
            .width = width,
            .height = height
        };

        const color_attachment_blend_state colorTargetNoBlend = {
            .enabled = true,
            .rgb = {
                .src = blend_factor::SRC_ALPHA,
                .dst = blend_factor::DST_ALPHA,
                .op = blend_op::ADD
            },
            .alpha = {
                .src = blend_factor::ONE,
                .dst = blend_factor::ZERO,
                .op = blend_op::ADD
            }
        };

        const color_attachment_blend_state colorBlends[] = { colorTargetNoBlend };

        const depth_stencil_state_info depthState = {
            .depthTest = depth_test_state{
                .writeEnabled = true,
                .depthCompareOp = compare_op::LESS
            }
        };

        const vertex_input_binding positionsBinding = {
            .binding = 0,
            .stride = 3 * sizeof(float),
            .rate = input_binding_rate::PER_VERTEX
        };

        const vertex_input_binding interleavedBinding = {
            .binding = 1,
            .stride = (2 + 3 + 4) * sizeof(float),
            .rate = input_binding_rate::PER_VERTEX
        };

        const vertex_input_binding vertexBindings[] = { positionsBinding, interleavedBinding };

        const vertex_input_attribute positionAttrib = {
            .location = 0,
            .binding = 0,
            .offset = 0,
            .format = data_format::R32G32B32_SFLOAT
        };

        const vertex_input_attribute texCoord0Attrib = {
            .location = 1,
            .binding = 1,
            .offset = 0,
            .format = data_format::R32G32_SFLOAT
        };

        const vertex_input_attribute normalAttrib = {
            .location = 2,
            .binding = 1,
            .offset = 2 * sizeof(float),
            .format = data_format::R32G32B32_SFLOAT
        };

        const vertex_input_attribute tangentAttrib = {
            .location = 3,
            .binding = 1,
            .offset = normalAttrib.offset + 3 * sizeof(float),
            .format = data_format::R32G32B32A32_SFLOAT
        };

        const vertex_input_attribute vertexAttribs[] = { positionAttrib, texCoord0Attrib, normalAttrib, tangentAttrib };
        const dynamic_pipeline_state dynStatesEnabled[] = { dynamic_pipeline_state::VIEWPORT, dynamic_pipeline_state::SCISSOR };

        const graphics_pipeline_create_info pipelineInfo = {
            .stages = span(stages),
            .vertexInput = {
                .bindings = span(vertexBindings),
                .attributes = span(vertexAttribs)
            },
            .inputAssembly = {
                .topology = primitive_topology::TRIANGLE_LIST,
                .restartEnabled = false
            },
            .viewport = {
                .viewports = span(&vp, 1),
                .scissors = span(&sc, 1)
            },
            .rasterizationState = {
                .depthClampEnabled = false,
                .rasterizerDiscardEnabled = false,
                .poly = polygon_rasterization_mode::FILL,
                .cull = cull_mode::BACK,
                .front = vertex_winding_order::COUNTER_CLOCKWISE,
                .depthBias = std::nullopt,
                .lineWidth = 1.0f
            },
            .multisampleState = {
                .samples = sample_count::COUNT_1,
                .enableSampleShading = false,
                .minSampleShading = 0.0f,
                .sampleMask = span<std::uint32_t>(),
                .alphaToCoverageEnabled = false,
                .alphaToOneEnabled = false
            },
            .depthStencilState = depthState,
            .blendState = {
                .logicOp = std::nullopt,
                .attachments = span(colorBlends),
                .blendConstants = { 1.0f, 1.0f, 1.0f, 1.0f }
            },
            .dynamicStates = span(dynStatesEnabled),
            .layout = _layout,
            .pass = pass,
            .subpass = 0
        };

        auto pipelineResult = _manager.create(pipelineInfo);
        if (pipelineResult)
        {
            _shader = *pipelineResult;
        }
    }
}