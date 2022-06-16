#include <ryujin/graphics/render_graph.hpp>

#include <ryujin/graphics/window.hpp>

#include <VkBootstrap.h>
#include <vma/vk_mem_alloc.h>

#undef APIENTRY
#include <spdlog/spdlog.h>

namespace ryujin
{
    namespace detail
    {
        static VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_message_callback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData) {

            if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
            {
                spdlog::error("Vulkan Validation Message: {}", pCallbackData->pMessage);
            }
            else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            {
                spdlog::warn("Vulkan Validation Message: {}", pCallbackData->pMessage);
            }
            else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
            {
                spdlog::info("Vulkan Validation Message: {}", pCallbackData->pMessage);
            }
            else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
            {
                spdlog::debug("Vulkan Validation Message: {}", pCallbackData->pMessage);
            }
            else
            {
                spdlog::debug("Vulkan Validation Message: {}", pCallbackData->pMessage);
            }

            return VK_FALSE;
        }

        void set_vulkan_api_dump(vkb::InstanceBuilder& bldr)
        {
#if defined(RYUJIN_ENABLE_VULKAN_API_DUMP)
            bldr.enable_layer("VK_LAYER_LUNARG_api_dump");
#endif
        }

        bool set_vulkan_debug_extensions(vkb::InstanceBuilder& bldr)
        {
#if defined(_DEBUG) || defined(RYUJIN_ENABLE_VULKAN_DEBUG_UTILITIES)
            bldr.request_validation_layers()
                .set_debug_callback(vulkan_debug_message_callback)
                .set_debug_messenger_severity(VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
                .set_debug_messenger_type(VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
                .add_validation_feature_enable(VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT)
                .add_validation_feature_enable(VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT);

            const auto sysInfoResult = vkb::SystemInfo::get_system_info();
            if (sysInfoResult)
            {
                if (sysInfoResult->is_extension_available(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
                {
                    bldr.enable_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
                    spdlog::info("Vulkan extension {} requested.", VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
                    return true;
                }
                else
                {
                    spdlog::warn("Vulkan extension {} required, but not available. Extension not requested.", VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
                    return false;
                }
            }
            else
            {
                spdlog::warn("Vulkan extension {} required, but failed to determine if it is available. Extension not requested.", VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
                return false;
            }
#endif
            return false;
        }
    }

    struct vulkan_deletion_queue
    {
        vector<move_only_function<void()>> queue;

        void flush()
        {
            for (auto& fn : queue)
            {
                fn();
            }
            queue.clear();
        }
    };

    struct vulkan_sampler_cache
    {

    };

    struct render_graph_frame_data
    {
        VkSemaphore renderComplete;
        VkSemaphore imageReady;
        VkFence renderFence;
    };

    struct vulkan_texture
    {
        VkImage image = VK_NULL_HANDLE;
        VkImageView view = VK_NULL_HANDLE;
        VkSampler sampler = VK_NULL_HANDLE; // non-owning. multiple textures can share the same sampler

        VmaAllocation imageAllocation = {};
        VmaAllocationInfo imageAllocationInfo = {};
    };

    struct render_target::impl
    {
        string name;
        u32 width = 0;
        u32 height = 0;
        VkFormat fmt = VK_FORMAT_UNDEFINED;
        VkImageUsageFlags usage = 0;
        
        vulkan_texture tex = {};
    };

    enum class pass_type
    {
        UNDEFINED,
        GRAPHICS,
        COMPUTE
    };

    struct pass::impl
    {
        string name;
        pass_type type = pass_type::UNDEFINED;

        VkRenderPass renderPass = VK_NULL_HANDLE;
        VkFramebuffer fbo = VK_NULL_HANDLE;

        move_only_function<void(command_buffer&)> commands;
        vector<render_resource_handle<pass>> src;
        vector<render_resource_handle<pass>> dst;
    };

    struct render_graph::impl
    {
        vkb::Instance instance = {};
        vkb::PhysicalDevice physical = {};
        vkb::Device logical = {};
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        vkb::Swapchain swap = {};
        vkb::DispatchTable funcs = {};
        bool objectNamingEnabled = false;

        VmaAllocator allocator = {};

        u32 swapchainImageIndex = 0;
        u32 currentFrame = 0;
        u32 framesInFlight = 0;

        vector<render_graph_frame_data> frameData;

        slot_map<render_target> targets;
        slot_map<render_target> swapchainImages;
        vector<slot_map_key> swapchainKeys;

        slot_map<pass> passes;
    };
    
    render_target_builder::render_target_builder(const string& name)
        : _name(name)
    {
    }
    
    render_target_builder& render_target_builder::width(const u32 w)
    {
        _width = w;
        return *this;
    }
    
    render_target_builder& render_target_builder::height(const u32 h)
    {
        _height = h;
        return *this;
    }
    
    render_target_builder& render_target_builder::format(const data_format fmt)
    {
        _fmt = fmt;
        return *this;
    }
    
    render_target_builder& render_target_builder::use_as_color_attachment()
    {
        _color = true;
        return *this;
    }
    
    render_target_builder& render_target_builder::use_as_depth_attachment()
    {
        _depth = true;
        return *this;
    }

    render_target_builder& render_target_builder::use_as_input_attachment()
    {
        _input = true;
        return *this;
    }
    
    bool render_target_builder::validate() const noexcept
    {
        const bool dimensionsValid = _width != 0 && _height != 0;
        const bool validFormat = _fmt != data_format::UNDEFINED;
        const bool usageSet = _color || _depth;
        return dimensionsValid && validFormat && usageSet;
    }

    render_graph::render_graph(const unique_ptr<window>& win, const u32 framesInFlight)
    {
        _impl = new impl();

        _impl->framesInFlight = framesInFlight;
        
        assert(load_instance() && "Failed to create Vulkan instance.");
        assert(query_physical_device() && "Failed to find suitable physical device.");
        assert(load_device() && "Failed to create Vulkan device.");
        assert(build_swapchain(win) && "Failed to create Vulkan surface and swapchain.");
        assert(build_frame_in_flight_data() && "Failed to create frame-specific data.");
    }

    render_graph::render_graph(render_graph&& other) noexcept
        : _impl(other._impl)
    {
        other._impl = nullptr;
    }

    render_graph::~render_graph()
    {
        if (_impl)
        {
            release_resources();
            delete _impl;
        }
        _impl = nullptr;
    }

    render_graph& render_graph::operator=(render_graph&& rhs) noexcept
    {
        if (&rhs == this)
        {
            return *this;
        }

        delete _impl;
        _impl = rhs._impl;
        rhs._impl = nullptr;

        return *this;
    }

    render_resource_handle<pass> render_graph::add_pass(const pass_builder& bldr)
    {
        return render_resource_handle<pass>();
    }

    render_resource_handle<render_target> render_graph::add_render_target(const render_target_builder& bldr)
    {
        const auto valid = bldr.validate();
        assert(valid && "Render target builder was not valid.");

        render_target tgt;
        tgt._impl->width = bldr._width;
        tgt._impl->height = bldr._height;
        tgt._impl->name = bldr._name;
        tgt._impl->fmt = as<VkFormat>(bldr._fmt);

        VkImageUsageFlags usage = 0;
        usage |= bldr._color ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : 0;
        usage |= bldr._depth ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : 0;
        usage |= bldr._input ? VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT : 0;

        tgt._impl->usage = usage;

        build_render_target(tgt);
        
        auto key = _impl->targets.insert(ryujin::move(tgt));

        render_resource_handle<render_target> handle;
        handle._graph = this;
        handle._key = key;

        return handle;
    }

    render_resource_handle<render_target> render_graph::get_back_buffer() const noexcept
    {
        render_resource_handle<render_target> tgt;
        tgt._graph = this;
        tgt._key = _impl->swapchainKeys[_impl->swapchainImageIndex];
        return tgt;
    }

    bool render_graph::execute()
    {
        return false;
    }

    bool render_graph::load_instance()
    {
        vkb::InstanceBuilder instanceBldr;
        instanceBldr.set_app_name("Ryujin Application")
            .set_app_version(VK_MAKE_VERSION(0, 0, 1))
            .set_engine_name("Ryujin Engine")
            .set_engine_version(VK_MAKE_VERSION(0, 0, 1))
            .require_api_version(VK_MAKE_VERSION(1, 2, 0));

        detail::set_vulkan_api_dump(instanceBldr);
        _impl->objectNamingEnabled = detail::set_vulkan_debug_extensions(instanceBldr);

        const auto result = instanceBldr.build();
        if (result)
        {
            spdlog::info("Successfully created VkInstance.");
            _impl->instance = *result;
            return true;
        }

        spdlog::critical("Failed to create VkInstance.");
        return false;
    }

    bool render_graph::query_physical_device()
    {
        VkPhysicalDeviceFeatures feats = {};
        feats.independentBlend = true;
        feats.logicOp = true;
        feats.alphaToOne = true;
        feats.depthBounds = true;
        feats.depthClamp = true;
        feats.depthBiasClamp = true;
        feats.fillModeNonSolid = true;

        VkPhysicalDeviceVulkan12Features feats12 = {};
        feats12.imagelessFramebuffer = VK_TRUE; // needed for framebuffers without images created ahead of time
        feats12.separateDepthStencilLayouts = VK_TRUE; // needed for depth only buffers
        feats12.drawIndirectCount = VK_TRUE; // needed to make frames go brrrt

        vkb::PhysicalDeviceSelector deviceSelector{ _impl->instance };
        deviceSelector.prefer_gpu_device_type(vkb::PreferredDeviceType::discrete)
            .set_required_features(feats)
            .set_required_features_12(feats12)
            .require_present()
            .defer_surface_initialization();

        const auto result = deviceSelector.select();
        if (result)
        {
            spdlog::info("Succesfully found a suitable VkPhysicalDevice: {}", result->properties.deviceName);
            _impl->physical = *result;
            return true;
        }

        spdlog::critical("Failed to find a suitable VkPhysicalDevice.");
        return false;
    }

    bool render_graph::load_device()
    {
        vkb::DeviceBuilder deviceBuilder{ _impl->physical };
        const auto result = deviceBuilder.build();

        if (result)
        {
            spdlog::info("Succesfully created a VkDevice from the VkPhysicalDevice - {}", _impl->physical.properties.deviceName);
            _impl->logical = *result;
            _impl->funcs = _impl->logical.make_table();

            const VmaVulkanFunctions fns = {
                .vkGetInstanceProcAddr = _impl->instance.fp_vkGetInstanceProcAddr,
                .vkGetDeviceProcAddr = _impl->logical.fp_vkGetDeviceProcAddr,
                .vkGetPhysicalDeviceProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(_impl->instance.fp_vkGetInstanceProcAddr(_impl->instance.instance, "vkGetPhysicalDeviceProperties")),
                .vkGetPhysicalDeviceMemoryProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties>(_impl->instance.fp_vkGetInstanceProcAddr(_impl->instance.instance, "vkGetPhysicalDeviceMemoryProperties")),
                .vkAllocateMemory = reinterpret_cast<PFN_vkAllocateMemory>(_impl->logical.fp_vkGetDeviceProcAddr(_impl->logical.device, "vkAllocateMemory")),
                .vkFreeMemory = reinterpret_cast<PFN_vkFreeMemory>(_impl->logical.fp_vkGetDeviceProcAddr(_impl->logical.device, "vkFreeMemory")),
                .vkMapMemory = reinterpret_cast<PFN_vkMapMemory>(_impl->logical.fp_vkGetDeviceProcAddr(_impl->logical.device, "vkMapMemory")),
                .vkUnmapMemory = reinterpret_cast<PFN_vkUnmapMemory>(_impl->logical.fp_vkGetDeviceProcAddr(_impl->logical.device, "vkUnmapMemory")),
                .vkFlushMappedMemoryRanges = reinterpret_cast<PFN_vkFlushMappedMemoryRanges>(_impl->logical.fp_vkGetDeviceProcAddr(_impl->logical.device, "vkFlushMappedMemoryRanges")),
                .vkInvalidateMappedMemoryRanges = reinterpret_cast<PFN_vkInvalidateMappedMemoryRanges>(_impl->logical.fp_vkGetDeviceProcAddr(_impl->logical.device, "vkInvalidateMappedMemoryRanges")),
                .vkBindBufferMemory = reinterpret_cast<PFN_vkBindBufferMemory>(_impl->logical.fp_vkGetDeviceProcAddr(_impl->logical.device, "vkBindBufferMemory")),
                .vkBindImageMemory = reinterpret_cast<PFN_vkBindImageMemory>(_impl->logical.fp_vkGetDeviceProcAddr(_impl->logical.device, "vkBindImageMemory")),
                .vkGetBufferMemoryRequirements = reinterpret_cast<PFN_vkGetBufferMemoryRequirements>(_impl->logical.fp_vkGetDeviceProcAddr(_impl->logical.device, "vkGetBufferMemoryRequirements")),
                .vkGetImageMemoryRequirements = reinterpret_cast<PFN_vkGetImageMemoryRequirements>(_impl->logical.fp_vkGetDeviceProcAddr(_impl->logical.device, "vkGetImageMemoryRequirements")),
                .vkCreateBuffer = reinterpret_cast<PFN_vkCreateBuffer>(_impl->logical.fp_vkGetDeviceProcAddr(_impl->logical.device, "vkCreateBuffer")),
                .vkDestroyBuffer = reinterpret_cast<PFN_vkDestroyBuffer>(_impl->logical.fp_vkGetDeviceProcAddr(_impl->logical.device, "vkDestroyBuffer")),
                .vkCreateImage = reinterpret_cast<PFN_vkCreateImage>(_impl->logical.fp_vkGetDeviceProcAddr(_impl->logical.device, "vkCreateImage")),
                .vkDestroyImage = reinterpret_cast<PFN_vkDestroyImage>(_impl->logical.fp_vkGetDeviceProcAddr(_impl->logical.device, "vkDestroyImage")),
                .vkCmdCopyBuffer = reinterpret_cast<PFN_vkCmdCopyBuffer>(_impl->logical.fp_vkGetDeviceProcAddr(_impl->logical.device, "vkCmdCopyBuffer")),
                .vkGetBufferMemoryRequirements2KHR = reinterpret_cast<PFN_vkGetBufferMemoryRequirements2KHR>(_impl->logical.fp_vkGetDeviceProcAddr(_impl->logical.device, "vkGetBufferMemoryRequirements2")),
                .vkGetImageMemoryRequirements2KHR = reinterpret_cast<PFN_vkGetImageMemoryRequirements2KHR>(_impl->logical.fp_vkGetDeviceProcAddr(_impl->logical.device, "vkGetImageMemoryRequirements2")),
                .vkBindBufferMemory2KHR = reinterpret_cast<PFN_vkBindBufferMemory2KHR>(_impl->logical.fp_vkGetDeviceProcAddr(_impl->logical.device, "vkBindBufferMemory2")),
                .vkBindImageMemory2KHR = reinterpret_cast<PFN_vkBindImageMemory2KHR>(_impl->logical.fp_vkGetDeviceProcAddr(_impl->logical.device, "vkBindImageMemory2")),
                .vkGetPhysicalDeviceMemoryProperties2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties2KHR>(_impl->instance.fp_vkGetInstanceProcAddr(_impl->instance.instance, "vkGetPhysicalDeviceMemoryProperties2")),
            };

            const VmaAllocatorCreateInfo allocatorInfo = {
                .physicalDevice = _impl->physical.physical_device,
                .device = _impl->logical.device,
                .pVulkanFunctions = &fns,
                .instance = _impl->instance.instance
            };

            const auto allocatorResult = vmaCreateAllocator(&allocatorInfo, &_impl->allocator);
            if (allocatorResult == VK_SUCCESS)
            {
                spdlog::info("Successfully created VmaAllocator for the VkDevice.");
                return true;
            }

            spdlog::critical("Failed to create VmaAllocator.");
            return false;
        }

        spdlog::critical("Failed to create VkDevice.");
        return false;
    }

    bool render_graph::build_swapchain(const unique_ptr<window>& win)
    {
        GLFWwindow* native = win->_native;
        VkSurfaceKHR surface;
        const auto surfaceResult = glfwCreateWindowSurface(_impl->instance.instance, native, _impl->instance.allocation_callbacks, &surface);
        if (surfaceResult != VK_SUCCESS)
        {
            spdlog::critical("Failed to create VkSurfaceKHR from the window.");
            return false;
        }

        _impl->logical.surface = surface;

        auto swapchainResult = vkb::SwapchainBuilder(_impl->logical, surface)
            .set_desired_present_mode(VK_PRESENT_MODE_IMMEDIATE_KHR)
            .build();

        if (!swapchainResult)
        {
            spdlog::critical("Failed to create VkSwapchainKHR from the surface.");
            vkb::destroy_surface(_impl->instance, surface);
            return false;
        }

        spdlog::info("Successfully created VkSurfaceKHR and VkSwapchainKHR.");
        _impl->surface = surface;
        _impl->swap = *swapchainResult;

        auto imagesResult = _impl->swap.get_images();
        
        if (!imagesResult)
        {
            spdlog::critical("Failed to get swapchain images.");
            vkb::destroy_swapchain(_impl->swap);
            vkb::destroy_surface(_impl->instance, _impl->surface);
            return false;
        }

        auto swapchainImages = *imagesResult;
        
        const VkImageViewUsageCreateInfo usageInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO,
            .pNext = VK_NULL_HANDLE,
            .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
        };

        VkImageViewCreateInfo viewInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = &usageInfo,
            .flags = 0,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = _impl->swap.image_format,
            .components = {
                .r = VK_COMPONENT_SWIZZLE_R,
                .g = VK_COMPONENT_SWIZZLE_G,
                .b = VK_COMPONENT_SWIZZLE_B,
                .a = VK_COMPONENT_SWIZZLE_A
            },
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        for (auto& img : swapchainImages)
        {
            viewInfo.image = img;

            VkImageView view;
            const auto viewResult = _impl->funcs.createImageView(&viewInfo, _impl->logical.allocation_callbacks, &view);
            if (viewResult != VK_SUCCESS)
            {
                spdlog::critical("Failed to create VkImageView from swapchain image.");
                return false;
            }

            render_target tgt;
            *tgt._impl = render_target::impl{
                .name = "",
                .width = _impl->swap.extent.width,
                .height = _impl->swap.extent.height,
                .fmt = _impl->swap.image_format,
                .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                .tex = {
                    .image = img,
                    .view = view,
                    .sampler = VK_NULL_HANDLE,
                    .imageAllocation = nullptr,
                    .imageAllocationInfo = {},
                }
            };

            auto key = _impl->swapchainImages.insert(ryujin::move(tgt));
            _impl->swapchainKeys.push_back(key);
        }

        return true;
    }

    bool render_graph::build_frame_in_flight_data()
    {
        for (sz i = 0; i < _impl->framesInFlight; ++i)
        {
            render_graph_frame_data data = {};

            const VkSemaphoreCreateInfo sem = {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                .pNext = VK_NULL_HANDLE,
                .flags = 0
            };

            const VkFenceCreateInfo fence = {
                .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                .pNext = VK_NULL_HANDLE,
                .flags = VK_FENCE_CREATE_SIGNALED_BIT
            };

            const auto renderComplete = _impl->funcs.createSemaphore(&sem, _impl->logical.allocation_callbacks, &data.renderComplete);
            const auto imageReady = _impl->funcs.createSemaphore(&sem, _impl->logical.allocation_callbacks, &data.imageReady);
            const auto renderFence = _impl->funcs.createFence(&fence, _impl->logical.allocation_callbacks, &data.renderFence);

            if (renderComplete != VK_SUCCESS || imageReady != VK_SUCCESS || renderFence != VK_SUCCESS)
            {
                spdlog::critical("Failed to initialize vulkan synchronization primitives");
                return false;
            }

            _impl->frameData.push_back(data);
        }

        spdlog::info("Successfully built frame data for {} frames in flight.", _impl->framesInFlight);

        return true;
    }

    shader* render_graph::get_shader(slot_map_key k) const noexcept
    {
        return nullptr;
    }

    pass* render_graph::get_pass(slot_map_key k) const noexcept
    {
        return nullptr;
    }

    render_target* render_graph::get_render_target(slot_map_key k) const noexcept
    {
        return _impl->targets.try_get(k);
    }

    void render_graph::build_render_target(render_target& tgt)
    {
        const VkImageCreateInfo imgInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = VK_NULL_HANDLE,
            .flags = 0,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = tgt._impl->fmt,
            .extent = {
                .width = tgt._impl->width,
                .height = tgt._impl->height,
                .depth = 1
            },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .usage = tgt._impl->usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = VK_NULL_HANDLE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
        };
        
        const VmaAllocationCreateInfo allocInfo = {
            .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
            .requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .preferredFlags = 0,
            .memoryTypeBits = 0,
            .pool = nullptr,
            .pUserData = nullptr,
            .priority = 1.0f
        };

        const auto imageAllocationResult = vmaCreateImage(_impl->allocator, &imgInfo, &allocInfo, &(tgt._impl->tex.image), &(tgt._impl->tex.imageAllocation), &(tgt._impl->tex.imageAllocationInfo));
        assert(imageAllocationResult == VK_SUCCESS && "Image allocation for render target failed.");

        const VkImageViewCreateInfo viewInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = VK_NULL_HANDLE,
            .flags = 0,
            .image = tgt._impl->tex.image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = tgt._impl->fmt,
            .components = {
                .r = VK_COMPONENT_SWIZZLE_R,
                .g = VK_COMPONENT_SWIZZLE_G,
                .b = VK_COMPONENT_SWIZZLE_B,
                .a = VK_COMPONENT_SWIZZLE_A
            },
            .subresourceRange = {
                .aspectMask = as<VkImageAspectFlags>((tgt._impl->usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) ? VK_IMAGE_ASPECT_COLOR_BIT : VK_IMAGE_ASPECT_DEPTH_BIT),
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        const auto viewAllocationResult = _impl->funcs.createImageView(&viewInfo, _impl->logical.allocation_callbacks, &(tgt._impl->tex.view));
        assert(viewAllocationResult == VK_SUCCESS && "Image view allocation for render target failed.");
    }

    void render_graph::release_resources()
    {
        for (const auto& frameSync : _impl->frameData)
        {
            _impl->funcs.destroySemaphore(frameSync.imageReady, _impl->logical.allocation_callbacks);
            _impl->funcs.destroySemaphore(frameSync.renderComplete, _impl->logical.allocation_callbacks);
            _impl->funcs.destroyFence(frameSync.renderFence, _impl->logical.allocation_callbacks);
        }

        for (const auto& img : _impl->swapchainImages)
        {
            auto& tex = img._impl->tex;
            _impl->funcs.destroyImageView(tex.view, _impl->logical.allocation_callbacks);
        }

        for (const auto& rt : _impl->targets)
        {
            auto& tex = rt._impl->tex;
            _impl->funcs.destroyImageView(tex.view, _impl->logical.allocation_callbacks);
            vmaDestroyImage(_impl->allocator, tex.image, tex.imageAllocation);
        }

        vkb::destroy_swapchain(_impl->swap);
        vkb::destroy_surface(_impl->instance, _impl->surface);
        vmaDestroyAllocator(_impl->allocator);
        vkb::destroy_device(_impl->logical);
        vkb::destroy_instance(_impl->instance);
    }

    render_target::render_target()
    {
        _impl = new impl();
    }
    
    const string& render_target::name() const noexcept
    {
        return _impl->name;
    }
    
    u32 render_target::width() const noexcept
    {
        return _impl->width;
    }
    
    u32 render_target::height() const noexcept
    {
        return _impl->height;
    }
    
    data_format render_target::format() const noexcept
    {
        return as<data_format>(_impl->fmt);
    }

    pass_builder::pass_builder(const string& name)
        : _name(name)
    {
    }

    pass_builder& pass_builder::add_color_target(const render_target_usage& usage)
    {
        _colors.push_back(usage);
        return *this;
    }
    pass_builder& pass_builder::set_depth_target(const render_target_usage& usage)
    {
        _depth = usage;
        return *this;
    }

    pass_builder& pass_builder::depends_on(const render_resource_handle<pass>& p)
    {
        _dependsOn.push_back(p);
        return *this;
    }

    pass_builder& pass_builder::on_pass_execute(move_only_function<void(command_buffer&)>&& commands)
    {
        _commands = ryujin::move(commands);
        return *this;
    }
}