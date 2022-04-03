#include <ryujin/graphics/passes/blit_pass.hpp>

#include <ryujin/core/as.hpp>
#include <ryujin/core/files.hpp>
#include <ryujin/graphics/render_manager.hpp>

namespace ryujin
{
	blit_pass::blit_pass(render_manager& manager)
		: _manager(manager)
	{
		build_render_pass();
		build_pipeline_layout();
		build_pipeline();
		build_input_sampler();
		rebuild_target(manager.get_swapchain_width(), manager.get_swapchain_height(), true);
	}

	void blit_pass::set_input_texture(image_view view)
	{
		_inputImageView = view;
	}

	void blit_pass::set_output_texture(image_view view, const std::uint32_t width, const std::uint32_t height)
	{
		_imageTarget = view;
		rebuild_target(width, height);
	}

	void blit_pass::record(graphics_command_list& commands)
	{
		const render_pass_attachment_begin_info attachments = { .views = span(_imageTarget) };
		const clear_value swapchainClear = { .color = { .float32 = { 1.0f, 0, 0, 1 } } };
		const clear_value clears[] = { swapchainClear };

		const render_pass_begin_info rpBegin = {
			.attachmentBegin = attachments,
			.pass = _pass,
			.buffer = _target,
			.x = 0,
			.y = 0,
			.width = _targetWidth,
			.height = _targetHeight,
			.clearValues = clears
		};

		scissor_region sc = {
			.x = 0,
			.y = 0,
			.width = _targetWidth,
			.height = _targetHeight
		};

		viewport vp = {
			.x = 0,
			.y = 0,
			.width = as<float>(_targetWidth),
			.height = as<float>(_targetHeight),
			.minDepth = 0,
			.maxDepth = 1
		};

		auto imageSet = *_manager.allocate_transient(_imageInputSetLayout);
		_sets[_frame] = imageSet;

		descriptor_image_info image = { .view = _inputImageView, .sam = _inputSampler, .layout = image_layout::SHADER_READ_ONLY_OPTIMAL };
		descriptor_writer(&_manager).write_image(_sets[_frame], 0, descriptor_type::COMBINED_IMAGE_SAMPLER, 0, span(image));

		commands.begin_render_pass(rpBegin);
		commands.bind_graphics_pipeline(_shader);
		commands.bind_graphics_descriptor_sets(_layout, _sets[_frame]);
		commands.set_scissors(sc);
		commands.set_viewports(vp);
		commands.draw_arrays(3);
		commands.end_render_pass();

		_frame = (_frame + 1) % 2;
	}
	
	void blit_pass::build_render_pass()
	{
		attachment_description swapchainAttachmentDesc = {
			.format = _manager.get_swapchain_format(),
			.samples = sample_count::COUNT_1,
			.load = attachment_load_op::CLEAR,
			.store = attachment_store_op::STORE,
			.stencilLoad = attachment_load_op::DONT_CARE,
			.stencilStore = attachment_store_op::DONT_CARE,
			.initialLayout = image_layout::UNDEFINED,
			.finalLayout = image_layout::PRESENT_SRC_KHR
		};

		attachment_reference swapchainAttachmentColorReference = {
			.attachment = 0,
			.layout = image_layout::COLOR_ATTACHMENT_OPTIMAL,
			.aspect = image_aspect::COLOR
		};

		attachment_reference resolveColorAttachments[] = { swapchainAttachmentColorReference };

		subpass_description resolveSubpass = {
			.inputs = span<attachment_reference>(),
			.colors = span(resolveColorAttachments),
			.resolves = span<attachment_reference>(),
			.depthStencil = std::nullopt,
			.preserveIndices = span<std::uint32_t>()
		};

		subpass_dependency externalInboundDependency = {
			.srcSubpassIndex = VK_SUBPASS_EXTERNAL,
			.dstSubpassIndex = 0,
			.srcStagesMask = pipeline_stage::ALL_COMMANDS,
			.dstStagesMask = pipeline_stage::FRAGMENT_SHADER,
			.srcAccessMask = access_type::COLOR_ATTACHMENT_WRITE,
			.dstAccessMask = access_type::MEMORY_READ,
		};

		subpass_dependency externalInboundDependency2 = {
			VK_SUBPASS_EXTERNAL,
			0,
			pipeline_stage::COLOR_ATTACHMENT_OUTPUT,
			pipeline_stage::COLOR_ATTACHMENT_OUTPUT,
			access_type::NONE,
			access_type::COLOR_ATTACHMENT_WRITE,
		};

		attachment_description attachmentDescriptions[] = { swapchainAttachmentDesc };
		subpass_description subpassDescriptions[] = { resolveSubpass };
		subpass_dependency subpassDependencies[] = { externalInboundDependency, externalInboundDependency2 };

		render_pass_create_info renderPassCreateInfo = {
			.attachments = attachmentDescriptions,
			.subpasses = subpassDescriptions,
			.dependencies = subpassDependencies,
			.name = "blit_render_pass"
		};

		_pass = *_manager.create(renderPassCreateInfo);
	}

