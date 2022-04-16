#ifndef render_manager_hpp__
#define render_manager_hpp__

#include "renderable.hpp"
#include "types.hpp"
#include "window.hpp"

#include "pipelines/base_render_pipeline.hpp"

#include "../core/linear_allocator.hpp"
#include "../core/vector.hpp"
#include "../core/span.hpp"

#include <VkBootstrap.h>
#include <vma/vk_mem_alloc.h>

#include <array>
#include <atomic>
#include <deque>
#include <functional>
#include <memory>
#include <optional>
#include <unordered_map>

namespace ryujin
{
    struct buffer_region
    {
        buffer buf;
        std::size_t offset;
        std::size_t range;
    };

    template <typename T>
    concept render_pipeline_type = std::is_base_of_v<base_render_pipeline, T> && std::is_default_constructible_v<T>;

    class render_manager;

    class command_list
    {
    public:
        command_list() = default;
        void begin();
        void end();
        void submit(const submit_info& info, const fence f = nullptr);

        void barrier(pipeline_stage src, pipeline_stage dst, const span<memory_barrier>& memBarriers, const span<buffer_memory_barrier>& bufMemBarriers, const span<image_memory_barrier>& imgMemBarriers);

        operator bool() const noexcept;

        std::uint32_t queue_index() const noexcept;
    protected:
        command_list(VkCommandBuffer cmdBuffer, vkb::DispatchTable& fns, VkQueue target, std::uint32_t queueIndex);

        friend class render_manager;

        VkCommandBuffer _buffer = VK_NULL_HANDLE;
        vkb::DispatchTable* _funcs;
        VkQueue _target = VK_NULL_HANDLE;
        std::uint32_t _queueIndex = 0;
    };

    class graphics_command_list : public command_list
    {
    public:
        ~graphics_command_list() = default;

        void begin_render_pass(const render_pass_begin_info& begin);
        void end_render_pass();

        void bind_graphics_pipeline(const pipeline& pipeline);
        void bind_graphics_descriptor_sets(const pipeline_layout& layout, const span<descriptor_set>& sets, const std::uint32_t firstSet = 0, const span<std::uint32_t>& offsets = {});
        void bind_index_buffer(const buffer& buf, const std::size_t offset = 0);
        void bind_vertex_buffers(const std::size_t first, const span<buffer>& buffers, const span<std::size_t>& offsets = {});

        void draw_arrays(const std::uint32_t count, const std::uint32_t instances = 1, const std::uint32_t firstVertex = 0, const std::uint32_t firstInstance = 0);
        void draw_indexed_indirect(const buffer& indirect, const std::size_t indirectOffset, const buffer& count, const std::size_t countOffset, const std::size_t maxDrawCount, const std::size_t stride);

        void set_viewports(const span<viewport>& viewports);
        void set_scissors(const span<scissor_region>& scissors);

    private:
        graphics_command_list(VkCommandBuffer cmdBuffer, vkb::DispatchTable& fns, VkQueue target, std::uint32_t queueIndex);

        friend class render_manager;
    };

    class transfer_command_list : public command_list
    {
    public:
        ~transfer_command_list() = default;

        void copy(const buffer& src, const buffer& dst, const span<buffer_copy_regions>& regions);
        void copy(const buffer& src, const image& dst, const image_layout layout, const span<buffer_image_copy_regions>& regions);

    private:
        transfer_command_list(VkCommandBuffer cmdBuffer, vkb::DispatchTable& fns, VkQueue target, std::uint32_t queueIndex);

        friend class render_manager;
    };

    class descriptor_allocator
    {
    public:
        enum class error_code
        {
            NO_ERROR,
            SET_ALLOCATION_FAILURE,
            POOL_ALLOCATION_FAILURE
        };

        descriptor_allocator() = default;
        explicit descriptor_allocator(render_manager* manager);

        void reset();
        result<descriptor_set, error_code> allocate(descriptor_set_layout layout);
        void clean_up();

    private:
        friend class render_manager;

        descriptor_pool create_pool(const std::size_t count);
        descriptor_pool fetch_pool();

        render_manager* _manager = {};
        vector<descriptor_pool> _used;
        vector<descriptor_pool> _free;
        descriptor_pool _current = {};
    };

    class descriptor_layout_cache
    {
    public:
        enum class error_code
        {
            NO_ERROR,
            ALLOCATION_FAILURE
        };

        explicit descriptor_layout_cache(render_manager& manager);

        result<descriptor_set_layout, error_code> allocate(const descriptor_set_layout_create_info& info);

        void flush();

        struct descriptor_layout_cache_entry
        {
            std::uint32_t count;
            std::array<descriptor_set_layout_binding, 32> bindings;

