#include <ryujin/graphics/render_system.hpp>

#include <ryujin/core/engine.hpp>
#include <ryujin/core/primitives.hpp>

#undef APIENTRY
#include <spdlog/spdlog.h>

namespace ryujin
{
	namespace detail
	{
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
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
	}

	render_system::render_system()
	{
		vkb::InstanceBuilder instanceBldr;
		instanceBldr.set_app_name("Ryujin Application")
			.set_app_version(VK_MAKE_VERSION(0, 0, 1))
			.set_engine_name("Ryujin Engine")
			.set_engine_version(VK_MAKE_VERSION(0, 0, 1))
			.require_api_version(VK_MAKE_VERSION(1, 2, 0));

#ifdef _ENABLE_API_DUMP
		instanceBldr.enable_layer("VK_LAYER_LUNARG_api_dump");
#endif

#ifdef _DEBUG
		instanceBldr.request_validation_layers()
			.set_debug_callback(detail::debugCallback)
			.set_debug_messenger_severity(VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
			.set_debug_messenger_type(VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
			.add_validation_feature_enable(VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT)
			.add_validation_feature_enable(VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT);
		const auto sysInfoResult = vkb::SystemInfo::get_system_info();
		if (sysInfoResult)
		{
			if (sysInfoResult->is_extension_available(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
			{
				instanceBldr.enable_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
				spdlog::info("Vulkan extension {} requested.", VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
				_shouldNameObjects = true;
			}
			else
			{
				spdlog::warn("Vulkan extension {} required, but not available. Extension not requested.", VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			}
		}
		else
		{
			spdlog::warn("Vulkan extension {} required, but failed to determine if it is available. Extension not requested.", VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
#endif

		const auto result = instanceBldr.build();
		if (!result)
		{
			spdlog::critical("Failed to create VkInstance.");
			exit(-1);
		}

		_instance = result.value();
		spdlog::info("Successfully created VkInstance.");

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

		vkb::PhysicalDeviceSelector deviceSelector{ _instance };
		deviceSelector.prefer_gpu_device_type(vkb::PreferredDeviceType::discrete)
			.set_required_features(feats)
			.set_required_features_12(feats12)
			.require_present()
			.defer_surface_initialization();

		const auto gpuResult = deviceSelector.select();
		if (!gpuResult)
		{
			spdlog::critical("Failed to find suitable VkPhysicalDevice.");
			exit(-1);
		}

		const auto gpu = gpuResult.value();
		
		spdlog::info("Successfully selected VkPhysicalDevice:{} - {}", gpu.properties.deviceID, gpu.properties.deviceName);

		vkb::DeviceBuilder deviceBuilder{ gpu };
		const auto deviceResult = deviceBuilder.build();
		if (!deviceResult)
		{
			spdlog::critical("Failed to create VkDevice.");
			exit(-1);
		}

		spdlog::info("Successfully created VkDevice from VkPhysicalDevice:{}", gpu.properties.deviceID);
		_device = deviceResult.value();

		VmaVulkanFunctions fns = {};
		fns.vkGetDeviceProcAddr = _device.fp_vkGetDeviceProcAddr;
		fns.vkGetInstanceProcAddr = _instance.fp_vkGetInstanceProcAddr;
		fns.vkAllocateMemory = reinterpret_cast<PFN_vkAllocateMemory>(_device.fp_vkGetDeviceProcAddr(_device.device, "vkAllocateMemory"));
		fns.vkBindBufferMemory = reinterpret_cast<PFN_vkBindBufferMemory>(_device.fp_vkGetDeviceProcAddr(_device.device, "vkBindBufferMemory"));
		fns.vkBindBufferMemory2KHR = reinterpret_cast<PFN_vkBindBufferMemory2KHR>(_device.fp_vkGetDeviceProcAddr(_device.device, "vkBindBufferMemory2"));
		fns.vkBindImageMemory = reinterpret_cast<PFN_vkBindImageMemory>(_device.fp_vkGetDeviceProcAddr(_device.device, "vkBindImageMemory"));
		fns.vkBindImageMemory2KHR = reinterpret_cast<PFN_vkBindImageMemory2KHR>(_device.fp_vkGetDeviceProcAddr(_device.device, "vkBindImageMemory2"));
		fns.vkCmdCopyBuffer = reinterpret_cast<PFN_vkCmdCopyBuffer>(_device.fp_vkGetDeviceProcAddr(_device.device, "vkCmdCopyBuffer"));
		fns.vkCreateBuffer = reinterpret_cast<PFN_vkCreateBuffer>(_device.fp_vkGetDeviceProcAddr(_device.device, "vkCreateBuffer"));
		fns.vkCreateImage = reinterpret_cast<PFN_vkCreateImage>(_device.fp_vkGetDeviceProcAddr(_device.device, "vkCreateImage"));
		fns.vkDestroyBuffer = reinterpret_cast<PFN_vkDestroyBuffer>(_device.fp_vkGetDeviceProcAddr(_device.device, "vkDestroyBuffer"));
		fns.vkDestroyImage = reinterpret_cast<PFN_vkDestroyImage>(_device.fp_vkGetDeviceProcAddr(_device.device, "vkDestroyImage"));
		fns.vkFlushMappedMemoryRanges = reinterpret_cast<PFN_vkFlushMappedMemoryRanges>(_device.fp_vkGetDeviceProcAddr(_device.device, "vkFlushMappedMemoryRanges"));
		fns.vkFreeMemory = reinterpret_cast<PFN_vkFreeMemory>(_device.fp_vkGetDeviceProcAddr(_device.device, "vkFreeMemory"));
		fns.vkGetBufferMemoryRequirements = reinterpret_cast<PFN_vkGetBufferMemoryRequirements>(_device.fp_vkGetDeviceProcAddr(_device.device, "vkGetBufferMemoryRequirements"));
		fns.vkGetBufferMemoryRequirements2KHR = reinterpret_cast<PFN_vkGetBufferMemoryRequirements2KHR>(_device.fp_vkGetDeviceProcAddr(_device.device, "vkGetBufferMemoryRequirements2"));
		fns.vkGetImageMemoryRequirements = reinterpret_cast<PFN_vkGetImageMemoryRequirements>(_device.fp_vkGetDeviceProcAddr(_device.device, "vkGetImageMemoryRequirements"));
		fns.vkGetImageMemoryRequirements2KHR = reinterpret_cast<PFN_vkGetImageMemoryRequirements2KHR>(_device.fp_vkGetDeviceProcAddr(_device.device, "vkGetImageMemoryRequirements2"));
		fns.vkGetPhysicalDeviceMemoryProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties>(_instance.fp_vkGetInstanceProcAddr(_instance.instance, "vkGetPhysicalDeviceMemoryProperties"));
		fns.vkGetPhysicalDeviceMemoryProperties2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties2KHR>(_instance.fp_vkGetInstanceProcAddr(_instance.instance, "vkGetPhysicalDeviceMemoryProperties2"));
		fns.vkGetPhysicalDeviceProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(_instance.fp_vkGetInstanceProcAddr(_instance.instance, "vkGetPhysicalDeviceProperties"));
		fns.vkInvalidateMappedMemoryRanges = reinterpret_cast<PFN_vkInvalidateMappedMemoryRanges>(_device.fp_vkGetDeviceProcAddr(_device.device, "vkInvalidateMappedMemoryRanges"));
		fns.vkMapMemory = reinterpret_cast<PFN_vkMapMemory>(_device.fp_vkGetDeviceProcAddr(_device.device, "vkMapMemory"));
		fns.vkUnmapMemory = reinterpret_cast<PFN_vkUnmapMemory>(_device.fp_vkGetDeviceProcAddr(_device.device, "vkUnmapMemory"));

		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.device = _device.device;
		allocatorInfo.physicalDevice = gpu.physical_device;
		allocatorInfo.instance = _instance.instance;
		allocatorInfo.pVulkanFunctions = &fns;

		const auto allocatorResult = vmaCreateAllocator(&allocatorInfo, &_allocator);
		if (allocatorResult != VK_SUCCESS)
		{
			spdlog::critical("Failed to create memory allocator.");
			exit(-1);
		}
		spdlog::info("Successfully created memory allocator.");
	}

	render_system::~render_system()
	{
		_managers.clear();
		vmaDestroyAllocator(_allocator);
		vkb::destroy_device(_device);
		vkb::destroy_instance(_instance);
	}

	void render_system::on_init(engine_context& ctx)
	{
		for (auto& [name, win] : ctx.get_windows())
		{
			auto managerResult = render_manager::create(win, _instance, _device, _allocator, _shouldNameObjects, ctx.get_registry());

			if (managerResult)
			{
				const auto idx = _managers.size();
				_managers.push_back(ryujin::move(managerResult.success()));
			}
			else
			{
				// TODO: Log an error message
			}
		}
	}

	void render_system::render_prework(engine_context& ctx)
	{
		// Pre-rendering work on main thread
		for (auto& pipeline : _managers)
		{
			pipeline->pre_render();
		}
	}
	
	void render_system::on_pre_render(engine_context& ctx)
	{
		// TODO: Pre-rendering work, on render thread
	}

	void render_system::on_render(engine_context& ctx)
	{
		for (auto& pipeline : _managers)
		{
			pipeline->render();
		}
	}

	void render_system::on_post_render(engine_context& ctx)
	{
		for (auto& pipeline : _managers)
		{
			pipeline->end_frame();
		}
	}

	unique_ptr<render_manager>& render_system::get_render_manager(sz idx) noexcept
	{
		return _managers[idx];
	}
	
	sz render_system::render_manager_count() const noexcept
	{
		return _managers.size();
	}
}