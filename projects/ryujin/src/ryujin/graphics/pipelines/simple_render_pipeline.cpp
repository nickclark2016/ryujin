#include <ryujin/graphics/pipelines/simple_render_pipeline.hpp>

#include <ryujin/core/as.hpp>
#include <ryujin/core/files.hpp>
#include <ryujin/graphics/render_manager.hpp>

namespace ryujin::detail
{
	void simple_render_pipeline::initialize()
	{
		init_simple_pass();
		init_render_target();
		init_graphics_pipeline();

		_blit = std::make_unique<blit_pass>(*get_render_manager());
		_triangle = std::make_unique<triangle_pass>(*get_render_manager(), _simplePass);
	}

	void simple_render_pipeline::pre_render()
	{
	}

	void simple_render_pipeline::render()
	{
		auto graphicsList = get_render_manager()->next_graphics_command_list();
		graphicsList.begin();

		clear_value clears[] = {
			{
				.color = {
					.float32 = {
						1.0f, 1.0f, 0.0f, 1.0f
					}
				}
			}
		};

		render_pass_begin_info beginInfo = {
			.attachmentBegin = std::nullopt,
			.pass = _simplePass,
			.buffer = _renderTarget,
			.x = 0,
			.y = 0,
			.width = 1920,
			.height = 1080,
			.clearValues = clears
		};

		graphicsList.begin_render_pass(beginInfo);
		_triangle->record(graphicsList);
		graphicsList.end_render_pass();

		_blit->set_input_texture(_targetView);
		_blit->set_output_texture(get_render_manager()->get_swapchain_image(), get_render_manager()->get_swapchain_width(), get_render_manager()->get_swapchain_height());
		_blit->record(graphicsList);

		graphicsList.end();

		wait_info wait[] = {
			{ 
				get_render_manager()->swapchain_image_ready_signal(),
				pipeline_stage::COLOR_ATTACHMENT_OUTPUT
			}
		};

		semaphore signal[] = { get_render_manager()->render_complete_signal() };

		const submit_info submit = {
			span(wait),
			span(signal)
		};

		graphicsList.submit(submit, get_render_manager()->flight_complete_fence());
	}
	
	void simple_render_pipeline::init_simple_pass()
	{
		attachment_description swapchainAttachmentDesc = {
			_targetFormat,
			sample_count::COUNT_1,
			attachment_load_op::CLEAR,
			attachment_store_op::STORE,
			attachment_load_op::DONT_CARE,
			attachment_store_op::DONT_CARE,
			image_layout::UNDEFINED,
			image_layout::SHADER_READ_ONLY_OPTIMAL
		};

		attachment_reference swapchainAttachmentColorReference = {
			0,
			image_layout::COLOR_ATTACHMENT_OPTIMAL,
			image_aspect::COLOR
		};

		attachment_reference resolveColorAttachments[] = { swapchainAttachmentColorReference };

		subpass_description resolveSubpass = {
			span<attachment_reference>(),
			span(resolveColorAttachments),
			span<attachment_reference>(),
			std::nullopt,
			span<u32>()
		};

		subpass_dependency externalInboundDependency = {
			VK_SUBPASS_EXTERNAL,
			0,
			pipeline_stage::COLOR_ATTACHMENT_OUTPUT,
			pipeline_stage::COLOR_ATTACHMENT_OUTPUT,
			access_type::NONE,
			access_type::COLOR_ATTACHMENT_WRITE,
		};

		subpass_dependency externalOutboundDependency = {
			0,
			VK_SUBPASS_EXTERNAL,
			pipeline_stage::COLOR_ATTACHMENT_OUTPUT,
			pipeline_stage::TOP_OF_PIPE,
			access_type::COLOR_ATTACHMENT_WRITE,
			access_type::NONE,
		};

		attachment_description attachmentDescriptions[] = { swapchainAttachmentDesc };
		subpass_description subpassDescriptions[] = { resolveSubpass };
		subpass_dependency subpassDependencies[] = { externalInboundDependency, externalOutboundDependency };

		render_pass_create_info renderPassCreateInfo = {
			attachmentDescriptions,
			subpassDescriptions,
			subpassDependencies,
			"simple_render_pass"
		};

		const auto simplePassResult = get_render_manager()->create(renderPassCreateInfo);
		if (simplePassResult)
		{
			_simplePass = *simplePassResult;
		}
	}
	
	void simple_render_pipeline::init_render_target()
	{		
		const image_create_info imgInfo = {
			.type = image_type::TYPE_2D,
			.format = _targetFormat,
			.width = 1920,
			.height = 1080,
			.depth = 1,
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = sample_count::COUNT_1,
			.usage = image_usage::COLOR_ATTACHMENT | image_usage::SAMPLED
		};

		const allocation_create_info imgAllocInfo = {
			.required = memory_property::DEVICE_LOCAL,
			.preferred = memory_property::DEVICE_LOCAL,
			.usage = memory_usage::PREFER_DEVICE,
			.hostSequentialWriteAccess = false,
			.hostRandomAccess = false
		};

		const auto targetRes = get_render_manager()->create(imgInfo, imgAllocInfo);
		if (targetRes)
		{
			_target = *targetRes;
		}

		const image_view_create_info targetViewInfo = {
			.usage = image_view_usage{ .usage = image_usage::COLOR_ATTACHMENT | image_usage::SAMPLED },
			.img = _target,
			.type = image_view_type::TYPE_2D,
			.fmt = _targetFormat,
			.subresource = { .aspect = image_aspect::COLOR, .baseMipLevel = 0, .mipLevelCount = 1, .baseLayer = 0, .layerCount = 1 }
		};

		const auto targetViewRes = get_render_manager()->create(targetViewInfo);
		if (targetViewRes)
		{
			_targetView = *targetViewRes;
		}

		const image_view attachments[] = { _targetView };

		frame_buffer_create_info fboInfo = {
			attachments,
			_simplePass,
			1920,
			1080,
			1,
			"final_render_target"
		};

		const auto fboResult = get_render_manager()->create(fboInfo);
		if (fboResult)
		{
			_renderTarget = *fboResult;
		}
	}
	
	void simple_render_pipeline::init_graphics_pipeline()
	{
	}
}