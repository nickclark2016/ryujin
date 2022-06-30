#include <ryujin/render_graph.hpp>

#include <ryujin/unordered_map.hpp>

#include <VkBootstrap.h>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#undef APIENTRY
#include <spdlog/spdlog.h>

namespace ryujin
{
    namespace detail
    {
        static constexpr bool enable_vulkan_debug_apis_enable()
        {
#if defined(_DEBUG) || defined(RYUJIN_VULKAN_API_DEBUG)
            return true;
#else
            return false;
#endif
        }

        static constexpr bool enable_vulkan_api_dump()
        {
#if defined(RYUJIN_VULKAN_API_TRACE_ENABLE)
            return true;
#else
            return false;
#endif
        }

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

        
    }

    class vulkan_render_graph;

    class vulkan_render_pass final : public render_pass
    {
    public:
        vulkan_render_pass(render_pass_create_info&& info, vkb::DispatchTable* fns)
            : _fns(fns)
        {
            vector<VkAttachmentDescription> attachments;
            vector<VkSubpassDescription> subpasses;
            vector<VkSubpassDependency> dependencies;
            vector<VkAttachmentReference> attachmentRefs;

            attachments.reserve(info.attachments.length());
            subpasses.reserve(info.subpasses.length());
            dependencies.reserve(info.dependencies.length());

            sz totalAttachmentRefCount = 0;
            u32 attachmentCount = 0;

            for (const auto& sub : info.subpasses)
            {
                totalAttachmentRefCount += sub.colors.length();
                totalAttachmentRefCount += sub.inputs.length();
                totalAttachmentRefCount += sub.depth.length();

                assert(sub.depth.length() <= 1 && "Subpass may define at most one depth attachment reference.");

                for (auto color : sub.colors)
                {
                    attachmentCount = ryujin::max(attachmentCount, color.index);
                }

                for (auto input : sub.inputs)
                {
                    attachmentCount = ryujin::max(attachmentCount, input.index);
                }

                for (auto depth : sub.depth)
                {
                    attachmentCount = ryujin::max(attachmentCount, depth.index);
                }
            }

            // compute last usage of each attachment
            vector<u32> lastUsage(attachmentCount, 0);

            for (sz i = 0; i < info.subpasses.length(); ++i)
            {
                auto& sub = info.subpasses[i];
                for (auto color : sub.colors)
                {
                    lastUsage[color.index] = ryujin::max(lastUsage[color.index], as<u32>(i));
                }

                for (auto input : sub.inputs)
                {
                    lastUsage[input.index] = ryujin::max(lastUsage[input.index], as<u32>(i));
                }

                for (auto depth : sub.depth)
                {
                    lastUsage[depth.index] = ryujin::max(lastUsage[depth.index], as<u32>(i));
                }
            }

            const sz maxPreserveAttachmentCount = totalAttachmentRefCount - attachmentCount;

            vector<u32> preserveIndices;
            preserveIndices.reserve(maxPreserveAttachmentCount);

            vector<u32> attachmentsUsed;
            attachmentsUsed.reserve(attachmentCount);

            for (sz i = 0; i < info.subpasses.length(); ++i)
            {
                const auto& subpass = info.subpasses[i];

                const auto colorStartIndex = attachmentRefs.size();
                for (const auto color : subpass.colors)
                {
                    attachmentRefs.push_back(VkAttachmentReference{
                            .attachment = color.index,
                            .layout = as<VkImageLayout>(color.layout)
                        });

                    attachmentsUsed.push_back(color.index);
                }

                const auto inputStartIndex = attachmentRefs.size();
                for (const auto input : subpass.inputs)
                {
                    attachmentRefs.push_back(VkAttachmentReference{
                            .attachment = input.index,
                            .layout = as<VkImageLayout>(input.layout)
                        });

                    attachmentsUsed.push_back(input.index);
                }

                const auto depthIndex = attachmentRefs.size();
                if (subpass.depth)
                {
                    attachmentRefs.push_back(VkAttachmentReference{
                            .attachment = subpass.depth[0].index,
                            .layout = as<VkImageLayout>(subpass.depth[0].layout)
                        });

                    attachmentsUsed.push_back(subpass.depth[0].index);
                }

                u32 unusedAttachmentCount = 0;
                const auto preserveStartIndex = preserveIndices.size();

                for (u32 attachment = 0; attachment < attachmentCount; ++attachment)
                {
                    for (const auto& usedAttachmentIndex : attachmentsUsed)
                    {
                        if (attachment == usedAttachmentIndex)
                        {
                            goto used;
                        }
                    }

                    if (i <= lastUsage[attachment])
                    {
                        ++unusedAttachmentCount;
                        preserveIndices.push_back(attachment);
                    }
                used:;
                }

                const VkSubpassDescription desc = {
                    .flags = 0,
                    .pipelineBindPoint = as<VkPipelineBindPoint>(subpass.type),
                    .inputAttachmentCount = as<u32>(subpass.inputs.length()),
                    .pInputAttachments = subpass.inputs ? attachmentRefs.data() + inputStartIndex : VK_NULL_HANDLE,
                    .colorAttachmentCount = as<u32>(subpass.colors.length()),
                    .pColorAttachments = subpass.colors ? attachmentRefs.data() + colorStartIndex : VK_NULL_HANDLE,
                    .pResolveAttachments = VK_NULL_HANDLE,
                    .pDepthStencilAttachment = subpass.depth ? attachmentRefs.data() + depthIndex : VK_NULL_HANDLE,
                    .preserveAttachmentCount = unusedAttachmentCount,
                    .pPreserveAttachments = unusedAttachmentCount > 0 ? preserveIndices.data() + preserveStartIndex : VK_NULL_HANDLE
                };

                subpasses.push_back(desc);

                attachmentsUsed.clear();
            }

            for (const auto& attachment : info.attachments)
            {
                const VkAttachmentDescription desc = {
                    .flags = 0,
                    .format = as<VkFormat>(attachment.fmt),
                    .samples = as<VkSampleCountFlagBits>(attachment.samples),
                    .loadOp = as<VkAttachmentLoadOp>(attachment.load),
                    .storeOp = as<VkAttachmentStoreOp>(attachment.store),
                    .stencilLoadOp = as<VkAttachmentLoadOp>(attachment.stencilLoad),
                    .stencilStoreOp = as<VkAttachmentStoreOp>(attachment.stencilStore),
                    .initialLayout = as<VkImageLayout>(attachment.input),
                    .finalLayout = as<VkImageLayout>(attachment.output)
                };

                attachments.push_back(desc);
            }

            const auto stageMaskMerger = [](const span<pipeline_stage>& stages) {
                VkPipelineStageFlags flags = 0;
                for (const auto& stage : stages)
                    flags |= as<VkPipelineStageFlags>(stage);
                return flags;
            };

            const auto accessMaskMerger = [](const span<memory_access>& accesses) {
                VkAccessFlags flags = 0;
                for (const auto& access : accesses)
                    flags |= as<VkAccessFlags>(access);
                return flags;
            };

            for (const auto& dependency : info.dependencies)
            {


                const VkSubpassDependency dep = {
                    .srcSubpass = dependency.srcSubpassIndex,
                    .dstSubpass = dependency.dstSubpassIndex,
                    .srcStageMask = stageMaskMerger(dependency.srcStageMask),
                    .dstStageMask = stageMaskMerger(dependency.dstStageMask),
                    .srcAccessMask = accessMaskMerger(dependency.srcAccessMask),
                    .dstAccessMask = accessMaskMerger(dependency.dstAccessMask),
                    .dependencyFlags = 0 // investigate dependency by region
                };

                dependencies.push_back(dep);
            }

            const VkRenderPassCreateInfo create = {
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                .pNext = VK_NULL_HANDLE,
                .flags = 0,
                .attachmentCount = as<u32>(attachments.size()),
                .pAttachments = attachments.data(),
                .subpassCount = as<u32>(subpasses.size()),
                .pSubpasses = subpasses.data(),
                .dependencyCount = as<u32>(dependencies.size()),
                .pDependencies = dependencies.data()
            };

            const auto result = fns->createRenderPass(&create, VK_NULL_HANDLE, &_pass);
            if (result != VK_SUCCESS)
            {
                spdlog::error("Failed to create VkRenderPass instance.");
                return;
            }
            spdlog::info("Successfully created VkRenderPass instance.");
        }