            inline constexpr bool operator==(const descriptor_layout_cache_entry& rhs) const noexcept
            {
                if (count != rhs.count)
                {
                    return false;
                }

                for (auto i = 0u; i < count; ++i)
                {
                    const auto bEquals = bindings[i].binding == rhs.bindings[i].binding;
                    const auto tEquals = bindings[i].type == rhs.bindings[i].type;
                    const auto cEquals = bindings[i].count == rhs.bindings[i].count;
                    const auto sEquals = bindings[i].stages == rhs.bindings[i].stages;

                    if (!(bEquals && tEquals && cEquals && sEquals))
                    {
                        return false;
                    }
                }

                return true;
            }

            inline constexpr bool operator!=(const descriptor_layout_cache_entry& rhs) const noexcept
            {
                return !(*this == rhs);
            }
        };

    private:

        struct descriptor_layout_binding_hasher
        {
            std::size_t operator()(const descriptor_set_layout_binding& key) const noexcept;
        };

        struct descriptor_layout_cache_hasher
        {
            std::size_t operator()(const descriptor_layout_cache_entry& key) const noexcept;
        };

        friend class render_manager;

        render_manager& _manager;
        std::unordered_map<descriptor_layout_cache_entry, descriptor_set_layout, descriptor_layout_cache_hasher> _cache;
    };

    class descriptor_writer
    {
    public:
        descriptor_writer(render_manager* manager);

        descriptor_writer() = default;
        descriptor_writer(const descriptor_writer&) = delete;
        descriptor_writer(descriptor_writer&& writer) noexcept;
        ~descriptor_writer() = default;

        descriptor_writer& operator=(const descriptor_writer&) = delete;
        descriptor_writer& operator=(descriptor_writer&& rhs) noexcept;
        
        descriptor_writer& write_buffer(const descriptor_set set, const std::uint32_t binding, const descriptor_type type, const std::uint32_t element,
            const span<descriptor_buffer_info>& buffers);

        descriptor_writer& write_image(const descriptor_set set, const std::uint32_t binding, const descriptor_type type, const std::uint32_t element,
            const span<descriptor_image_info>& images);

    private:

        void write();

        render_manager* _manager;
        static_vector<descriptor_write_info> _writes;
        static_vector<descriptor_buffer_info, 128> _buffers;
        static_vector<descriptor_buffer_info, 128> _images;
    };

    class render_manager
    {
        struct cached_command_pool
        {
            VkCommandPool pool = VK_NULL_HANDLE;
            vector<VkCommandBuffer> buffers;
            std::int32_t fetchIndex = 0;
            VkQueue queue;
            std::uint32_t queueIndex;
        };

        struct DeletionQueue
        {
            std::deque<std::function<void()>> deletors;

            void push_deletor(std::function<void()>&& fn);
            void flush();
        };

        struct per_frame_in_flight_resource
        {
            VkSemaphore present = VK_NULL_HANDLE;
            VkSemaphore render = VK_NULL_HANDLE;
            VkFence renderFence = VK_NULL_HANDLE;

            cached_command_pool compute;
            cached_command_pool graphics;
            cached_command_pool transfer;

            descriptor_allocator descriptorAllocator;

            DeletionQueue dtorQueue;
        };

        struct staging_buffer_alloc_info
        {
            buffer buf;
            std::size_t offset;
            std::size_t size;
        };

    public:
        enum class error_code
        {
            NO_ERROR,
            SURFACE_ACQUISITION_FAILURE,
            DEVICE_QUEUE_ACQUISITION_FAILURE,
            SWAPCHAIN_INITIALIZATION_FAILURE,
            SWAPCHAIN_IMAGE_ACQUISITION_FAILURE,
            PRESENT_FAILURE,
            RESOURCE_ALLOCATION_FAILURE,
            RESOURCE_ALLOCATION_FROM_POOL_FAILURE,
            OUT_OF_MEMORY,
            ILLEGAL_ARGUMENT
        };

        static result<std::unique_ptr<render_manager>, error_code> create(const std::unique_ptr<window>& win, vkb::Instance instance, vkb::Device device, VmaAllocator allocator, const bool nameObjects, registry& reg);

        render_manager(const render_manager&) = delete;
        render_manager(render_manager&&) noexcept = delete;
        ~render_manager();

        render_manager& operator=(const render_manager&) = delete;
        render_manager& operator=(render_manager&&) noexcept = delete;

        error_code pre_render();
        error_code render();

        std::uint32_t get_swapchain_image_count() const noexcept;
        result<image_view, error_code> get_swapchain_image(const std::uint32_t index) const noexcept;
        image_view get_swapchain_image() const noexcept;
        data_format get_swapchain_format() const noexcept;
        std::uint32_t get_swapchain_width() const noexcept;
        std::uint32_t get_swapchain_height() const noexcept;
        std::uint32_t get_frame_in_flight() const noexcept;
        std::uint32_t get_frames_in_flight() const noexcept;

