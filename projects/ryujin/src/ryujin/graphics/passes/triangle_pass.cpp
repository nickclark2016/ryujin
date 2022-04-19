#include <ryujin/graphics/passes/triangle_pass.hpp>

#include <ryujin/core/files.hpp>
#include <ryujin/graphics/render_manager.hpp>

namespace ryujin
{
    triangle_pass::triangle_pass(render_manager& manager, const render_pass& pass)
        : _manager(manager)
    {
        build_pipeline_layout();
        build_pipeline(pass);
    }

    void triangle_pass::record(graphics_command_list& commands)
    {
		commands.bind_graphics_pipeline(_shader);
		commands.draw_arrays(3);
    }
    
    void triangle_pass::build_pipeline_layout()
    {
        const pipeline_layout_create_info layoutCi = {
            .name = "empty_pipeline_layout"
        };

        const auto res = _manager.create(layoutCi);
        if (res)
        {
            _layout = *res;
        }
    }
    
    void triangle_pass::build_pipeline(const render_pass& pass)
    {
		auto vertexSource = files::load_binary("data/shaders/simple_triangle/shader.vert.spv");
		auto fragmentSource = files::load_binary("data/shaders/simple_triangle/shader.frag.spv");

		shader_module_create_info vertexModuleInfo = {
			.bytes = span(reinterpret_cast<unsigned char*>(vertexSource->bytes.get()), vertexSource->length)
		};

		shader_module_create_info fragmentModuleInfo = {
			.bytes = span(reinterpret_cast<unsigned char*>(fragmentSource->bytes.get()), fragmentSource->length)
		};

		auto vertexModule = _manager.create(vertexModuleInfo);
		auto fragmentModule = _manager.create(fragmentModuleInfo);

		shader_stage_info vertexStage = {
			.stage = shader_stage::VERTEX,
			.mod = *vertexModule
		};

		shader_stage_info fragmentStage = {
			.stage = shader_stage::FRAGMENT,
			.mod = *fragmentModule
		};

		shader_stage_info stages[] = { vertexStage, fragmentStage };

		viewport vp = {
			.x = 0.0f,
			.y = 0.0f,
			.width = 1920,
			.height = 1080,
			.minDepth = 0.0f,
			.maxDepth = 1.0f
		};

		scissor_region sc = {
			.x = 0,
			.y = 0,
			.width = 1920,
			.height = 1080
		};

		color_attachment_blend_state swapchainBlend = {
			.enabled = false,
			.rgb = {},
			.alpha = {}
		};

		color_attachment_blend_state blendStates[] = { swapchainBlend };

		graphics_pipeline_create_info pipelineInfo = {
			.stages = span(stages),
			.vertexInput = {
				.bindings = {},
				.attributes = {}
			},
			.inputAssembly = {
				.topology = primitive_topology::TRIANGLE_LIST,
				.restartEnabled = false
			},
			.viewport = {
				.viewports = vp,
				.scissors = sc
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
			.depthStencilState = std::nullopt,
			.blendState = {
				.logicOp = std::nullopt,
				.attachments = span(blendStates),
				.blendConstants = { 1.0f, 1.0f, 1.0f, 1.0f }
			},
			.dynamicStates = {},
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