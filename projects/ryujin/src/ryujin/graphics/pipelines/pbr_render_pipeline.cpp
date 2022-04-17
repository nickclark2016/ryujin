#include <ryujin/graphics/pipelines/pbr_render_pipeline.hpp>

#include <ryujin/graphics/render_manager.hpp>
#include <ryujin/math/transformations.hpp>

namespace ryujin
{
    void pbr_render_pipeline::pre_render()
    {
        auto& renderables = get_render_manager()->renderables();
        auto frameInFlight = get_render_manager()->get_frame_in_flight();

        _numBufferGroupsToDraw = renderables.write_draw_calls(_indirectCommands, _indirectCount, frameInFlight * _maxDrawCalls);
        renderables.write_materials(_materials, frameInFlight * _maxMaterials);
        renderables.write_instances(_instanceData, frameInFlight * _maxInstances);
        _hostSceneData.texturesLoaded = as<std::uint32_t>(renderables.write_textures(_textures.data(), frameInFlight * _maxTextures));

        auto projection = perspective(16.0f / 9.0f, 90.0f, 0.01f, 1000.0f);

        // Build scene data
        scene_camera cam = {
            .view = mat4(1.0f),
            .proj = projection,
            .viewProj = projection,
            .position = vec3(0.0f),
            .orientation = vec3(0.0f)
        };

        auto camPtr = reinterpret_cast<scene_camera*>(_cameraData.info.pMappedData) + frameInFlight;
        *camPtr = cam;

        auto scenePtr = reinterpret_cast<scene_data*>(_sceneData.info.pMappedData) + frameInFlight;
        *scenePtr = _hostSceneData;
    }