        result<buffer, error_code> create(const buffer_create_info& bufferInfo, const allocation_create_info& allocInfo);
        result<descriptor_pool, error_code> create(const descriptor_pool_create_info& info);
        result<descriptor_set_layout, error_code> create(const descriptor_set_layout_create_info& info);
        result<fence, error_code> create(const fence_create_info& info);
        result<frame_buffer, error_code> create(const frame_buffer_create_info& framebufferInfo);
        result<image, error_code> create(const image_create_info& imageInfo, const allocation_create_info& allocInfo);
        result<image_view, error_code> create(const image_view_create_info& info);
        result<pipeline, error_code> create(const graphics_pipeline_create_info& info);
        result<pipeline_layout, error_code> create(const pipeline_layout_create_info& info);
        result<render_pass, error_code> create(const render_pass_create_info& info);
        result<image_sampler, error_code> create(const sampler_create_info& info);
        result<shader_module, error_code> create(const shader_module_create_info& info);

        result<descriptor_set, error_code> allocate_transient(const descriptor_set_layout& layout);

        result<buffer_region, error_code> write_to_staging_buffer(const void* data, const std::size_t bytes);
        void reset_staging_buffer();

        void write(const span<descriptor_write_info>& infos);

        void release(const buffer buf, const bool immediate = false);
        void release(const descriptor_pool pool, const bool immediate = false);
        void release(const descriptor_set_layout layout, const bool immediate = false);
        void release(const fence f, const bool immediate = false);
        void release(const frame_buffer fbo, const bool immediate = false);
        void release(const image img, const bool immediate = false);
        void release(const image_view view, const bool immediate = false);

        void reset(const descriptor_pool pool);
        void reset(const fence f);

        graphics_command_list next_graphics_command_list();
        transfer_command_list next_transfer_command_list();

        semaphore swapchain_image_ready_signal() const noexcept;
        semaphore render_complete_signal() const noexcept;
        fence flight_complete_fence() const noexcept;

        renderable_manager& renderables() noexcept;
        
        template <render_pipeline_type T>
        void use_render_pipeline();

        void wait(const fence& f);
    private:
        render_manager(const std::unique_ptr<window>& win, vkb::Instance instance, vkb::Device device, VmaAllocator allocator, const bool nameObjects, registry* reg);

        error_code create_surface();
        error_code create_swapchain();
        error_code recreate_swapchain();
        error_code fetch_queues();
        error_code build_resources_per_frame_in_flight();
        error_code build_staging_buffers();

        error_code rebuild_swapchain();
        void release_resources();

        void reset(cached_command_pool& pool);

        result<descriptor_set_layout, error_code> _create(const descriptor_set_layout_create_info& info);
        result<descriptor_set, error_code> _allocate(const descriptor_set_allocate_info& info);

        const std::unique_ptr<window>& _win;
        vkb::Device _device;
        vkb::Instance _instance;
        VmaAllocator _allocator;
        const bool _nameObjects;

        vkb::DispatchTable _funcs = {};
        VkSurfaceKHR _surface = VK_NULL_HANDLE;
        vkb::Swapchain _swapchain = {};

        std::uint32_t _currentFrame = 0;
        std::uint32_t _framesInFlight = 2;
        std::uint32_t _swapchainImageIndex = 0;

        std::optional<VkAllocationCallbacks> _allocationCallbacks;

        vector<per_frame_in_flight_resource> _resourcesPerFrameInFlight;
        vector<VkImageView> _swapchainImages;

        vector<buffer> _buffers;
        vector<descriptor_set_layout> _descriptorLayouts;
        vector<descriptor_pool> _descriptorPools;
        vector<fence> _fences;
        vector<frame_buffer> _frameBuffers;
        vector<image> _images;
        vector<image_view> _imageViews;
        vector<pipeline> _pipelines;
        vector<pipeline_layout> _layouts;
        vector<render_pass> _renderPasses;
        vector<image_sampler> _samplers;
        vector<shader_module> _shaderModules;

        static_vector<staging_buffer_alloc_info, 1> _stagingBuffers;

        VkQueue _graphics = VK_NULL_HANDLE;
        VkQueue _transfer = VK_NULL_HANDLE;
        VkQueue _compute = VK_NULL_HANDLE;
        VkQueue _present = VK_NULL_HANDLE;

        friend class descriptor_allocator;
        friend class descriptor_layout_cache;
        descriptor_layout_cache _descriptorLayoutCache;

        inline_linear_allocator<256 * 1024> _inlineScratchBuffer;

        std::unique_ptr<base_render_pipeline> _renderer;

        std::atomic_bool _isMinimized = false;

        renderable_manager _renderables;

        inline VkAllocationCallbacks* get_allocation_callbacks()
        {
            return _allocationCallbacks.has_value() ? &_allocationCallbacks.value() : VK_NULL_HANDLE;
        }

        inline per_frame_in_flight_resource& get_current_frame_resources() noexcept
        {
            return _resourcesPerFrameInFlight[_currentFrame];
        }

        inline const per_frame_in_flight_resource& get_current_frame_resources() const noexcept
        {
            return _resourcesPerFrameInFlight[_currentFrame];
        }
    };
    
    template<render_pipeline_type T>
    inline void render_manager::use_render_pipeline()
    {
        _renderer = std::unique_ptr<base_render_pipeline>(new T());
        _renderer->set_render_manager(this);
    }
}

#endif // render_manager_hpp__