        ~vulkan_render_pass()
        {
            _fns->destroyRenderPass(_pass, VK_NULL_HANDLE);
        }

        vulkan_render_pass(vulkan_render_pass&& rhs) noexcept
            : _pass(rhs._pass), _fns(rhs._fns), _cmds(ryujin::move(_cmds)), _handle(rhs._handle)
        {
            rhs._pass = VK_NULL_HANDLE;
        }

        vulkan_render_pass& operator=(vulkan_render_pass&& rhs) noexcept
        {
            if (&rhs == this)
            {
                return *this;
            }

            _fns->destroyRenderPass(_pass, VK_NULL_HANDLE);

            _pass = rhs._pass;
            _fns = rhs._fns;
            _cmds = ryujin::move(rhs._cmds);
            _handle = rhs._handle;

            rhs._pass = VK_NULL_HANDLE;

            return *this;
        }

        void execute(commands& cmds) override
        {
            _cmds(cmds);
        }

    private:
        friend class vulkan_render_graph;

        vkb::DispatchTable* _fns;

        move_only_function<void(commands&)> _cmds;

        VkRenderPass _pass = {};
        render_graph::handle _handle = invalid_slot_map_key;
    };

    class vulkan_render_graph final : public render_graph
    {
    public:
        vulkan_render_graph(const window& win)
        {
            assert(build_vulkan_instance());
            assert(select_vulkan_physical_device());
            assert(build_logical_device());
            assert(fetch_surface(win));
            assert(build_swapchain());
            assert(fetch_swapchain_images());
        }