	void blit_pass::build_pipeline_layout()
	{
		const descriptor_set_layout_binding inputImageBinding = {
			.binding = 0,
			.type = descriptor_type::COMBINED_IMAGE_SAMPLER,
			.count = 1,
			.stages = shader_stage::FRAGMENT
		};

		const descriptor_set_layout_binding set0Bindings[] = { inputImageBinding };

		const descriptor_set_layout_create_info setLayout = {
			.bindings = set0Bindings
		};

		const auto setBindingResult = _manager.create(setLayout);
		if (setBindingResult)
		{
			_imageInputSetLayout = *setBindingResult;
		}
		else
		{
			return;
		}

		const pipeline_layout_create_info linfo = {
			.layouts = span(_imageInputSetLayout),
			.name = "blit_pipeline_layout"
		};
		auto layoutResult = _manager.create(linfo);
		if (layoutResult)
		{
			_layout = *layoutResult;
		}
	}

	void blit_pass::build_pipeline()
	{
		auto vertexSource = files::load_binary("data/shaders/fullscreen/shader.vert.spv");
		auto fragmentSource = files::load_binary("data/shaders/blit/shader.frag.spv");

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
			.width = as<float>(_manager.get_swapchain_width()),
			.height = as<float>(_manager.get_swapchain_height()),
			.minDepth = 0.0f,
			.maxDepth = 1.0f
		};

		scissor_region sc = { 
			.x = 0,
			.y = 0, 
			.width = _manager.get_swapchain_width(),
			.height = _manager.get_swapchain_height()
		};

		color_attachment_blend_state swapchainBlend = {
			.enabled = false,
			.rgb = {},
			.alpha = {}
		};

		color_attachment_blend_state blendStates[] = { swapchainBlend };
		dynamic_pipeline_state dynamicStates[] = { dynamic_pipeline_state::VIEWPORT, dynamic_pipeline_state::SCISSOR };

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
				.viewports = span(&vp, 1),
				.scissors = span(&sc, 1)
			},
			.rasterizationState = {
				.depthClampEnabled = false,
				.rasterizerDiscardEnabled = false,
				.poly = polygon_rasterization_mode::FILL,
				.cull = cull_mode::NONE,
				.front = vertex_winding_order::CLOCKWISE,
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
			.depthStencilState = std::nullopt,
			.blendState = {
				.logicOp = std::nullopt,
				.attachments = span(blendStates),
				.blendConstants = { 1.0f, 1.0f, 1.0f, 1.0f }
			},
			.dynamicStates = span(dynamicStates),
			.layout = _layout,
			.pass = _pass,
			.subpass = 0
		};

		auto pipelineResult = _manager.create(pipelineInfo);
		if (pipelineResult)
		{
			_shader = *pipelineResult;
		}
	}

	void blit_pass::build_input_sampler()
	{
		sampler_create_info inputSamplerInfo = {
			.min = filter::LINEAR,
			.mag = filter::LINEAR,
			.mipmapMode = mipmap_mode::LINEAR,
			.u = address_mode::REPEAT,
			.v = address_mode::REPEAT,
			.w = address_mode::REPEAT,
			.mipLodBias = 0.0f,
			.enableAnisotropy = false,
			.maxAnisotropy = 0.0f,
			.compare = std::nullopt,
			.minLod = 0.0f,
			.maxLod = 1.0f,
			.unnormalizedCoordinates = false,
			.name = "blit_input_sampler"
		};

		auto samplerRes = _manager.create(inputSamplerInfo);
		if (samplerRes)
		{
			_inputSampler = *samplerRes;
		}
	}

	void blit_pass::rebuild_target(const std::uint32_t width, const std::uint32_t height, const bool force)
	{
		if (_targetWidth == width && _targetHeight == height && !force)
		{
			return;
		}

		if (_target)
		{
			_manager.release(_target);
		}

		data_format swapchainFormats[] = { _manager.get_swapchain_format() };

		frame_buffer_attachment_image_info swapchainAttachmentCreate = {
			.usage = image_usage::COLOR_ATTACHMENT,
			.formats = swapchainFormats
		};

		frame_buffer_attachment_image_info attachmentImageInfos[] = { swapchainAttachmentCreate };

		frame_buffer_attachment_create_info attachments = {
			.infos = attachmentImageInfos
		};

		frame_buffer_create_info finfo = {
			.attachments = attachments,
			.pass = _pass,
			.width = width,
			.height = height,
			.layers = 1,
			.name = "swapchain_blit_target",
		};

		auto framebufferResult = _manager.create(finfo);
		if (framebufferResult)
		{
			_target = *framebufferResult;
			_targetWidth = width;
			_targetHeight = height;
		}
	}
}