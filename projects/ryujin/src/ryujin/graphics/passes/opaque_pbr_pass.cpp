#include <ryujin/graphics/passes/opaque_pbr_pass.hpp>

#include <ryujin/core/files.hpp>
#include <ryujin/core/primitives.hpp>
#include <ryujin/graphics/render_manager.hpp>

namespace ryujin
{
    opaque_pbr_pass::opaque_pbr_pass(render_manager& manager, pipeline_layout layout, render_pass pass, u32 width, u32 height)
        : _manager(manager), _layout(layout)
    {
        init_graphics_pipeline(width, height, pass);
    }

    void opaque_pbr_pass::render(graphics_command_list& cmd, buffer& indirect, buffer& count, sz indirectOffset, sz countOffset, sz numBufferGroups)
    {
        cmd.bind_graphics_pipeline(_shader);

        const auto countPtr = reinterpret_cast<u32*>(count.info.pMappedData) + countOffset;
        const auto indirectBaseAddr = countOffset * sizeof(gpu_indirect_call);
        auto indirectAddrOffset = 0;
        const auto countBaseAddr = indirectOffset * sizeof(u32);
        auto countAddrOffset = 0;

        for (sz bufferGroupId = 0; bufferGroupId < numBufferGroups; ++bufferGroupId)
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

    void opaque_pbr_pass::init_graphics_pipeline(u32 width, u32 height, render_pass pass)
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
            .enabled = false,
            .rgb = {},
            .alpha = {}
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
            .stride = 3 * sizeof(u16),
            .rate = input_binding_rate::PER_VERTEX
        };

        const vertex_input_binding interleavedBinding = {
            .binding = 1,
            .stride = 6 * sizeof(u16),
            .rate = input_binding_rate::PER_VERTEX
        };

        const vertex_input_binding vertexBindings[] = { positionsBinding, interleavedBinding };

        const vertex_input_attribute positionAttrib = {
            .location = 0,
            .binding = 0,
            .offset = 0,
            .format = data_format::R16G16B16_SNORM
        };

        const vertex_input_attribute tbnAttrib = {
            .location = 1,
            .binding = 1,
            .offset = 0,
            .format = data_format::R16G16B16A16_SNORM
        };

        const vertex_input_attribute texCoord0Attrib = {
            .location = 2,
            .binding = 1,
            .offset = 4 * sizeof(u16),
            .format = data_format::R16G16_SNORM
        };

        const vertex_input_attribute vertexAttribs[] = { positionAttrib, tbnAttrib, texCoord0Attrib };
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
				.sampleMask = span<u32>(),
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