        ~vulkan_render_graph() override
        {
            _passes.clear();

            for (auto view : _swapchainImageViews)
            {
                _funcs.destroyImageView(view, VK_NULL_HANDLE);
            }
            _swapchainImageViews.clear();

            vkb::destroy_swapchain(_swapchain);
            vkb::destroy_surface(_instance, _surface);
            vkb::destroy_device(_device);
            vkb::destroy_instance(_instance);
        }

        handle create_render_pass(render_pass_create_info&& info) override
        {
            _graphDirty = true;
            vulkan_render_pass p(ryujin::move(info), &_funcs);
            auto key = _passes.insert(ryujin::move(p));
            _passes.try_get(key)->_handle = key;
            return key;
        }

        void on_render_pass_execute(handle h, move_only_function<void(commands&)>&& cmds) override
        {
            auto pass = _passes.try_get(h);
            if (pass)
            {
                pass->_cmds = ryujin::move(cmds);
            }
        }

        void add_render_pass_dependency(const render_pass_dependency_info& info)
        {
            _deps.push_back(info);
            _graphDirty = true;
        }

        void execute() override
        {
            if (_graphDirty)
            {
                spdlog::info("Detected change in render dependency graph, rebuilding.");
                build_graph();
            }
        }

        render_target_format swapchain_image_format() const noexcept override
        {
            return as<render_target_format>(_swapchain.image_format);
        }

    private:
        slot_map<vulkan_render_pass> _passes;
        vector<render_pass_dependency_info> _deps;

        vkb::Instance _instance;
        vkb::PhysicalDevice _physical;
        vkb::Device _device;
        vkb::DispatchTable _funcs;
        VkSurfaceKHR _surface;
        vkb::Swapchain _swapchain = {};

        vector<VkImageView> _swapchainImageViews = {};

        bool _graphDirty = true;

        bool build_vulkan_instance()
        {
            vkb::InstanceBuilder bldr = vkb::InstanceBuilder()
                .require_api_version(VK_API_VERSION_1_2)
                .set_app_name("Ryujin Application")
                .set_engine_name("Ryujin Engine")
                .set_app_version(VK_MAKE_VERSION(0, 1, 0))
                .set_engine_version(VK_MAKE_VERSION(0, 1, 0));

            apply_debug_messenger(bldr);
            apply_api_dump(bldr);

            auto result = bldr.build();
            if (result)
            {
                _instance = *result;
                spdlog::info("Successfully created vkb::Instance.");
                return true;
            }
            
            spdlog::critical("Failed to create vkb::Instance.");
            return false;
        }

        bool select_vulkan_physical_device()
        {
            const VkPhysicalDeviceFeatures feats = {
                .independentBlend = VK_TRUE,
                .logicOp = VK_TRUE,
                .depthClamp = VK_TRUE,
                .depthBiasClamp = VK_TRUE,
                .fillModeNonSolid = VK_TRUE,
                .depthBounds = VK_TRUE,
                .alphaToOne = VK_TRUE,
            };

            const VkPhysicalDeviceVulkan12Features feats12 = {
                .drawIndirectCount = VK_TRUE,
                .imagelessFramebuffer = VK_TRUE,
                .separateDepthStencilLayouts = VK_TRUE,
            };

            vkb::PhysicalDeviceSelector selector = vkb::PhysicalDeviceSelector(_instance)
                .prefer_gpu_device_type(vkb::PreferredDeviceType::discrete)
                .defer_surface_initialization()
                .set_required_features(feats)
                .set_required_features_12(feats12)
                .require_present();

            auto result = selector.select();
            if (result)
            {
                _physical = *result;
                spdlog::info("Successfully selected vkb::PhysicalDevice {}.", _physical.properties.deviceName);
                return true;
            }

            spdlog::critical("Failed to detect a suitable vkb::PhysicalDevice.");
            return false;
        }

        bool build_logical_device()
        {
            vkb::DeviceBuilder bldr = vkb::DeviceBuilder(_physical);

            auto result = bldr.build();
            if (result)
            {
                spdlog::info("Successfully created vkb::Device.");
                _device = *result;
                _funcs = _device.make_table();
                return true;
            }

            spdlog::critical("Failed to create vkb::Device.");
            return false;
        }

        bool fetch_surface(const window& win)
        {
            const auto result = glfwCreateWindowSurface(_instance.instance, win._native, nullptr, &_surface);
            if (result == VK_SUCCESS)
            {
                spdlog::info("Successfully created VkSurfaceKHR.");
                return true;
            }
            spdlog::critical("Failed to create VkSurfaceKHR.");
            return false;
        }

