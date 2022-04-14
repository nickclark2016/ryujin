#include <VkBootstrap.h>

#include "type_converter.hpp"

#include <ryujin/core/as.hpp>
#include <ryujin/core/linear_allocator.hpp>
#include <ryujin/graphics/render_manager.hpp>

#undef APIENTRY
#include <spdlog/spdlog.h>

#include <utility>

#undef NO_ERROR

namespace ryujin
{
    namespace detail
    {
        VkResult name_object(const vkb::DispatchTable& fns, const std::string& name, VkObjectType type, void* handle)
        {
            if (name.empty())
            {
                return VK_INCOMPLETE;
            }

            const VkDebugUtilsObjectNameInfoEXT info = {
                VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                nullptr,
                type,
                reinterpret_cast<std::uint64_t>(handle),
                name.c_str()
            };

            return fns.setDebugUtilsObjectNameEXT(&info);
        }

        enum class vulkan_build_errors
        {
            SUCCESS,
            OUT_OF_MEMORY
        };

        template <std::size_t N>
        result<VkWriteDescriptorSet, vulkan_build_errors> build_write(const descriptor_write_info& info, inline_linear_allocator<N>& allocator)
        {
            auto bufs = std::get_if<span<descriptor_buffer_info>>(&info.info);
            auto imgs = std::get_if<span<descriptor_image_info>>(&info.info);

            auto bufInfos = allocator.template typed_allocate<VkDescriptorBufferInfo>(bufs ? bufs->length() : 0);
            auto imgInfos = allocator.template typed_allocate<VkDescriptorImageInfo>(imgs ? imgs->length() : 0);

            if (bufInfos == nullptr && imgInfos == nullptr)
            {
                return result<VkWriteDescriptorSet, vulkan_build_errors>::from_error(vulkan_build_errors::OUT_OF_MEMORY);
            }

            auto count = bufs ? bufs->length() : (imgs ? imgs->length() : 0);

            if (bufInfos)
            {
                for (std::size_t i = 0; i < count; ++i)
                {
                    bufInfos[i] = {
                        .buffer = (*bufs)[i].buf.buffer,
                        .offset = (*bufs)[i].offset,
                        .range = (*bufs)[i].length
                    };
                }
            }
            else if (imgInfos)
            {
                for (std::size_t i = 0; i < count; ++i)
                {
                    imgInfos[i] = {
                        .sampler = (*imgs)[i].sam,
                        .imageView = (*imgs)[i].view,
                        .imageLayout = to_vulkan((*imgs)[i].layout),
                    };
                }
            }

            const VkWriteDescriptorSet write = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = nullptr,
                .dstSet = info.set,
                .dstBinding = info.binding,
                .dstArrayElement = info.element,
                .descriptorCount = as<std::uint32_t>(count),
                .descriptorType = to_vulkan(info.type),
                .pImageInfo = imgInfos,
                .pBufferInfo = bufInfos,
                .pTexelBufferView = nullptr
            };