    void pbr_render_pipeline::render()
    {
        clear_value clears[] = {
			{
				.color = {
					.float32 = {
						1.0f, 1.0f, 0.0f, 1.0f
					}
				}
			},
			{
				.depthStencil = {
					.depth = 1.0f
				}
			}
		};

        render_pass_begin_info beginInfo = {
			.attachmentBegin = std::nullopt,
			.pass = _scenePass,
			.buffer = _sceneRenderTarget,
			.x = 0,
			.y = 0,
			.width = _targetWidth,
			.height = _targetHeight,
			.clearValues = clears
		};

        auto sceneDescriptor = get_render_manager()->allocate_transient(_sceneWideLayout);
        auto drawableDescriptor = get_render_manager()->allocate_transient(_drawableLayout);
        assert(*sceneDescriptor && *drawableDescriptor);

        auto frameInFlight = get_render_manager()->get_frame_in_flight();

        const descriptor_buffer_info cameraBufferInfo = {
                .buf = _cameraData,
                .offset = frameInFlight * sizeof(scene_camera),
                .length = sizeof(scene_camera)
        };

        const descriptor_write_info camera = {
            .set = *sceneDescriptor,
            .type = descriptor_type::UNIFORM_BUFFER,
            .binding = 0,
            .element = 0,
            .info = span(cameraBufferInfo)
        };

        const descriptor_buffer_info sceneBufferInfo = {
            .buf = _sceneData,
            .offset = frameInFlight * sizeof(scene_data),
            .length = sizeof(scene_data)
        };

        const descriptor_write_info scene = {
            .set = *sceneDescriptor,
            .type = descriptor_type::STORAGE_BUFFER,
            .binding = 1,
            .element = 0,
            .info = span(sceneBufferInfo)
        };

        const descriptor_buffer_info instanceBufferInfo = {
            .buf = _instanceData,
            .offset = frameInFlight * _maxInstances * sizeof(gpu_instance_data),
            .length = _maxInstances * sizeof(gpu_instance_data)
        };

        const descriptor_write_info instances = {
            .set = *drawableDescriptor,
            .type = descriptor_type::DYNAMIC_STORAGE_BUFFER,
            .binding = 0,
            .element = 0,
            .info = span(instanceBufferInfo)
        };

        const descriptor_buffer_info materialBufferInfo = {
            .buf = _materials,
            .offset = frameInFlight * _maxMaterials * sizeof(gpu_material_data),
            .length = _maxMaterials * sizeof(gpu_material_data)
        };

        const descriptor_write_info materials = {
            .set = *drawableDescriptor,
            .type = descriptor_type::DYNAMIC_STORAGE_BUFFER,
            .binding = 1,
            .element = 0,
            .info = span(materialBufferInfo)
        };

        _textureWriteScratchBuffer.clear();
        for (std::size_t i = 0; i < _hostSceneData.texturesLoaded; ++i)
        {
            auto& tex = _textures[i];
            const descriptor_image_info info = {
                .view = tex.view,
                .sam = tex.sampler,
                .layout = image_layout::SHADER_READ_ONLY_OPTIMAL
            };
            _textureWriteScratchBuffer.push_back(info);
        }

        for (std::size_t i = _hostSceneData.texturesLoaded; i < _maxTextures; ++i)
        {
            const descriptor_image_info info = {
                .view = _invalidTexture.view,
                .sam = _invalidTexture.sampler,
                .layout = image_layout::SHADER_READ_ONLY_OPTIMAL
            };
            _textureWriteScratchBuffer.push_back(info);
        }

        const descriptor_write_info textures = {
            .set = *drawableDescriptor,
            .type = descriptor_type::COMBINED_IMAGE_SAMPLER,
            .binding = 2,
            .element = 0,
            .info = span(_textureWriteScratchBuffer.data(), _textureWriteScratchBuffer.size())
        };

        const descriptor_write_info writes[] = { camera, scene, instances, materials, textures };
        get_render_manager()->write(writes);

        const descriptor_set descriptors[] = { *sceneDescriptor, *drawableDescriptor };
        const std::uint32_t dynamicOffsets[] = { 0, 0 };

        auto graphicsList = get_render_manager()->next_graphics_command_list();
		graphicsList.begin();

        graphicsList.bind_graphics_descriptor_sets(_sceneLayout, span(descriptors), 0, span(dynamicOffsets));
		graphicsList.begin_render_pass(beginInfo);
        _opaque->render(graphicsList, _indirectCommands, _indirectCount, _maxDrawCalls * frameInFlight, _numBufferGroupsToDraw);
        graphicsList.end_render_pass();

        _blit->set_input_texture(_colorTarget.view);
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

    void pbr_render_pipeline::initialize()
    {
        init_scene_pass();
        init_scene_render_target();
        init_scene_layout();
        initialize_buffers();
        initialize_textures();

        _blit = std::make_unique<blit_pass>(*get_render_manager());
        _opaque = std::make_unique<opaque_pbr_pass>(*get_render_manager(), _sceneLayout, _scenePass, _targetWidth, _targetHeight);
    }

    void pbr_render_pipeline::init_scene_pass()
    {
        const attachment_description colorTarget = {
            .format = _colorAttachmentFmt,
            .samples = sample_count::COUNT_1,
            .load = attachment_load_op::CLEAR,
            .store = attachment_store_op::STORE,
            .stencilLoad = attachment_load_op::DONT_CARE,
            .stencilStore = attachment_store_op::DONT_CARE,
            .initialLayout = image_layout::UNDEFINED,
            .finalLayout = image_layout::SHADER_READ_ONLY_OPTIMAL
        };

        const attachment_description depthTarget = {
            .format = _depthAttachmentFmt,
            .samples = sample_count::COUNT_1,
            .load = attachment_load_op::CLEAR,
            .store = attachment_store_op::STORE,
            .stencilLoad = attachment_load_op::DONT_CARE,
            .stencilStore = attachment_store_op::DONT_CARE,
            .initialLayout = image_layout::UNDEFINED,
            .finalLayout = image_layout::DEPTH_ATTACHMENT_OPTIMAL
        };

        const attachment_reference colorTargetReference = {
            .attachment = 0,
            .layout = image_layout::COLOR_ATTACHMENT_OPTIMAL,
            .aspect = image_aspect::COLOR
        };

        const attachment_reference depthReadWriteReference = {
            .attachment = 1,
            .layout = image_layout::DEPTH_ATTACHMENT_OPTIMAL,
            .aspect = image_aspect::DEPTH
        };
        
        const subpass_description opaqueSubpass = {
            .inputs = {},
            .colors = span(colorTargetReference),
            .resolves = {},
            .depthStencil = depthReadWriteReference,
            .preserveIndices = {},
        };

        const subpass_dependency externalToOpaque = {
            .srcSubpassIndex = VK_SUBPASS_EXTERNAL,
            .dstSubpassIndex = 0,
            .srcStagesMask = pipeline_stage::COLOR_ATTACHMENT_OUTPUT | pipeline_stage::EARLY_FRAGMENT_TESTS,
            .dstStagesMask = pipeline_stage::COLOR_ATTACHMENT_OUTPUT | pipeline_stage::EARLY_FRAGMENT_TESTS,
            .srcAccessMask = access_type::NONE,
            .dstAccessMask = access_type::COLOR_ATTACHMENT_WRITE | access_type::DEPTH_STENCIL_ATTACHMENT_WRITE
        };

        const subpass_dependency opaqueToExternal = {
            .srcSubpassIndex = 0,
            .dstSubpassIndex = VK_SUBPASS_EXTERNAL,
            .srcStagesMask = pipeline_stage::COLOR_ATTACHMENT_OUTPUT,
            .dstStagesMask = pipeline_stage::TOP_OF_PIPE,
            .srcAccessMask = access_type::COLOR_ATTACHMENT_WRITE,
            .dstAccessMask = access_type::NONE
        };

        attachment_description attachments[] = { colorTarget, depthTarget };
        subpass_description subpasses[] = { opaqueSubpass };
        subpass_dependency dependencies[] = { externalToOpaque, opaqueToExternal };

        render_pass_create_info rpci = {
            .attachments = attachments,
            .subpasses = subpasses,
            .dependencies = dependencies,
            .name = "pbr_scene_render_pass"
        };

        const auto res = get_render_manager()->create(rpci);
        if (res)
        {
            _scenePass = *res;
        }
    }

    void pbr_render_pipeline::init_scene_render_target()
    {
        const image_create_info colorImageCi = {
            .type = image_type::TYPE_2D,
			.format = _colorAttachmentFmt,
			.width = _targetWidth,
			.height = _targetHeight,
			.depth = 1,
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = sample_count::COUNT_1,
			.usage = image_usage::COLOR_ATTACHMENT | image_usage::SAMPLED
        };

        const image_create_info depthImageCi = {
            .type = image_type::TYPE_2D,
			.format = _depthAttachmentFmt,
			.width = _targetWidth,
			.height = _targetHeight,
			.depth = 1,
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = sample_count::COUNT_1,
			.usage = image_usage::DEPTH_STENCIL_ATTACHMENT
        };

        const allocation_create_info imgAllocInfo = {
			.required = memory_property::DEVICE_LOCAL,
			.preferred = memory_property::DEVICE_LOCAL,
			.usage = memory_usage::PREFER_DEVICE,
			.hostSequentialWriteAccess = false,
			.hostRandomAccess = false
		};

        const auto colorImageRes = get_render_manager()->create(colorImageCi, imgAllocInfo);
        const auto depthImageRes = get_render_manager()->create(depthImageCi, imgAllocInfo);

        if (colorImageRes && depthImageRes)
        {
            const image_view_create_info colorViewCi = {
                .usage = image_view_usage{
                    .usage = image_usage::COLOR_ATTACHMENT | image_usage::SAMPLED,
                },
                .img = *colorImageRes,
                .type = image_view_type::TYPE_2D,
                .fmt = colorImageCi.format,
                .subresource = {
                    .aspect = image_aspect::COLOR,
                    .baseMipLevel = 0,
                    .mipLevelCount = 1,
                    .baseLayer = 0,
                    .layerCount = 1
                }
            };

            const image_view_create_info depthViewCi = {
                .usage = image_view_usage{
                    .usage = image_usage::DEPTH_STENCIL_ATTACHMENT,
                },
                .img = *depthImageRes,
                .type = image_view_type::TYPE_2D,
                .fmt = depthImageCi.format,
                .subresource = {
                    .aspect = image_aspect::DEPTH,
                    .baseMipLevel = 0,
                    .mipLevelCount = 1,
                    .baseLayer = 0,
                    .layerCount = 1
                }
            };

            const auto colorViewRes = get_render_manager()->create(colorViewCi);
            const auto depthViewRes = get_render_manager()->create(depthViewCi);

            if (colorViewRes && depthViewRes)
            {
                _colorTarget = {
                    .img = *colorImageRes,
                    .view = *colorViewRes
                };

                _depthTarget = {
                    .img = *depthImageRes,
                    .view = *depthViewRes
                };

                const image_view attachments[] = { *colorViewRes, *depthViewRes };

                frame_buffer_create_info fboInfo = {
                    .attachments = attachments,
                    .pass = _scenePass,
                    .width = _targetWidth,
                    .height = _targetHeight,
                    .layers = 1,
                    .name = "pbr_scene_render_target"
                };

                const auto fboResult = get_render_manager()->create(fboInfo);
                if (fboResult)
                {
                    _sceneRenderTarget = *fboResult;
                }
            }
        }
    }

    void pbr_render_pipeline::init_scene_layout()
    {
        descriptor_set_layout_binding sceneCamera = {
            .binding = 0,
            .type = descriptor_type::UNIFORM_BUFFER,
            .count = 1,
            .stages = shader_stage::VERTEX | shader_stage::FRAGMENT
        };

        descriptor_set_layout_binding sceneData = {
            .binding = 1,
            .type = descriptor_type::STORAGE_BUFFER,
            .count = 1,
            .stages = shader_stage::FRAGMENT
        };

        descriptor_set_layout_binding instanceData = {
            .binding = 0,
            .type = descriptor_type::DYNAMIC_STORAGE_BUFFER,
            .count = 1,
            .stages = shader_stage::VERTEX | shader_stage::FRAGMENT
        };

        descriptor_set_layout_binding materialData = {
            .binding = 1,
            .type = descriptor_type::DYNAMIC_STORAGE_BUFFER,
            .count = 1,
            .stages = shader_stage::FRAGMENT
        };

        descriptor_set_layout_binding textures = {
            .binding = 2,
            .type = descriptor_type::COMBINED_IMAGE_SAMPLER,
            .count = _maxTextures,
            .stages = shader_stage::FRAGMENT
        };

        const descriptor_set_layout_binding sceneBindings[] = { sceneCamera, sceneData };
        const descriptor_set_layout_binding drawableBindings[] = { instanceData, materialData, textures };

        const descriptor_set_layout_create_info sceneSetLayoutCi = {
            .bindings = span(sceneBindings)
        };

        const descriptor_set_layout_create_info drawableSetLayoutCi = {
            .bindings = span(drawableBindings)
        };

        auto sceneSetLayoutResult = get_render_manager()->create(sceneSetLayoutCi);
        auto drawableSetLayoutResult = get_render_manager()->create(drawableSetLayoutCi);

        if (sceneSetLayoutResult && drawableSetLayoutResult)
        {
            descriptor_set_layout layouts[] = { *sceneSetLayoutResult, *drawableSetLayoutResult };

            const pipeline_layout_create_info layoutCi = {
                .layouts = span(layouts),
                .name = "pbr_pipeline_layout"
            };

            auto pipelineLayoutResult = get_render_manager()->create(layoutCi);
            if (pipelineLayoutResult)
            {
                _sceneLayout = *pipelineLayoutResult;
                _sceneWideLayout = *sceneSetLayoutResult;
                _drawableLayout = *drawableSetLayoutResult;
            }
        }
        else
        {
            // error out
        }
    }

    void pbr_render_pipeline::initialize_buffers()
    {
        const auto frames = get_render_manager()->get_frames_in_flight();
        const auto instanceBytes = sizeof(gpu_instance_data) * _maxInstances * frames;
        const auto materialBytes = sizeof(gpu_material_data) * _maxMaterials * frames;
        const auto indirectBytes = sizeof(gpu_indirect_call) * _maxDrawCalls * frames;
        const auto drawCallBytes = sizeof(std::uint32_t) * _maxDrawCalls * frames;
        const auto cameraDataBytes = sizeof(scene_camera) * frames;
        const auto sceneDataBytes = sizeof(scene_data) * frames;
        const auto textureCount = _maxTextures * frames;

        const buffer_create_info instanceBufferCi = {
            .size = instanceBytes,
            .usage = buffer_usage::STORAGE
        };

        const buffer_create_info materialBufferCi = {
            .size = materialBytes,
            .usage = buffer_usage::STORAGE
        };

        const buffer_create_info indirectBufferCi = {
            .size = indirectBytes,
            .usage = buffer_usage::INDIRECT | buffer_usage::STORAGE
        };

        const buffer_create_info countBufferCi = {
            .size = drawCallBytes,
            .usage = buffer_usage::INDIRECT | buffer_usage::STORAGE
        };

        const buffer_create_info cameraBufferCi = {
            .size = cameraDataBytes,
            .usage = buffer_usage::UNIFORM
        };

        const buffer_create_info sceneBufferCi = {
            .size = sceneDataBytes,
            .usage = buffer_usage::STORAGE
        };

        const allocation_create_info allocCi = {
            .required = memory_property::HOST_COHERENT | memory_property::HOST_VISIBLE,
            .preferred = memory_property::NONE,
            .usage = memory_usage::PREFER_AUTO_SELECT,
            .hostSequentialWriteAccess = false,
            .hostRandomAccess = true,
            .persistentlyMapped = true
        };

        auto instanceBufferResult = get_render_manager()->create(instanceBufferCi, allocCi);
        auto materialBufferResult = get_render_manager()->create(materialBufferCi, allocCi);
        auto indirectBufferResult = get_render_manager()->create(indirectBufferCi, allocCi);
        auto countBufferResult = get_render_manager()->create(countBufferCi, allocCi);
        auto cameraBufferResult = get_render_manager()->create(cameraBufferCi, allocCi);
        auto sceneBufferResult = get_render_manager()->create(sceneBufferCi, allocCi);

        const auto buffersBuilt = instanceBufferResult && materialBufferResult && indirectBufferResult && countBufferResult && cameraBufferResult && sceneBufferResult;
        assert(buffersBuilt);

        _indirectCommands = *indirectBufferResult;
        _indirectCount = *countBufferResult;
        _materials = *materialBufferResult;
        _instanceData = *instanceBufferResult;
        _cameraData = *cameraBufferResult;
        _sceneData = *sceneBufferResult;
        _textures.resize(textureCount);
        _textureWriteScratchBuffer.resize(textureCount);
    }
    
    void ryujin::pbr_render_pipeline::initialize_textures()
    {
        auto texOptional = get_render_manager()->renderables().try_fetch_texture("internal_illegal_texture");
        if (texOptional)
        {
            _invalidTexture = *texOptional;
        }
        else
        {
            // wat do
        }
    }
}