        bool build_swapchain()
        {
            _device.surface = _surface;
            auto result = vkb::SwapchainBuilder(_device, _surface)
                .set_desired_present_mode(VK_PRESENT_MODE_IMMEDIATE_KHR)
                .build();
            
            if (result)
            {
                spdlog::info("Successfully created vkb::Swapchain.");
                _swapchain = *result;
                return true;
            }

            spdlog::critical("Failed to create vkb::Swapchain.");
            return false;
        }

        bool fetch_swapchain_images()
        {
            auto result = _swapchain.get_images();
            if (result)
            {
                spdlog::info("Successfully fetched VkImage instances from vkb::Swapchain.");

                for (const VkImage img : *result)
                {
                    const VkImageViewUsageCreateInfo uinfo = {
                        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO,
                        .pNext = VK_NULL_HANDLE,
                        .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
                    };

                    const VkImageViewCreateInfo cinfo = {
                        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                        .pNext = &uinfo,
                        .flags = 0,
                        .image = img,
                        .viewType = VK_IMAGE_VIEW_TYPE_2D,
                        .format = _swapchain.image_format,
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

                    VkImageView view = VK_NULL_HANDLE;
                    const auto viewRes = _funcs.createImageView(&cinfo, VK_NULL_HANDLE, &view);
                    if (viewRes != VK_SUCCESS)
                    {
                        spdlog::critical("Failed to create VkImageView from VkImage fetched from vkb::Swapchain.");
                        return false;
                    }
                    _swapchainImageViews.push_back(view);
                }
                spdlog::info("Successfully created {} VkImageView instances from VkImage instances fetched from vkb::Swapchain.", _swapchain.image_count);
                return true;
            }

            spdlog::critical("Failed to fetch VkImage instances from vkb::Swapchain.");
            return false;
        }

        void apply_debug_messenger(vkb::InstanceBuilder& bldr)
        {
            if (detail::enable_vulkan_debug_apis_enable())
            {
                bldr.request_validation_layers()
                    .set_debug_callback(detail::vulkan_debug_message_callback)
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
            }
        }

        void apply_api_dump(vkb::InstanceBuilder& bldr)
        {
            if (detail::enable_vulkan_api_dump())
            {
                bldr.enable_layer("VK_LAYER_LUNARG_api_dump");
            }
        }

        void build_graph()
        {
            /*
             * Precondtions:
             * - There are no cylces in the graph (TODO: Perform check in debug build for cycles)
             * Step 1: Build the graph structure
             * Step 2: Perform a topological sort on the graph structure
             * Step 3: Store the output to a vector that can be executed over
             */

            // store vertices in an adjacency list
            unordered_map<render_graph::handle, vector<render_graph::handle>, slot_map_key_hash> graph;
            for (const auto& pass : _passes)
            {
                graph[pass._handle] = {};
            }

            // store edges in an adjacency list
            for (const auto& dep : _deps)
            {
                const auto srcContained = graph.find(dep.src);
                const auto dstContained = graph.find(dep.dst);

                if (srcContained == graph.end() || dstContained == graph.end())
                {
                    spdlog::error("Failed to build graph. A dependency has been defined such that the source or the destination is not in the graph.");
                    return;
                }

                graph[dep.src].push_back(dep.dst);
            }

            // Perform topological sorting algorithm on graph to determine the order of execution
            unordered_map<render_graph::handle, bool, slot_map_key_hash> visited;
            vector<render_graph::handle> reverse;

            for (auto& [node, adj] : graph)
            {
                if (!visited[node])
                {
                    build_graph_utility(node, graph, visited, reverse);
                }
            }

            vector<render_graph::handle> sortedPasses;
            sortedPasses.reserve(reverse.size());

            while (!reverse.empty())
            {
                sortedPasses.push_back(reverse.back());
                reverse.pop_back();
            }

            _graphDirty = false;
        }

        void build_graph_utility(render_graph::handle curr, unordered_map<render_graph::handle, vector<render_graph::handle>, slot_map_key_hash> graph, unordered_map<render_graph::handle, bool, slot_map_key_hash>& visited, vector<render_graph::handle>& sorted)
        {
            visited[curr] = true;

            const auto& adjacencies = graph[curr];

            for (auto& adj : adjacencies)
            {
                if (!visited[adj])
                {
                    build_graph_utility(adj, graph, visited, sorted);
                }
            }

            sorted.push_back(curr);
        }
    };

    unique_ptr<render_graph> render_graph::create_render_graph(const window& win)
    {
        auto graph = make_unique<vulkan_render_graph>(win);
        return unique_ptr<render_graph>(graph.release());
    }
}