            return result<VkWriteDescriptorSet, vulkan_build_errors>::from_success(write);
        }
    }

    result<std::unique_ptr<render_manager>, render_manager::error_code> render_manager::create(const std::unique_ptr<window>& win, vkb::Instance instance, vkb::Device device, VmaAllocator allocator, const bool nameObjects, registry& reg)
    {
        using result_type = result<std::unique_ptr<render_manager>, error_code>;

        std::unique_ptr<render_manager> manager(new render_manager(win, instance, device, allocator, nameObjects, &reg));

        const auto surfaceResult = manager->create_surface();
        if (surfaceResult != error_code::NO_ERROR)
        {
            return result_type::from_error(surfaceResult);
        }

        const auto swapchainResult = manager->create_swapchain();
        if (swapchainResult != error_code::NO_ERROR)
        {
            return result_type::from_error(swapchainResult);
        }

        const auto queuesResult = manager->fetch_queues();
        if (queuesResult != error_code::NO_ERROR)
        {
            return result_type::from_error(queuesResult);
        }

        const auto perFrameInFlightResourceResult = manager->build_resources_per_frame_in_flight();
        if (perFrameInFlightResourceResult != error_code::NO_ERROR)
        {
            return result_type::from_error(perFrameInFlightResourceResult);
        }

        const auto buildingBufferResults = manager->build_staging_buffers();
        if (buildingBufferResults != error_code::NO_ERROR)
        {
            return result_type::from_error(buildingBufferResults);
        }

        manager->_renderer.set_render_manager(manager.get());

        return result_type::from_success(std::move(manager));
    }

    render_manager::~render_manager()
    {
        release_resources();
    }

    render_manager::error_code render_manager::pre_render()
    {
        if (_isMinimized)
        {
            return error_code::NO_ERROR;
        }

        _renderer.pre_render();
        return error_code::NO_ERROR;
    }

    render_manager::error_code render_manager::render()
    {
        if (_isMinimized)
        {
            return error_code::NO_ERROR;
        }

        _funcs.waitForFences(1, &(get_current_frame_resources().renderFence), VK_TRUE, UINT64_MAX);
        _funcs.resetFences(1, &(get_current_frame_resources().renderFence));

        reset(get_current_frame_resources().graphics);
        reset(get_current_frame_resources().transfer);
        reset(get_current_frame_resources().compute);

        auto list = next_graphics_command_list();
        auto cmdBuffer = list._buffer;

        // reset the staging buffers
        for (auto& b : _stagingBuffers)
        {
            b.offset = 0;
#ifdef _DEBUG
            memset(b.buf.info.pMappedData, 0, b.buf.info.size);
#endif
        }

        VkAcquireNextImageInfoKHR imageAcquireInfo = {
            VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR,
            nullptr,
            _swapchain.swapchain,
            UINT64_MAX,
            get_current_frame_resources().present,
            VK_NULL_HANDLE,
            1
        };

        const auto imageAcquireResult = _funcs.acquireNextImage2KHR(&imageAcquireInfo, &_swapchainImageIndex);
        if (imageAcquireResult == VK_ERROR_OUT_OF_DATE_KHR)
        {
            return recreate_swapchain();
        }
        else if (imageAcquireResult != VK_SUCCESS)
        {
            // failed to acquire image
            return error_code::SWAPCHAIN_IMAGE_ACQUISITION_FAILURE;
        }

        get_current_frame_resources().dtorQueue.flush();
        get_current_frame_resources().descriptorAllocator.reset();

        _renderer.render();

        auto submitSignal = render_complete_signal();

        VkPresentInfoKHR presentInfo = {
            VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            nullptr,
            1,
            &submitSignal,
            1,
            &_swapchain.swapchain,
            &_swapchainImageIndex,
            nullptr
        };

        const auto presentResult = _funcs.queuePresentKHR(_present, &presentInfo);
        if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR)
        {
            recreate_swapchain();
        }
        else if (presentResult != VK_SUCCESS)
        {
            spdlog::error("Presentation to swapchain failed.");
            return error_code::PRESENT_FAILURE;
        }
        

        _currentFrame = (_currentFrame + 1) % _framesInFlight;

        return error_code::NO_ERROR;
    }

    std::uint32_t render_manager::get_swapchain_image_count() const noexcept
    {
        return static_cast<std::uint32_t>(_swapchainImages.size());
    }

    result<image_view, render_manager::error_code> render_manager::get_swapchain_image(const std::uint32_t index) const noexcept
    {
        if (index >= get_swapchain_image_count())
        {
            return result<image_view, error_code>::from_error(error_code::ILLEGAL_ARGUMENT);
        }
        return result<image_view, error_code>::from_success(_swapchainImages[index]);
    }

    image_view render_manager::get_swapchain_image() const noexcept
    {
        return _swapchainImages[_swapchainImageIndex];
    }

    data_format render_manager::get_swapchain_format() const noexcept
    {
        return as<data_format>(_swapchain.image_format);
    }

    std::uint32_t render_manager::get_swapchain_width() const noexcept
    {
        return _swapchain.extent.width;
    }

    std::uint32_t render_manager::get_swapchain_height() const noexcept
    {
        return _swapchain.extent.height;
    }

    result<buffer, render_manager::error_code> render_manager::create(const buffer_create_info& bufferInfo, const allocation_create_info& allocInfo)
    {
        buffer buf = {};

        const VkBufferCreateInfo cinfo = {
            VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            nullptr,
            0,
            bufferInfo.size,
            to_vulkan(bufferInfo.usage),
            VK_SHARING_MODE_EXCLUSIVE,
            0,
            nullptr
        };

        const VmaAllocationCreateInfo ainfo = {
            as<VmaAllocationCreateFlags>((allocInfo.hostSequentialWriteAccess ? VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT : 0u)
                | (allocInfo.hostRandomAccess ? VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT : 0u)
                | (allocInfo.persistentlyMapped ? VMA_ALLOCATION_CREATE_MAPPED_BIT : 0u)),
            to_vma(allocInfo.usage),
            to_vulkan(allocInfo.required),
            to_vulkan(allocInfo.preferred),
            0,
            VK_NULL_HANDLE,
            VK_NULL_HANDLE,
            0.0f
        };

        const auto res = vmaCreateBuffer(_allocator, &cinfo, &ainfo, &buf.buffer, &buf.allocation, &buf.info);
        if (res != VK_SUCCESS)
        {
            return result<buffer, error_code>::from_error(error_code::RESOURCE_ALLOCATION_FAILURE);
        }

        _buffers.push_back(buf);

        return result<buffer, error_code>::from_success(buf);
    }

    result<descriptor_pool, render_manager::error_code> render_manager::create(const descriptor_pool_create_info& info)
    {
        linear_allocator_lock lk(_inlineScratchBuffer);

        descriptor_pool pool = {};

        auto sizes = _inlineScratchBuffer.typed_allocate<VkDescriptorPoolSize>(info.sizes.length());
        for (size_t i = 0; i < info.sizes.length(); ++i)
        {
            sizes[i] = {
                .type = to_vulkan(info.sizes[i].type),
                .descriptorCount = info.sizes[i].count
            };
        }

        const VkDescriptorPoolCreateInfo cinfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .maxSets = info.maxSetCount,
            .poolSizeCount = as<std::uint32_t>(info.sizes.length()),
            .pPoolSizes = sizes
        };

        const auto res = _funcs.createDescriptorPool(&cinfo, get_allocation_callbacks(), &pool);
        switch (res)
        {
        case VK_SUCCESS:
            return result<descriptor_pool, error_code>::from_success(pool);
        case VK_ERROR_FRAGMENTED_POOL:
        case VK_ERROR_OUT_OF_POOL_MEMORY:
            return result<descriptor_pool, error_code>::from_error(error_code::RESOURCE_ALLOCATION_FROM_POOL_FAILURE);
        default:
            return result<descriptor_pool, error_code>::from_error(error_code::RESOURCE_ALLOCATION_FAILURE);
        }
    }

    result<descriptor_set_layout, render_manager::error_code> render_manager::create(const descriptor_set_layout_create_info& info)
    {
        const auto res = _descriptorLayoutCache.allocate(info);
        if (res)
        {
            return result<descriptor_set_layout, error_code>::from_success(*res);
        }
        else
        {
            return result<descriptor_set_layout, error_code>::from_error(error_code::RESOURCE_ALLOCATION_FAILURE);
        }
    }

    result<descriptor_set_layout, render_manager::error_code> render_manager::_create(const descriptor_set_layout_create_info& info)
    {
        linear_allocator_lock lk(_inlineScratchBuffer);

        descriptor_set_layout layout = {};

        auto bindings = _inlineScratchBuffer.typed_allocate<VkDescriptorSetLayoutBinding>(info.bindings.length());
        for (std::size_t i = 0; i < info.bindings.length(); ++i)
        {
            auto& binding = info.bindings[i];

            bindings[i] = {
                binding.binding,
                to_vulkan(binding.type),
                binding.count,
                as<VkShaderStageFlags>(to_vulkan(binding.stages)),
                nullptr
            };
        }

        const VkDescriptorSetLayoutCreateInfo cinfo = {
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            VK_NULL_HANDLE,
            0,
            as<std::uint32_t>(info.bindings.length()),
            bindings
        };

        const auto res = _funcs.createDescriptorSetLayout(&cinfo, get_allocation_callbacks(), &layout);
        if (res != VK_SUCCESS)
        {
            return result<descriptor_set_layout, error_code>::from_error(error_code::RESOURCE_ALLOCATION_FAILURE);
        }

        _descriptorLayouts.push_back(layout);

        return result<descriptor_set_layout, error_code>::from_success(layout);
    }

    result<fence, render_manager::error_code> render_manager::create(const fence_create_info& info)
    {
        const VkFenceCreateInfo cinfo = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = VK_NULL_HANDLE,
            .flags = as<VkFenceCreateFlags>(info.signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0ul)
        };

        fence f = {};
        const auto fenceRes = _funcs.createFence(&cinfo, get_allocation_callbacks(), &f);
        if (fenceRes != VK_SUCCESS)
        {
            return result<fence, error_code>::from_error(error_code::RESOURCE_ALLOCATION_FAILURE);
        }

        _fences.push_back(f);

        return result<fence, error_code>::from_success(f);
    }

    result<frame_buffer, render_manager::error_code> render_manager::create(const frame_buffer_create_info& framebufferInfo)
    {
        linear_allocator_lock lk(_inlineScratchBuffer);

        VkFramebufferAttachmentsCreateInfo* imagelessAttachments = nullptr;
        VkImageView* imageAttachments = nullptr;
        std::uint32_t attachmentCount = 0;

        if (std::holds_alternative<frame_buffer_attachment_create_info>(framebufferInfo.attachments))
        {
            auto& attachments = std::get<frame_buffer_attachment_create_info>(framebufferInfo.attachments);
            imagelessAttachments = _inlineScratchBuffer.typed_allocate<VkFramebufferAttachmentsCreateInfo>(1);
            imagelessAttachments->sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO;
            imagelessAttachments->pNext = nullptr;
            imagelessAttachments->attachmentImageInfoCount = as<std::uint32_t>(attachments.infos.length());
            auto vkAttachments = _inlineScratchBuffer.typed_allocate<VkFramebufferAttachmentImageInfo>(attachments.infos.length());
            for (size_t i = 0; i < imagelessAttachments->attachmentImageInfoCount; ++i)
            {
                const auto formatCount = as<std::uint32_t>(attachments.infos[i].formats.length());
                auto formats = _inlineScratchBuffer.typed_allocate<VkFormat>(formatCount);

                for (size_t j = 0; j < formatCount; ++j)
                {
                    formats[j] = to_vulkan(attachments.infos[i].formats[j]);
                }

                vkAttachments[i] = {
                    VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO,
                    nullptr,
                    0,
                    to_vulkan(attachments.infos[i].usage),
                    framebufferInfo.width,
                    framebufferInfo.height,
                    framebufferInfo.layers,
                    formatCount,
                    formats
                };
            }
            imagelessAttachments->pAttachmentImageInfos = vkAttachments;
            attachmentCount = imagelessAttachments->attachmentImageInfoCount;
        }
        else if (std::holds_alternative<span<image_view>>(framebufferInfo.attachments))
        {
            auto& attachments = std::get<span<image_view>>(framebufferInfo.attachments);
            auto vkAttachments = _inlineScratchBuffer.typed_allocate<VkImageView>(attachments.length());

            for (std::size_t i = 0; i < attachments.length(); ++i)
            {
                vkAttachments[i] = attachments[i];
            }

            imageAttachments = vkAttachments;
            attachmentCount = as<std::uint32_t>(attachments.length());
        }

        VkFramebufferCreateInfo cinfo = {};
        cinfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        cinfo.pNext = imagelessAttachments;
        cinfo.flags = imagelessAttachments ? VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT : 0;
        cinfo.renderPass = framebufferInfo.pass;
        cinfo.attachmentCount = attachmentCount;
        cinfo.pAttachments = imageAttachments;
        cinfo.width = framebufferInfo.width;
        cinfo.height = framebufferInfo.height;
        cinfo.layers = framebufferInfo.layers;

        frame_buffer buffer = {};
        const auto res = _funcs.createFramebuffer(&cinfo, get_allocation_callbacks(), &buffer);

        if (res != VK_SUCCESS)
        {
            spdlog::error("Failed to create VkFramebuffer: {}", framebufferInfo.name);
            return result<frame_buffer, error_code>::from_error(error_code::RESOURCE_ALLOCATION_FAILURE);
        }

        _frameBuffers.push_back(buffer);

        if (_nameObjects)
        {
            const auto res = detail::name_object(_funcs, framebufferInfo.name, VK_OBJECT_TYPE_FRAMEBUFFER, buffer);
            if (res == VK_INCOMPLETE)
            {
                spdlog::debug("Name for VkRenderPass not provided.");
            }
            else if (res != VK_SUCCESS)
            {
                spdlog::warn("Failed to name VkRenderPass: {}", framebufferInfo.name);
            }
            else
            {
                SPDLOG_TRACE("Successfully named VkRenderPass: {}", framebufferInfo.name);
            }
        }

        spdlog::info("Successfully created VkFramebuffer: {}", framebufferInfo.name);
        return result<frame_buffer, error_code>::from_success(buffer);
    }

    result<image, render_manager::error_code> render_manager::create(const image_create_info& imageInfo, const allocation_create_info& allocInfo)
    {
        image img = {};

        const VkImageCreateInfo info = {
            VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            VK_NULL_HANDLE,
            0,
            to_vulkan(imageInfo.type),
            to_vulkan(imageInfo.format),
            {
                imageInfo.width,
                imageInfo.type == image_type::TYPE_1D ? 1 : imageInfo.height,
                imageInfo.type == image_type::TYPE_1D || imageInfo.type == image_type::TYPE_2D ? 1 : imageInfo.depth
            },
            imageInfo.mipLevels,
            imageInfo.arrayLayers,
            to_vulkan(imageInfo.samples),
            VK_IMAGE_TILING_OPTIMAL,
            to_vulkan(imageInfo.usage),
            VK_SHARING_MODE_EXCLUSIVE,
            0,
            VK_NULL_HANDLE,
            VK_IMAGE_LAYOUT_UNDEFINED
        };

        const VmaAllocationCreateInfo ainfo = {
            as<VmaAllocationCreateFlags>((allocInfo.hostSequentialWriteAccess ? VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT : 0ul)
                | (allocInfo.hostRandomAccess ? VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT : 0ul)),
            to_vma(allocInfo.usage),
            to_vulkan(allocInfo.required),
            to_vulkan(allocInfo.preferred),
            0,
            VK_NULL_HANDLE,
            VK_NULL_HANDLE,
            0.0f
        };

        const auto res = vmaCreateImage(_allocator, &info, &ainfo, &img.image, &img.allocation, &img.info);
        if (res != VK_SUCCESS)
        {
            return result<image, error_code>::from_error(error_code::RESOURCE_ALLOCATION_FAILURE);
        }

        _images.push_back(img);

        return result<image, error_code>::from_success(img);
    }

    result<image_view, render_manager::error_code> render_manager::create(const image_view_create_info& info)
    {
        linear_allocator_lock lk(_inlineScratchBuffer);

        image_view view = {};

        auto imageUsage = info.usage ? _inlineScratchBuffer.typed_allocate<VkImageViewUsageCreateInfo>() : nullptr;
        if (imageUsage)
        {
            *imageUsage = {
                VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO,
                VK_NULL_HANDLE,
                to_vulkan(info.usage->usage)
            };
        }

        const VkImageViewCreateInfo cinfo = {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            imageUsage,
            0,
            info.img.image,
            to_vulkan(info.type),
            to_vulkan(info.fmt),
            {
                VK_COMPONENT_SWIZZLE_IDENTITY,
                VK_COMPONENT_SWIZZLE_IDENTITY,
                VK_COMPONENT_SWIZZLE_IDENTITY,
                VK_COMPONENT_SWIZZLE_IDENTITY
            },
            {
                to_vulkan(info.subresource.aspect),
                info.subresource.baseMipLevel,
                info.subresource.mipLevelCount,
                info.subresource.baseLayer,
                info.subresource.layerCount
            }
        };

        const auto res = _funcs.createImageView(&cinfo, get_allocation_callbacks(), &view);
        if (res != VK_SUCCESS)
        {
            return result<image_view, error_code>::from_error(error_code::RESOURCE_ALLOCATION_FAILURE);
        }

        _imageViews.push_back(view);

        return result<image_view, error_code>::from_success(view);
    }

    result<pipeline, render_manager::error_code> render_manager::create(const graphics_pipeline_create_info& info)
    {
        linear_allocator_lock lk(_inlineScratchBuffer);

        auto stages = _inlineScratchBuffer.typed_allocate<VkPipelineShaderStageCreateInfo>(info.stages.length());
        for (size_t i = 0; i < info.stages.length(); ++i)
        {
            stages[i] = {
                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                nullptr,
                0,
                to_vulkan(info.stages[i].stage),
                info.stages[i].mod,
                "main",
                nullptr
            };
        }

        auto vertexInput = _inlineScratchBuffer.typed_allocate<VkPipelineVertexInputStateCreateInfo>(1);
        auto vertexAttributes = _inlineScratchBuffer.typed_allocate<VkVertexInputAttributeDescription>(info.vertexInput.attributes.length());
        auto vertexBindings = _inlineScratchBuffer.typed_allocate<VkVertexInputBindingDescription>(info.vertexInput.bindings.length());

        for (size_t i = 0; i < info.vertexInput.attributes.length(); ++i)
        {
            auto& attr = info.vertexInput.attributes[i];
            vertexAttributes[i] = {
                attr.location,
                attr.binding,
                to_vulkan(attr.format),
                attr.offset
            };
        }

        for (size_t i = 0; i < info.vertexInput.bindings.length(); ++i)
        {
            auto& binding = info.vertexInput.bindings[i];
            vertexBindings[i] = {
                binding.binding,
                binding.stride,
                to_vulkan(binding.rate)
            };
        }

        *vertexInput = {
            VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            nullptr,
            0,
            as<std::uint32_t>(info.vertexInput.bindings.length()),
            vertexBindings,
            as<std::uint32_t>(info.vertexInput.attributes.length()),
            vertexAttributes
        };

        auto inputAssemblyState = _inlineScratchBuffer.typed_allocate<VkPipelineInputAssemblyStateCreateInfo>();
        *inputAssemblyState = {
            VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            nullptr,
            0,
            to_vulkan(info.inputAssembly.topology),
            info.inputAssembly.restartEnabled ? VK_TRUE : VK_FALSE
        };

        auto tessellationState = nullptr;

        auto viewportState = _inlineScratchBuffer.typed_allocate<VkPipelineViewportStateCreateInfo>();
        auto viewports = _inlineScratchBuffer.typed_allocate<VkViewport>(info.viewport.viewports.length());
        auto scissors = _inlineScratchBuffer.typed_allocate<VkRect2D>(info.viewport.scissors.length());

        for (size_t i = 0; i < info.viewport.viewports.length(); ++i)
        {
            auto vp = info.viewport.viewports[i];
            viewports[i] = {
                vp.x,
                vp.y,
                vp.width,
                vp.height,
                vp.minDepth,
                vp.maxDepth
            };
        }

        for (size_t i = 0; i < info.viewport.scissors.length(); ++i)
        {
            auto scissor = info.viewport.scissors[i];
            scissors[i] = {
                { scissor.x, scissor.y },
                { scissor.width, scissor.height }
            };
        }

        *viewportState = {
            VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            nullptr,
            0,
            as<std::uint32_t>(info.viewport.viewports.length()),
            viewports,
            as<std::uint32_t>(info.viewport.scissors.length()),
            scissors
        };

        auto rasterizationState = _inlineScratchBuffer.typed_allocate<VkPipelineRasterizationStateCreateInfo>();
        *rasterizationState = {
            VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            nullptr,
            0,
            info.rasterizationState.depthClampEnabled ? VK_TRUE : VK_FALSE,
            info.rasterizationState.rasterizerDiscardEnabled ? VK_TRUE : VK_FALSE,
            to_vulkan(info.rasterizationState.poly),
            to_vulkan(info.rasterizationState.cull),
            to_vulkan(info.rasterizationState.front),
            info.rasterizationState.depthBias.has_value(),
            info.rasterizationState.depthBias.has_value() ? info.rasterizationState.depthBias->constant : 0.0f,
            info.rasterizationState.depthBias.has_value() ? info.rasterizationState.depthBias->clamp : 0.0f,
            info.rasterizationState.depthBias.has_value() ? info.rasterizationState.depthBias->slope : 0.0f,
            info.rasterizationState.lineWidth
        };

        auto multisampleState = _inlineScratchBuffer.typed_allocate<VkPipelineMultisampleStateCreateInfo>();
        *multisampleState = {
            VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            nullptr,
            0,
            to_vulkan(info.multisampleState.samples),
            info.multisampleState.enableSampleShading ? VK_TRUE : VK_FALSE,
            info.multisampleState.minSampleShading,
            info.multisampleState.sampleMask.data(),
            info.multisampleState.alphaToCoverageEnabled,
            info.multisampleState.alphaToOneEnabled
        };

        auto depthStencil = info.depthStencilState.has_value() ? _inlineScratchBuffer.typed_allocate<VkPipelineDepthStencilStateCreateInfo>() : nullptr;
        if (depthStencil != nullptr)
        {
            const depth_test_state defaultDepthTestState{ false, compare_op::NEVER };
            const depth_bounds_state defaultDepthBoundsState{ 0.0f, 1.0f };
            const stencil_test_state defaultStencilOpState{ stencil_op::KEEP, stencil_op::KEEP, stencil_op::KEEP, compare_op::NEVER, 0, 0, 0 };

            *depthStencil = {
                VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
                nullptr,
                0,
                info.depthStencilState->depthTest.has_value() ? VK_TRUE : VK_FALSE,
                info.depthStencilState->depthTest.value_or(defaultDepthTestState).writeEnabled ? VK_TRUE : VK_FALSE,
                to_vulkan(info.depthStencilState->depthTest.value_or(defaultDepthTestState).depthCompareOp),
                info.depthStencilState->depthBoundsTest.has_value() ? VK_TRUE : VK_FALSE,
                info.depthStencilState->stencilTest.has_value(),
                {
                    to_vulkan(info.depthStencilState->stencilTest.value_or(defaultStencilOpState).fail),
                    to_vulkan(info.depthStencilState->stencilTest.value_or(defaultStencilOpState).pass),
                    to_vulkan(info.depthStencilState->stencilTest.value_or(defaultStencilOpState).depthFail),
                    to_vulkan(info.depthStencilState->stencilTest.value_or(defaultStencilOpState).compare),
                    info.depthStencilState->stencilTest.value_or(defaultStencilOpState).compareMask,
                    info.depthStencilState->stencilTest.value_or(defaultStencilOpState).writeMask,
                    info.depthStencilState->stencilTest.value_or(defaultStencilOpState).reference
                },
                {
                    to_vulkan(info.depthStencilState->stencilTest.value_or(defaultStencilOpState).fail),
                    to_vulkan(info.depthStencilState->stencilTest.value_or(defaultStencilOpState).pass),
                    to_vulkan(info.depthStencilState->stencilTest.value_or(defaultStencilOpState).depthFail),
                    to_vulkan(info.depthStencilState->stencilTest.value_or(defaultStencilOpState).compare),
                    info.depthStencilState->stencilTest.value_or(defaultStencilOpState).compareMask,
                    info.depthStencilState->stencilTest.value_or(defaultStencilOpState).writeMask,
                    info.depthStencilState->stencilTest.value_or(defaultStencilOpState).reference
                },
                info.depthStencilState->depthBoundsTest.value_or(defaultDepthBoundsState).minDepthBounds,
                info.depthStencilState->depthBoundsTest.value_or(defaultDepthBoundsState).maxDepthBounds
            };
        }

        auto colorBlendState = _inlineScratchBuffer.typed_allocate<VkPipelineColorBlendStateCreateInfo>();
        auto attachments = _inlineScratchBuffer.typed_allocate<VkPipelineColorBlendAttachmentState>(info.blendState.attachments.length());

        for (std::size_t i = 0; i < info.blendState.attachments.length(); ++i)
        {
            auto& attachment = info.blendState.attachments[i];

            attachments[i] = {
                attachment.enabled ? VK_TRUE : VK_FALSE,
                to_vulkan(attachment.rgb.src),
                to_vulkan(attachment.rgb.dst),
                to_vulkan(attachment.rgb.op),
                to_vulkan(attachment.alpha.src),
                to_vulkan(attachment.alpha.dst),
                to_vulkan(attachment.alpha.op),
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
            };
        }

        *colorBlendState = {
            VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            nullptr,
            0,
            info.blendState.logicOp.has_value() ? VK_TRUE : VK_FALSE,
            to_vulkan(info.blendState.logicOp.value_or(logic_op::CLEAR)),
            as<std::uint32_t>(info.blendState.attachments.length()),
            attachments,
            {
                info.blendState.blendConstants[0],
                info.blendState.blendConstants[1],
                info.blendState.blendConstants[2],
                info.blendState.blendConstants[3]
            }
        };

        auto dynamicState = _inlineScratchBuffer.typed_allocate<VkPipelineDynamicStateCreateInfo>();
        auto states = _inlineScratchBuffer.typed_allocate<VkDynamicState>(info.dynamicStates.length());

        for (std::size_t i = 0; i < info.dynamicStates.length(); ++i)
        {
            states[i] = to_vulkan(info.dynamicStates[i]);
        }

        *dynamicState = {
            VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            nullptr,
            0,
            as<std::uint32_t>(info.dynamicStates.length()),
            states
        };

        const VkGraphicsPipelineCreateInfo cinfo = {
            VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            nullptr,
            0,
            as<std::uint32_t>(info.stages.length()),
            stages,
            vertexInput,
            inputAssemblyState,
            tessellationState,
            viewportState,
            rasterizationState,
            multisampleState,
            depthStencil,
            colorBlendState,
            dynamicState,
            info.layout,
            info.pass,
            info.subpass,
            VK_NULL_HANDLE,
            0
        };

        pipeline pipe = {};

        const auto res = _funcs.createGraphicsPipelines(VK_NULL_HANDLE, 1, &cinfo, get_allocation_callbacks(), &pipe);
        if (res != VK_SUCCESS)
        {
            return result<pipeline, error_code>::from_error(error_code::RESOURCE_ALLOCATION_FAILURE);
        }

        _pipelines.push_back(pipe);

        return result<pipeline, error_code>::from_success(pipe);
    }

    result<pipeline_layout, render_manager::error_code> render_manager::create(const pipeline_layout_create_info& info)
    {
        pipeline_layout layout = {};

        const VkPipelineLayoutCreateInfo cinfo = {
            VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            nullptr,
            0,
            as<std::uint32_t>(info.layouts.length()),
            info.layouts.data(),
            0,
            nullptr
        };

        const auto res = _funcs.createPipelineLayout(&cinfo, get_allocation_callbacks(), &layout);
        if (res != VK_SUCCESS)
        {
            spdlog::error("Failed to create VkPipelineLayout: {}", info.name);
            return result<pipeline_layout, error_code>::from_error(error_code::RESOURCE_ALLOCATION_FAILURE);
        }

        _layouts.push_back(layout);

        spdlog::info("Successfully created VkPipelineLayout: {}", info.name);
        if (_nameObjects)
        {
            const auto res = detail::name_object(_funcs, info.name, VK_OBJECT_TYPE_PIPELINE_LAYOUT, layout);
            if (res == VK_INCOMPLETE)
            {
                spdlog::debug("Name for VkPipelineLayout not provided.");
            }
            else if (res != VK_SUCCESS)
            {
                spdlog::warn("Failed to name VkPipelineLayout: {}", info.name);
            }
            else
            {
                SPDLOG_TRACE("Successfully named VkPipelineLayout: {}", info.name);
            }
        }

        return result<pipeline_layout, error_code>::from_success(layout);
    }

    result<render_pass, render_manager::error_code> render_manager::create(const render_pass_create_info& info)
    {
        linear_allocator_lock ln(_inlineScratchBuffer);

        VkRenderPassCreateInfo2 create = {};
        create.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2;
        create.attachmentCount = as<std::uint32_t>(info.attachments.length());
        create.subpassCount = as<std::uint32_t>(info.subpasses.length());
        create.dependencyCount = as<std::uint32_t>(info.dependencies.length());

        auto attachments = _inlineScratchBuffer.typed_allocate<VkAttachmentDescription2>(create.attachmentCount);
        auto subpasses = _inlineScratchBuffer.typed_allocate<VkSubpassDescription2>(create.subpassCount);
        auto dependencies = _inlineScratchBuffer.typed_allocate<VkSubpassDependency2>(create.dependencyCount);

        for (std::size_t i = 0; i < create.attachmentCount; ++i)
        {
            VkAttachmentDescription2 attachment = {
                VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
                nullptr,
                0,
                to_vulkan(info.attachments[i].format),
                to_vulkan(info.attachments[i].samples),
                to_vulkan(info.attachments[i].load),
                to_vulkan(info.attachments[i].store),
                to_vulkan(info.attachments[i].stencilLoad),
                to_vulkan(info.attachments[i].stencilStore),
                to_vulkan(info.attachments[i].initialLayout),
                to_vulkan(info.attachments[i].finalLayout)
            };

            attachments[i] = attachment;
        }

        const auto to_reference = [this](const span<attachment_reference>& refs)
        {
            const auto len = refs.length();
            const auto data = len > 0 ? _inlineScratchBuffer.typed_allocate<VkAttachmentReference2>(len) : nullptr;

            for (size_t i = 0; i < len; ++i)
            {
                data[i] = {
                    VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
                    nullptr,
                    refs[i].attachment,
                    to_vulkan(refs[i].layout),
                    to_vulkan(refs[i].aspect)
                };
            }

            return data;
        };

        for (std::size_t i = 0; i < create.subpassCount; ++i)
        {
            const auto& sp = info.subpasses[i];

            VkSubpassDescription2 subpass = {
                VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2,
                nullptr,
                0,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                0,
                as<std::uint32_t>(sp.inputs.length()),
                to_reference(sp.inputs),
                as<std::uint32_t>(sp.colors.length()),
                to_reference(sp.colors),
                to_reference(sp.resolves),
                to_reference(span(sp.depthStencil)),
                as<std::uint32_t>(sp.preserveIndices.length()),
                sp.preserveIndices.data()
            };

            subpasses[i] = subpass;
        }

        for (std::size_t i = 0; i < create.dependencyCount; ++i)
        {
            const auto& dep = info.dependencies[i];

            VkSubpassDependency2 dependency = {
                VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
                nullptr,
                dep.srcSubpassIndex,
                dep.dstSubpassIndex,
                to_vulkan(dep.srcStagesMask),
                to_vulkan(dep.dstStagesMask),
                to_vulkan(dep.srcAccessMask),
                to_vulkan(dep.dstAccessMask),
                0,
                0
            };

            dependencies[i] = dependency;
        }

        create.pAttachments = attachments;
        create.pSubpasses = subpasses;
        create.pDependencies = dependencies;

        render_pass pass = {};
        const auto res = _funcs.createRenderPass2(&create, get_allocation_callbacks(), &pass);

        if (res != VK_SUCCESS)
        {
            spdlog::error("Failed to create VkRenderpass: {}", info.name);
            return result<render_pass, error_code>::from_error(error_code::RESOURCE_ALLOCATION_FAILURE);
        }

        _renderPasses.push_back(pass);

        spdlog::info("Successfully created VkRenderpass: {}", info.name);
        if (_nameObjects)
        {
            const auto res = detail::name_object(_funcs, info.name, VK_OBJECT_TYPE_RENDER_PASS, pass);
            if (res == VK_INCOMPLETE)
            {
                spdlog::debug("Name for VkRenderPass not provided.");
            }
            else if (res != VK_SUCCESS)
            {
                spdlog::warn("Failed to name VkRenderPass: {}", info.name);
            }
            else
            {
                SPDLOG_TRACE("Successfully named VkRenderPass: {}", info.name);
            }
        }

        return result<render_pass, error_code>::from_success(pass);
    }

    result<image_sampler, render_manager::error_code> render_manager::create(const sampler_create_info& info)
    {
        image_sampler sam = {};

        const VkSamplerCreateInfo sinfo = {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .magFilter = to_vulkan(info.mag),
            .minFilter = to_vulkan(info.min),
            .mipmapMode = to_vulkan(info.mipmapMode),
            .addressModeU = to_vulkan(info.u),
            .addressModeV = to_vulkan(info.v),
            .addressModeW = to_vulkan(info.w),
            .mipLodBias = info.mipLodBias,
            .anisotropyEnable = info.enableAnisotropy ? VK_TRUE : VK_FALSE,
            .maxAnisotropy = info.maxAnisotropy,
            .compareEnable = info.compare.has_value() ? VK_TRUE : VK_FALSE,
            .minLod = info.minLod,
            .maxLod = info.maxLod,
            .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            .unnormalizedCoordinates = info.unnormalizedCoordinates ? VK_TRUE : VK_FALSE
        };

        const auto res = _funcs.createSampler(&sinfo, get_allocation_callbacks(), &sam);
        if (res != VK_SUCCESS)
        {
            spdlog::error("Failed to create VkSampler: {}", info.name);
            return result<image_sampler, error_code>::from_error(error_code::RESOURCE_ALLOCATION_FAILURE);
        }

        _samplers.push_back(sam);

        spdlog::info("Successfully created VkSampler: {}", info.name);
        if (_nameObjects)
        {
            const auto res = detail::name_object(_funcs, info.name, VK_OBJECT_TYPE_SAMPLER, sam);
            if (res == VK_INCOMPLETE)
            {
                spdlog::debug("Name for VkSampler not provided.");
            }
            else if (res != VK_SUCCESS)
            {
                spdlog::warn("Failed to name VkSampler: {}", info.name);
            }
            else
            {
                SPDLOG_TRACE("Successfully named VkSampler: {}", info.name);
            }
        }

        return result<image_sampler, error_code>::from_success(sam);
    }

    result<shader_module, render_manager::error_code> render_manager::create(const shader_module_create_info& info)
    {
        shader_module sm = {};

        VkShaderModuleCreateInfo cinfo = {
            VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            nullptr,
            0,
            as<std::uint32_t>(info.bytes.length()),
            reinterpret_cast<const std::uint32_t*>(info.bytes.data())
        };

        const auto res = _funcs.createShaderModule(&cinfo, get_allocation_callbacks(), &sm);
        if (res != VK_SUCCESS)
        {
            return result<shader_module, error_code>::from_error(error_code::RESOURCE_ALLOCATION_FAILURE);
        }

        _shaderModules.push_back(sm);

        return result<shader_module, error_code>::from_success(sm);
    }

    result<descriptor_set, render_manager::error_code> render_manager::allocate_transient(const descriptor_set_layout& layout)
    {
        auto& frameResources = this->get_current_frame_resources();
        const auto res = frameResources.descriptorAllocator.allocate(layout);
        if (res)
        {
            return result<descriptor_set, error_code>::from_success(*res);
        }

        return result<descriptor_set, error_code>::from_error(error_code::RESOURCE_ALLOCATION_FROM_POOL_FAILURE);
    }

    result<buffer_region, render_manager::error_code> render_manager::write_to_staging_buffer(const void* data, const std::size_t bytes)
    {
        auto stagingBuffer = _stagingBuffers[0];
        if (stagingBuffer.offset + bytes > stagingBuffer.size)
        {
            return result<buffer_region, error_code>::from_error(error_code::OUT_OF_MEMORY);
        }

        const buffer_region region = {
            .buf = _stagingBuffers[0].buf,
            .offset = _stagingBuffers[0].offset,
            .range = bytes
        };

#ifdef _RYUJIN_WINDOWS
        memcpy_s(reinterpret_cast<char*>(region.buf.info.pMappedData) + region.offset, bytes, data, bytes);
#else
        memcpy(reinterpret_cast<char*>(region.buf.info.pMappedData) + region.offset, data, bytes);
#endif
        _stagingBuffers[0].offset += bytes;

        return result<buffer_region, error_code>::from_success(region);
    }

    void render_manager::reset_staging_buffer()
    {
        for (auto& buf : _stagingBuffers)
        {
            buf.offset = 0;
        }
    }

    void render_manager::write(const span<descriptor_write_info>& infos)
    {
        linear_allocator_lock lk(_inlineScratchBuffer);

        auto count = infos.length();
        auto infoPtr = _inlineScratchBuffer.typed_allocate<VkWriteDescriptorSet>(count);

        for (size_t i = 0; i < count; ++i)
        {
            auto res = detail::build_write(infos[i], _inlineScratchBuffer);
            if (res)
            {
                infoPtr[i] = *res;
            }
            else
            {
                _funcs.updateDescriptorSets(as<std::uint32_t>(i), infoPtr, 0, nullptr);
                --i;

                _inlineScratchBuffer.reset();
                count -= i;

                infoPtr = _inlineScratchBuffer.typed_allocate<VkWriteDescriptorSet>(count);

                spdlog::warn("Out of memory in scratch buffer. Flushing writes and restarting.");
            }
        }

        _funcs.updateDescriptorSets(as<std::uint32_t>(count), infoPtr, 0, nullptr);
    }

    result<descriptor_set, render_manager::error_code> render_manager::_allocate(const descriptor_set_allocate_info& info)
    {
        descriptor_set set = {};
        
        const VkDescriptorSetAllocateInfo ainfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorPool = info.pool,
            .descriptorSetCount = 1,
            .pSetLayouts = &info.layout
        };

        const auto res = _funcs.allocateDescriptorSets(&ainfo, &set);
        if (res != VK_SUCCESS)
        {
            return result<descriptor_set, error_code>::from_error(error_code::RESOURCE_ALLOCATION_FAILURE);
        }

        return result<descriptor_set, error_code>::from_success(set);
    }

    void render_manager::release(const buffer buf, const bool immediate)
    {
        auto it = std::find(_buffers.begin(), _buffers.end(), buf);
        if (it != _buffers.end())
        {
            _buffers.erase(it);

            // add to deletion queue
            const auto dtor = [this, buf]() {
                vmaDestroyBuffer(_allocator, buf.buffer, buf.allocation);
            };
            if (immediate)
            {
                dtor();
            }
            else
            {
                get_current_frame_resources().dtorQueue.push_deletor(dtor);
            }
        }
    }

    void render_manager::release(const descriptor_pool pool, const bool immediate)
    {
        auto it = std::find(_descriptorPools.begin(), _descriptorPools.end(), pool);
        if (it != _descriptorPools.end())
        {
            _descriptorPools.erase(it);

            // add to deletion queue
            const auto dtor = [this, pool]() {
                _funcs.destroyDescriptorPool(pool, get_allocation_callbacks());
            };
            if (immediate)
            {
                dtor();
            }
            else
            {
                get_current_frame_resources().dtorQueue.push_deletor(dtor);
            }
        }
    }

    void render_manager::release(const descriptor_set_layout layout, const bool immediate)
    {
        auto it = std::find(_descriptorLayouts.begin(), _descriptorLayouts.end(), layout);
        if (it != _descriptorLayouts.end())
        {
            _descriptorLayouts.erase(it);

            // add to deletion queue
            const auto dtor = [this, layout]() {
                _funcs.destroyDescriptorSetLayout(layout, get_allocation_callbacks());
            };
            if (immediate)
            {
                dtor();
            }
            else
            {
                get_current_frame_resources().dtorQueue.push_deletor(dtor);
            }
        }
    }

    void render_manager::release(const fence f, const bool immediate)
    {
        auto it = std::find(_fences.begin(), _fences.end(), f);
        if (it != _fences.end())
        {
            _fences.erase(it);

            // add to deletion queue
            const auto dtor = [this, f]() {
                _funcs.destroyFence(f, get_allocation_callbacks());
            };
            if (immediate)
            {
                dtor();
            }
            else
            {
                get_current_frame_resources().dtorQueue.push_deletor(dtor);
            }
        }
    }

    void render_manager::release(const image_view view, const bool immediate)
    {
        auto it = std::find(_imageViews.begin(), _imageViews.end(), view);
        if (it != _imageViews.end())
        {
            _imageViews.erase(it);

            // add to deletion queue
            const auto dtor = [this, view]() {
                _funcs.destroyImageView(view, get_allocation_callbacks());
            };
            if (immediate)
            {
                dtor();
            }
            else
            {
                get_current_frame_resources().dtorQueue.push_deletor(dtor);
            }
        }
    }

    void render_manager::release(const image img, const bool immediate)
    {
        auto it = std::find(_images.begin(), _images.end(), img);
        if (it != _images.end())
        {
            _images.erase(it);

            // add to deletion queue
            const auto dtor = [this, img]() {
                vmaDestroyImage(_allocator, img.image, img.allocation);
            };
            if (immediate)
            {
                dtor();
            }
            else
            {
                get_current_frame_resources().dtorQueue.push_deletor(dtor);
            }
        }
    }

    void render_manager::release(const frame_buffer fbo, const bool immediate)
    {
        auto it = std::find(_frameBuffers.begin(), _frameBuffers.end(), fbo);
        if (it != _frameBuffers.end())
        {
            _frameBuffers.erase(it);

            // add to deletion queue
            const auto dtor = [this, fbo]() {
                _funcs.destroyFramebuffer(fbo, get_allocation_callbacks());
            };
            if (immediate)
            {
                dtor();
            }
            else
            {
                get_current_frame_resources().dtorQueue.push_deletor(dtor);
            }
        }
    }

    void render_manager::reset(const descriptor_pool pool)
    {
        _funcs.resetDescriptorPool(pool, 0);
    }

    void render_manager::reset(const fence f)
    {
        _funcs.resetFences(1, &f);
    }

    graphics_command_list render_manager::next_graphics_command_list()
    {
        auto& perSwapchainResource = get_current_frame_resources();
        auto& graphicsCachedPool = perSwapchainResource.graphics;

        if (graphicsCachedPool.fetchIndex >= graphicsCachedPool.buffers.size())
        {
            VkCommandBufferAllocateInfo allocInfo = {
                VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                nullptr,
                graphicsCachedPool.pool,
                VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                1
            };

            VkCommandBuffer buf;
            _funcs.allocateCommandBuffers(&allocInfo, &buf);
            graphicsCachedPool.buffers.push_back(buf);
        }

        graphics_command_list commands(graphicsCachedPool.buffers[graphicsCachedPool.fetchIndex], _funcs, graphicsCachedPool.queue, graphicsCachedPool.queueIndex);
        ++graphicsCachedPool.fetchIndex;

        return commands;
    }

    transfer_command_list render_manager::next_transfer_command_list()
    {
        auto& perSwapchainResource = get_current_frame_resources();
        auto& transferCachedPool = perSwapchainResource.transfer;

        if (transferCachedPool.fetchIndex >= transferCachedPool.buffers.size())
        {
            VkCommandBufferAllocateInfo allocInfo = {
                VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                nullptr,
                transferCachedPool.pool,
                VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                1
            };

            VkCommandBuffer buf;
            _funcs.allocateCommandBuffers(&allocInfo, &buf);
            transferCachedPool.buffers.push_back(buf);
        }

        transfer_command_list commands(transferCachedPool.buffers[transferCachedPool.fetchIndex], _funcs, transferCachedPool.queue, transferCachedPool.queueIndex);
        ++transferCachedPool.fetchIndex;

        return commands;
    }

    semaphore render_manager::swapchain_image_ready_signal() const noexcept
    {
        return get_current_frame_resources().present;
    }

    semaphore render_manager::render_complete_signal() const noexcept
    {
        return get_current_frame_resources().render;
    }

    fence render_manager::flight_complete_fence() const noexcept
    {
        return get_current_frame_resources().renderFence;
    }

    renderable_manager& render_manager::renderables() noexcept
    {
        return _renderables;
    }

    void render_manager::wait(const fence& f)
    {
        _funcs.waitForFences(1, &f, VK_TRUE, UINT64_MAX);
    }

    render_manager::render_manager(const std::unique_ptr<window>& win, vkb::Instance instance, vkb::Device device, VmaAllocator allocator, const bool nameObjects, registry* reg)
        : _win(win), _instance(instance), _device(device), _allocator(allocator), _funcs(device.make_table()), _nameObjects(nameObjects), _descriptorLayoutCache(*this),
            _renderables(this, reg)
    {
        auto setMinimized = [this]() { spdlog::info("Render manager notifying minimized."); _isMinimized = true; };
        auto setVisible = [this]() { spdlog::info("Render manager notifying not minimized."); _isMinimized = false; };

        _win->on_iconify(setMinimized);
        _win->on_maximize(setVisible);
        _win->on_restore(setVisible);
    }

    render_manager::error_code render_manager::create_surface()
    {
        VkSurfaceKHR surface;
        GLFWwindow* w = _win->_native;
        const auto surfaceResult = glfwCreateWindowSurface(_instance.instance, w, get_allocation_callbacks(), &surface);
        if (surfaceResult != VK_SUCCESS)
        {
            return error_code::SURFACE_ACQUISITION_FAILURE;
        }

        _surface = surface;
        _device.surface = surface;

        return error_code::NO_ERROR;
    }

    render_manager::error_code render_manager::create_swapchain()
    {
        vkb::SwapchainBuilder swapchainBuilder(_device, _surface);

        const auto swapchainResult = swapchainBuilder.build();
        if (!swapchainResult)
        {
            return error_code::SWAPCHAIN_INITIALIZATION_FAILURE;
        }

        auto swapchain = swapchainResult.value();
        const auto imageResults = swapchain.get_images();
        if (imageResults)
        {
            _swapchain = swapchain;
            _swapchainImages.reserve(imageResults->size());
            for (auto image : *imageResults)
            {
                const image_view_usage use = { image_usage::COLOR_ATTACHMENT };
                const image_view_create_info cinfo = {
                    use,
                    { image, nullptr, { } },
                    image_view_type::TYPE_2D,
                    as<data_format>(_swapchain.image_format),
                    {
                        image_aspect::COLOR,
                        0,
                        1,
                        0,
                        1
                    }
                };

                auto viewResult = create(cinfo);
                if (!viewResult)
                {
                    return error_code::SWAPCHAIN_INITIALIZATION_FAILURE;
                }
                _swapchainImages.push_back(viewResult.success());
            }
        }
        else
        {
            error_code::SWAPCHAIN_INITIALIZATION_FAILURE;
        }

        return error_code::NO_ERROR;
    }

    render_manager::error_code render_manager::recreate_swapchain()
    {
        spdlog::debug("Recreating swapchain.");
        _funcs.deviceWaitIdle();

        for (const auto image : _swapchainImages)
        {
            release(image);
        }
        _swapchainImages.clear();

        for (auto& resources : _resourcesPerFrameInFlight)
        {
            resources.dtorQueue.flush();
            resources.descriptorAllocator.reset();
        }

        vkb::SwapchainBuilder swapchainBuilder(_device, _surface);
        if (_swapchain.swapchain)
        {
            spdlog::debug("Found old swapchain to construct with.");
            swapchainBuilder.set_old_swapchain(_swapchain);
        }

        const auto swapchainResult = swapchainBuilder.build();
        if (!swapchainResult)
        {
            return error_code::SWAPCHAIN_INITIALIZATION_FAILURE;
        }

        vkb::destroy_swapchain(_swapchain);

        auto swapchain = swapchainResult.value();
        const auto imageResults = swapchain.get_images();
        if (imageResults)
        {
            _swapchain = swapchain;
            _swapchainImages.reserve(imageResults->size());
            for (auto image : *imageResults)
            {
                const image_view_usage use = { image_usage::COLOR_ATTACHMENT };
                const image_view_create_info cinfo = {
                    use,
                    { image, nullptr, { } },
                    image_view_type::TYPE_2D,
                    as<data_format>(_swapchain.image_format),
                    {
                        image_aspect::COLOR,
                        0,
                        1,
                        0,
                        1
                    }
                };

                auto viewResult = create(cinfo);
                if (!viewResult)
                {
                    return error_code::SWAPCHAIN_INITIALIZATION_FAILURE;
                }
                _swapchainImages.push_back(viewResult.success());
            }
        }
        else
        {
            return error_code::SWAPCHAIN_INITIALIZATION_FAILURE;
        }

        return error_code::NO_ERROR;
    }

    render_manager::error_code render_manager::build_staging_buffers()
    {
        const buffer_create_info cinfo = {
            .size = 256 * 1024 * 1024,
            .usage = buffer_usage::TRANSFER_SRC
        };


        // TODO: Do we really need coherent buffer?
        const allocation_create_info ainfo = {
            .required = memory_property::HOST_COHERENT | memory_property::HOST_VISIBLE,
            .usage = memory_usage::PREFER_DEVICE,
            .hostSequentialWriteAccess = false,
            .hostRandomAccess = true,
            .persistentlyMapped = true
        };

        const auto allocationResult = create(cinfo, ainfo);
        if (!allocationResult)
        {
            return error_code::RESOURCE_ALLOCATION_FAILURE;
        }


        _stagingBuffers[0] = {
            .buf = *allocationResult,
            .offset = 0,
            .size = cinfo.size
        };

        return error_code::NO_ERROR;
    }

    render_manager::error_code render_manager::fetch_queues()
    {
        const auto graphicsQueueResult = _device.get_queue(vkb::QueueType::graphics);
        const auto transferQueueResult = _device.get_queue(vkb::QueueType::transfer);
        const auto computeQueueResult = _device.get_queue(vkb::QueueType::compute);
        const auto presentQueueResult = _device.get_queue(vkb::QueueType::present);

        if (!(graphicsQueueResult && transferQueueResult && computeQueueResult && presentQueueResult))
        {
            return error_code::DEVICE_QUEUE_ACQUISITION_FAILURE;
        }

        _graphics = graphicsQueueResult.value();
        _transfer = transferQueueResult.value();
        _compute = computeQueueResult.value();
        _present = presentQueueResult.value();

        return error_code::NO_ERROR;
    }

    render_manager::error_code render_manager::build_resources_per_frame_in_flight()
    {
        _resourcesPerFrameInFlight.resize(_framesInFlight);

        VkSemaphoreCreateInfo semaphoreInfo = {
            VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            nullptr,
            0
        };

        VkFenceCreateInfo fenceInfo = {
            VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            nullptr,
            VK_FENCE_CREATE_SIGNALED_BIT
        };

        VkCommandPoolCreateInfo graphicsCommandPoolInfo = {
            VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            nullptr,
            0u,
            _device.get_queue_index(vkb::QueueType::graphics).value()
        };

        VkCommandPoolCreateInfo transferCommandPoolInfo = {
            VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            nullptr,
            0u,
            _device.get_queue_index(vkb::QueueType::transfer).value()
        };

        VkCommandPoolCreateInfo computeCommandPoolInfo = {
            VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            nullptr,
            0u,
            _device.get_queue_index(vkb::QueueType::compute).value()
        };

        for (std::size_t i = 0; i < _framesInFlight; ++i)
        {
            VkSemaphore imageReady = {}, renderComplete = {};
            VkFence inFlight = {};

            if (_funcs.createSemaphore(&semaphoreInfo, get_allocation_callbacks(), &imageReady) != VK_SUCCESS
                || _funcs.createSemaphore(&semaphoreInfo, get_allocation_callbacks(), &renderComplete) != VK_SUCCESS
                || _funcs.createFence(&fenceInfo, get_allocation_callbacks(), &inFlight) != VK_SUCCESS)
            {
                return error_code::RESOURCE_ALLOCATION_FAILURE;
            }

            VkCommandPool graphicsCmdPool = {}, computeCmdPool = {}, transferCmdPool = {};
            if (_funcs.createCommandPool(&graphicsCommandPoolInfo, get_allocation_callbacks(), &graphicsCmdPool) != VK_SUCCESS
                || _funcs.createCommandPool(&computeCommandPoolInfo, get_allocation_callbacks(), &computeCmdPool) != VK_SUCCESS
                || _funcs.createCommandPool(&transferCommandPoolInfo, get_allocation_callbacks(), &transferCmdPool) != VK_SUCCESS)
            {
                return error_code::RESOURCE_ALLOCATION_FAILURE;
            }

            _resourcesPerFrameInFlight[i] =
            {
                imageReady,
                renderComplete,
                inFlight,
                {
                    computeCmdPool,
                    {},
                    0,
                    _compute,
                    computeCommandPoolInfo.queueFamilyIndex
                },
                {
                    graphicsCmdPool,
                    {},
                    0,
                    _graphics,
                    graphicsCommandPoolInfo.queueFamilyIndex
                },
                {
                    transferCmdPool,
                    {},
                    0,
                    _transfer,
                    transferCommandPoolInfo.queueFamilyIndex
                },
                descriptor_allocator(this),
                {}
            };
        }

        return error_code::NO_ERROR;
    }

    render_manager::error_code render_manager::rebuild_swapchain()
    {
        return error_code::NO_ERROR;
    }

    void render_manager::release_resources()
    {
        _funcs.deviceWaitIdle();

        for (auto& perSwapchainImageResource : _resourcesPerFrameInFlight)
        {
            if (!perSwapchainImageResource.compute.buffers.empty())
                _funcs.freeCommandBuffers(perSwapchainImageResource.compute.pool, as<uint32_t>(perSwapchainImageResource.compute.buffers.size()), perSwapchainImageResource.compute.buffers.data());
            if (!perSwapchainImageResource.transfer.buffers.empty())
                _funcs.freeCommandBuffers(perSwapchainImageResource.transfer.pool, as<uint32_t>(perSwapchainImageResource.transfer.buffers.size()), perSwapchainImageResource.transfer.buffers.data());
            if (!perSwapchainImageResource.graphics.buffers.empty())
                _funcs.freeCommandBuffers(perSwapchainImageResource.graphics.pool, as<uint32_t>(perSwapchainImageResource.graphics.buffers.size()), perSwapchainImageResource.graphics.buffers.data());
            _funcs.destroyCommandPool(perSwapchainImageResource.compute.pool, get_allocation_callbacks());
            _funcs.destroyCommandPool(perSwapchainImageResource.transfer.pool, get_allocation_callbacks());
            _funcs.destroyCommandPool(perSwapchainImageResource.graphics.pool, get_allocation_callbacks());

            perSwapchainImageResource.descriptorAllocator.reset();
            perSwapchainImageResource.dtorQueue.flush();

            for (const auto& pool : perSwapchainImageResource.descriptorAllocator._free)
            {
                _funcs.destroyDescriptorPool(pool, get_allocation_callbacks());
            }
        }

        for (const auto& perFrameInFlightResource : _resourcesPerFrameInFlight)
        {
            _funcs.destroySemaphore(perFrameInFlightResource.present, get_allocation_callbacks());
            _funcs.destroySemaphore(perFrameInFlightResource.render, get_allocation_callbacks());
            _funcs.destroyFence(perFrameInFlightResource.renderFence, get_allocation_callbacks());
        }

        for (const auto& f : _fences)
        {
            _funcs.destroyFence(f, get_allocation_callbacks());
        }

        for (const auto& sam : _samplers)
        {
            _funcs.destroySampler(sam, get_allocation_callbacks());
        }

        for (const auto& mod : _shaderModules)
        {
            _funcs.destroyShaderModule(mod, get_allocation_callbacks());
        }

        for (const auto& pipeline : _pipelines)
        {
            _funcs.destroyPipeline(pipeline, get_allocation_callbacks());
        }

        for (const auto& buf : _buffers)
        {
            vmaDestroyBuffer(_allocator, buf.buffer, buf.allocation);
        }

        for (const auto& desc : _descriptorLayouts)
        {
            _funcs.destroyDescriptorSetLayout(desc, get_allocation_callbacks());
        }

        for (const auto& frameBuffer : _frameBuffers)
        {
            _funcs.destroyFramebuffer(frameBuffer, get_allocation_callbacks());
        }

        for (const auto& img : _images)
        {
            vmaDestroyImage(_allocator, img.image, img.allocation);
        }

        for (const auto& view : _imageViews)
        {
            _funcs.destroyImageView(view, get_allocation_callbacks());
        }

        for (const auto& layout : _layouts)
        {
            _funcs.destroyPipelineLayout(layout, get_allocation_callbacks());
        }

        for (const auto& renderPass : _renderPasses)
        {
            _funcs.destroyRenderPass(renderPass, get_allocation_callbacks());
        }

        vkb::destroy_swapchain(_swapchain);
        vkb::destroy_surface(_instance, _surface);
    }

    void render_manager::reset(cached_command_pool& pool)
    {
        _funcs.resetCommandPool(pool.pool, 0);
        pool.fetchIndex = 0;
    }

    void command_list::begin()
    {
        VkCommandBufferBeginInfo info = {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            nullptr,
            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            nullptr
        };
        _funcs->beginCommandBuffer(_buffer, &info);
    }

    void command_list::end()
    {
        _funcs->endCommandBuffer(_buffer);
    }

    void command_list::submit(const submit_info& info, const fence f)
    {
        VkSubmitInfo i = {
            VK_STRUCTURE_TYPE_SUBMIT_INFO,
            nullptr
        };

        i.waitSemaphoreCount = as<std::uint32_t>(info.wait.length());
        i.commandBufferCount = 1;
        i.signalSemaphoreCount = as<std::uint32_t>(info.signal.length());

        i.pCommandBuffers = &_buffer;
        i.pSignalSemaphores = info.signal.data();

        linear_allocator allocator(1024);

        auto waitSemaphores = allocator.typed_allocate<VkSemaphore>(i.waitSemaphoreCount);
        auto waitStages = allocator.typed_allocate<VkPipelineStageFlags>(i.waitSemaphoreCount);

        for (std::size_t idx = 0; idx < i.waitSemaphoreCount; ++idx)
        {
            waitSemaphores[idx] = info.wait[idx].sem;
            waitStages[idx] = to_vulkan(info.wait[idx].stageMask);
        }

        i.pWaitSemaphores = waitSemaphores;
        i.pWaitDstStageMask = waitStages;

        _funcs->queueSubmit(_target, 1, &i, f);
    }

    void command_list::barrier(pipeline_stage src, pipeline_stage dst, const span<memory_barrier>& memBarriers, const span<buffer_memory_barrier>& bufMemBarriers, const span<image_memory_barrier>& imgMemBarriers)
    {       
        const auto memBarrierSz = sizeof(VkMemoryBarrier) * memBarriers.length();
        const auto bufMemBarrierSz = sizeof(VkBufferMemoryBarrier) * bufMemBarriers.length();
        const auto imgMemBarrierSz = sizeof(VkImageMemoryBarrier) * imgMemBarriers.length();
        const auto allocSize = memBarrierSz + bufMemBarrierSz + imgMemBarrierSz;

        linear_allocator allocator(allocSize);
        VkMemoryBarrier* vkMemBarriers = allocator.typed_allocate<VkMemoryBarrier>(memBarriers.length());
        VkBufferMemoryBarrier* vkBufMemBarriers = allocator.typed_allocate<VkBufferMemoryBarrier>(bufMemBarriers.length());
        VkImageMemoryBarrier* vkImgMemBarriers = allocator.typed_allocate<VkImageMemoryBarrier>(imgMemBarriers.length());

        for (std::size_t i = 0; i < memBarriers.length(); ++i)
        {
            vkMemBarriers[i] = {
                .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
                .pNext = nullptr,
                .srcAccessMask = to_vulkan(memBarriers[i].src),
                .dstAccessMask = to_vulkan(memBarriers[i].dst)
            };
        }

        for (std::size_t i = 0; i < bufMemBarriers.length(); ++i)
        {
            vkBufMemBarriers[i] = {
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                .pNext = nullptr,
                .srcAccessMask = to_vulkan(bufMemBarriers[i].src),
                .dstAccessMask = to_vulkan(bufMemBarriers[i].dst),
                .srcQueueFamilyIndex = bufMemBarriers[i].srcQueue,
                .dstQueueFamilyIndex = bufMemBarriers[i].dstQueue,
                .buffer = bufMemBarriers[i].buf.buffer,
                .offset = bufMemBarriers[i].offset,
                .size = bufMemBarriers[i].size
            };
        }

        for (std::size_t i = 0; i < imgMemBarriers.length(); ++i)
        {
            vkImgMemBarriers[i] = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .pNext = nullptr,
                .srcAccessMask = to_vulkan(imgMemBarriers[i].src),
                .dstAccessMask = to_vulkan(imgMemBarriers[i].dst),
                .oldLayout = to_vulkan(imgMemBarriers[i].oldLayout),
                .newLayout = to_vulkan(imgMemBarriers[i].newLayout),
                .srcQueueFamilyIndex = imgMemBarriers[i].srcQueue,
                .dstQueueFamilyIndex = imgMemBarriers[i].dstQueue,
                .image = imgMemBarriers[i].img.image,
                .subresourceRange = {
                    .aspectMask = to_vulkan(imgMemBarriers[i].range.aspect),
                    .baseMipLevel = imgMemBarriers[i].range.baseMipLevel,
                    .levelCount = imgMemBarriers[i].range.mipLevelCount,
                    .baseArrayLayer = imgMemBarriers[i].range.baseLayer,
                    .layerCount = imgMemBarriers[i].range.layerCount
                }
            };
        }

        _funcs->cmdPipelineBarrier(_buffer, to_vulkan(src), to_vulkan(dst), 0, as<std::uint32_t>(memBarriers.length()), vkMemBarriers, as<std::uint32_t>(bufMemBarriers.length()), vkBufMemBarriers, as<std::uint32_t>(imgMemBarriers.length()), vkImgMemBarriers);
    }

    command_list::operator bool() const noexcept
    {
        return _buffer != nullptr;
    }

    std::uint32_t command_list::queue_index() const noexcept
    {
        return _queueIndex;
    }

    command_list::command_list(VkCommandBuffer cmdBuffer, vkb::DispatchTable& fns, VkQueue target, std::uint32_t queueIndex)
        : _buffer(cmdBuffer), _funcs(&fns), _target(target), _queueIndex(queueIndex)
    {
    }

    void graphics_command_list::begin_render_pass(const render_pass_begin_info& begin)
    {
        VkRenderPassAttachmentBeginInfo attachmentInfo = {};
        if (begin.attachmentBegin)
        {
            attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO;
            attachmentInfo.pNext = nullptr;
            attachmentInfo.attachmentCount = as<std::uint32_t>(begin.attachmentBegin->views.length());
            attachmentInfo.pAttachments = begin.attachmentBegin->views.data();
        }

        VkRenderPassBeginInfo rpBegin = {
            VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            begin.attachmentBegin ? &attachmentInfo : nullptr,
            begin.pass,
            begin.buffer,
            { begin.x, begin.y, begin.width, begin.height },
            as<std::uint32_t>(begin.clearValues.length()),
            begin.clearValues.data()
        };

        _funcs->cmdBeginRenderPass(_buffer, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);
    }

    void graphics_command_list::end_render_pass()
    {
        _funcs->cmdEndRenderPass(_buffer);
    }

    void graphics_command_list::draw_arrays(const std::uint32_t count, const std::uint32_t instances, const std::uint32_t firstVertex, const std::uint32_t firstInstance)
    {
        _funcs->cmdDraw(_buffer, count, instances, firstVertex, firstInstance);
    }

    void graphics_command_list::bind_graphics_pipeline(const pipeline& pipeline)
    {
        _funcs->cmdBindPipeline(_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    }

    void graphics_command_list::bind_graphics_descriptor_sets(const pipeline_layout& layout, const span<descriptor_set>& sets, const std::uint32_t firstSet, const span<std::uint32_t>& offsets)
    {
        _funcs->cmdBindDescriptorSets(_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, firstSet, as<std::uint32_t>(sets.length()), sets.data(), as<std::uint32_t>(offsets.length()), offsets.data());
    }

    void graphics_command_list::set_viewports(const span<viewport>& viewports)
    {
        static constexpr size_t max = 4;
        VkViewport vps[max] = {};
        for (size_t i = 0; i < max && i < viewports.length(); ++i)
        {
            vps[i] = {
                .x = viewports[i].x,
                .y = viewports[i].y,
                .width = viewports[i].width,
                .height = viewports[i].height,
                .minDepth = viewports[i].minDepth,
                .maxDepth = viewports[i].maxDepth
            };
        }
        
        _funcs->cmdSetViewport(_buffer, 0, as<std::uint32_t>(viewports.length()), vps);
    }

    void graphics_command_list::set_scissors(const span<scissor_region>& scissors)
    {
        static constexpr size_t max = 4;
        VkRect2D srs[max] = {};
        for (size_t i = 0; i < max && i < scissors.length(); ++i)
        {
            srs[i] = {
                .offset = { .x = scissors[i].x, .y = scissors[i].y },
                .extent = { .width = scissors[i].width, .height = scissors[i].height }
            };
        }

        _funcs->cmdSetScissor(_buffer, 0, as<std::uint32_t>(scissors.length()), srs);
    }

    graphics_command_list::graphics_command_list(VkCommandBuffer cmdBuffer, vkb::DispatchTable& fns, VkQueue target, std::uint32_t queueIndex)
        : command_list(cmdBuffer, fns, target, queueIndex)
    {
    }

    void transfer_command_list::copy(const buffer& src, const buffer& dst, const span<buffer_copy_regions>& regions)
    {
        static constexpr std::size_t max = 16;
        VkBufferCopy copies[max] = {};

        std::size_t copied = 0;

        for (std::size_t i = 0; i < regions.length(); i += max)
        {
            std::size_t count = std::min(regions.length() - copied, max);

            for (std::size_t j = 0; j < count; ++j)
            {
                copies[j] = {
                    .srcOffset = regions[copied + j].srcOffset,
                    .dstOffset = regions[copied + j].dstOffset,
                    .size = regions[copied + j].size
                };
            }

            _funcs->cmdCopyBuffer(_buffer, src.buffer, dst.buffer, as<std::uint32_t>(count), copies + copied);
            copied += count;
        }
    }

    void transfer_command_list::copy(const buffer& src, const image& dst, const image_layout layout, const span<buffer_image_copy_regions>& regions)
    {
        static constexpr std::size_t max = 16;
        VkBufferImageCopy copies[max] = {};

        std::size_t copied = 0;

        for (std::size_t i = 0; i < regions.length(); i += max)
        {
            std::size_t count = std::min(regions.length() - copied, max);

            for (std::size_t j = 0; j < count; ++j)
            {
                auto& region = regions[copied + j];
                copies[j] = {
                    .bufferOffset = region.bufferOffset,
                    .bufferRowLength = as<std::uint32_t>(region.rowLength),
                    .bufferImageHeight = as<std::uint32_t>(region.imageHeight),
                    .imageSubresource = {
                        .aspectMask = to_vulkan(region.subresource.aspect),
                        .mipLevel = region.subresource.mipLevel,
                        .baseArrayLayer = region.subresource.baseArrayLayer,
                        .layerCount = region.subresource.layerCount
                    },
                    .imageOffset = {
                        .x = region.x,
                        .y = region.y,
                        .z = region.z
                    },
                    .imageExtent = {
                        .width = region.width,
                        .height = region.height,
                        .depth = region.depth
                    }
                };
            }

            _funcs->cmdCopyBufferToImage(_buffer, src.buffer, dst.image, to_vulkan(layout), as<std::uint32_t>(count), copies + copied);
            copied += count;
        }
    }

    transfer_command_list::transfer_command_list(VkCommandBuffer cmdBuffer, vkb::DispatchTable& fns, VkQueue target, std::uint32_t queueIndex)
        : command_list(cmdBuffer, fns, target, queueIndex)
    {
    }

    void render_manager::DeletionQueue::push_deletor(std::function<void()>&& fn)
    {
        deletors.push_back(std::move(fn));
    }

    void render_manager::DeletionQueue::flush()
    {
        for (auto& fn : deletors)
        {
            fn();
        }
        deletors.clear();
    }

    std::size_t descriptor_layout_cache::descriptor_layout_binding_hasher::operator()(const descriptor_set_layout_binding& key) const noexcept
    {
        std::size_t hash = 7;

        hash = 31 * hash + std::hash<std::uint32_t>()(key.binding);
        hash = 31 * hash + std::hash<std::uint32_t>()(key.count);
        hash = 31 * hash + std::hash<std::uint32_t>()(as<std::uint32_t>(key.stages));
        hash = 31 * hash + std::hash<std::uint32_t>()(as<std::uint32_t>(key.type));

        return hash;
    }

    std::size_t descriptor_layout_cache::descriptor_layout_cache_hasher::operator()(const descriptor_layout_cache_entry& key) const noexcept
    {
        std::size_t result = 7;
        result = 31 * std::hash<size_t>()(key.count);
        for (auto i = 0u; i < key.count; ++i)
        {
            result = 31 * result + descriptor_layout_binding_hasher()(key.bindings[i]);
        }
        return result;
    }   

    descriptor_layout_cache::descriptor_layout_cache(render_manager& manager)
        : _manager(manager)
    {
    }

    result<descriptor_set_layout, descriptor_layout_cache::error_code> descriptor_layout_cache::allocate(const descriptor_set_layout_create_info& info)
    {
        descriptor_set_layout res = {};

        descriptor_layout_cache_entry key = {};
        key.count = as<std::uint32_t>(info.bindings.length());

#ifdef _RYUJIN_WINDOWS
        memcpy_s(key.bindings.data(), key.bindings.size() * sizeof(descriptor_set_layout_binding), info.bindings.data(), info.bindings.length() * sizeof(descriptor_set_layout_binding));
#else
        memcpy(key.bindings.data(), info.bindings.data(), info.bindings.length() * sizeof(descriptor_set_layout_binding));
#endif
        const auto it = _cache.find(key);
        if (it == _cache.end())
        {
            const auto allocRes = _manager._create(info);
            if (allocRes)
            {
                res = *allocRes;
                _cache[key] = res;
            }
            else
            {
                return result<descriptor_set_layout, error_code>::from_error(error_code::ALLOCATION_FAILURE);
            }
        }
        else
        {
            res = it->second;
        }

        return result<descriptor_set_layout, error_code>::from_success(res);
    }

    void descriptor_layout_cache::flush()
    {
        for (const auto& [cacheEnty, layout] : _cache)
        {
            _manager.release(layout);
        }
    }

    descriptor_allocator::descriptor_allocator(render_manager* manager)
        : _manager(manager)
    {
    }

    void descriptor_allocator::reset()
    {
        _free.reserve(_free.size() + _used.size());
        for (auto& pool : _used)
        {
            _manager->reset(pool);
            _free.push_back(pool);
        }
        _used.clear();

        _current = VK_NULL_HANDLE;
    }

    result<descriptor_set, descriptor_allocator::error_code> descriptor_allocator::allocate(descriptor_set_layout layout)
    {
        if (_current == VK_NULL_HANDLE)
        {
            _current = fetch_pool();
            _used.push_back(_current);
        }

        descriptor_set_allocate_info ainfo = {
            .pool = _current,
            .layout = layout
        };

        descriptor_set set = {};

        auto res = _manager->_allocate(ainfo);

        bool needRealloc = false;
        if (!res)
        {
            const auto error = res.error_code();
            switch (error)
            {
            case render_manager::error_code::RESOURCE_ALLOCATION_FROM_POOL_FAILURE:
                needRealloc = true;
            }
        }
        else
        {
            set = *res;
            return result<descriptor_set, error_code>::from_success(set);
        }

        if (needRealloc)
        {
            _current = fetch_pool();
            _used.push_back(_current);

            ainfo.pool = _current;
            res = _manager->_allocate(ainfo);

            if (res)
            {
                set = *res;
                return result<descriptor_set, error_code>::from_success(set);
            }
        }
        return result<descriptor_set, error_code>::from_error(error_code::POOL_ALLOCATION_FAILURE);
    }

    void descriptor_allocator::clean_up()
    {
        for (auto& p : _free)
        {
            _manager->release(p);
        }

        for (auto& p : _used)
        {
            _manager->release(p);
        }
    }

    descriptor_pool descriptor_allocator::create_pool(const std::size_t count)
    {
        descriptor_pool pool = {};

        const descriptor_pool_size sizes[] = {
            { descriptor_type::SAMPLER, as<std::uint32_t>(count * 0.5f) },
            { descriptor_type::COMBINED_IMAGE_SAMPLER, as<std::uint32_t>(count * 4.0f) },
            { descriptor_type::SAMPLED_IMAGE, as<std::uint32_t>(count * 4.0f) },
            { descriptor_type::STORAGE_IMAGE, as<std::uint32_t>(count * 1.0f) },
            { descriptor_type::UNIFORM_TEXEL_BUFFER, as<std::uint32_t>(count * 1.0f) },
            { descriptor_type::STORAGE_TEXEL_BUFFER, as<std::uint32_t>(count * 1.0f) },
            { descriptor_type::UNIFORM_BUFFER, as<std::uint32_t>(count * 2.0f) },
            { descriptor_type::STORAGE_BUFFER, as<std::uint32_t>(count * 2.0f) },
            { descriptor_type::DYNAMIC_UNIFORM_BUFFER, as<std::uint32_t>(count * 1.0f) },
            { descriptor_type::DYNAMIC_STORAGE_BUFFER, as<std::uint32_t>(count * 1.0f) },
            { descriptor_type::INPUT_ATTACHMENT, as<std::uint32_t>(count * 0.5f) }
        };

        const descriptor_pool_create_info info = {
            .maxSetCount = as<const std::uint32_t>(count),
            .sizes = span(sizes)
        };
        const auto res = _manager->create(info);
        if (res)
        {
            pool = *res;
        }
        else
        {
            spdlog::error("Failed to create descriptor pool for allocator.");
        }

        return pool;
    }

    descriptor_pool descriptor_allocator::fetch_pool()
    {
        if (_free.empty())
        {
            return create_pool(1000);
        }
        else
        {
            auto pool = _free.back();
            _free.pop_back();
            return pool;
        }
    }

    descriptor_writer::descriptor_writer(render_manager* manager)
        : _manager(manager)
    {
    }

    descriptor_writer::descriptor_writer(descriptor_writer&& writer) noexcept
        : _manager(writer._manager), _writes(std::move(writer._writes)), _buffers(std::move(writer._buffers)), _images(std::move(writer._images))
    {
        writer._manager = nullptr;
    }

    descriptor_writer& descriptor_writer::operator=(descriptor_writer&& rhs) noexcept
    {
        _writes = std::move(rhs._writes);
        _buffers = std::move(rhs._buffers);
        _images = std::move(rhs._images);
        _manager = rhs._manager;
        rhs._manager = nullptr;
        return *this;
    }

    descriptor_writer& descriptor_writer::write_buffer(const descriptor_set set, const std::uint32_t binding, const descriptor_type type, const std::uint32_t element, const span<descriptor_buffer_info>& buffers)
    {
        descriptor_write_info info = {
            .set = set,
            .type = type,
            .binding = binding,
            .element = element,
            .info = buffers
        };

        _manager->write(info);

        return *this;
    }

    descriptor_writer& descriptor_writer::write_image(const descriptor_set set, const std::uint32_t binding, const descriptor_type type, const std::uint32_t element, const span<descriptor_image_info>& images)
    {
        descriptor_write_info info = {
            .set = set,
            .type = type,
            .binding = binding,
            .element = element,
            .info = images
        };

        _manager->write(info);

        return *this;
    }

    void descriptor_writer::write()
    {
        if (_manager)
        {

        }
    }

}