#ifndef render_manager_hpp__
#define render_manager_hpp__

#include "renderable.hpp"
#include "types.hpp"
#include "window.hpp"

#include "pipelines/base_render_pipeline.hpp"

#include "../core/export.hpp"
#include "../core/linear_allocator.hpp"
#include "../core/memory.hpp"
#include "../core/optional.hpp"
#include "../core/primitives.hpp"
#include "../core/vector.hpp"
#include "../core/span.hpp"

#include <VkBootstrap.h>
#include <vma/vk_mem_alloc.h>

#include <array>
#include <atomic>
#include <deque>
#include <functional>
#include <optional>
#include <unordered_map>

namespace ryujin
{
    struct buffer_region
    {
        buffer buf;
        sz offset;
        sz range;
    };

    template <typename T>
    concept render_pipeline_type = std::is_base_of_v<base_render_pipeline, T> && std::is_default_constructible_v<T>;

    class render_manager;

    class command_list
    {
    public:
        RYUJIN_API command_list() = default;
        RYUJIN_API void begin();
        RYUJIN_API void end();
        RYUJIN_API void submit(const submit_info& info, const fence f = nullptr);

        RYUJIN_API void barrier(pipeline_stage src, pipeline_stage dst, const span<memory_barrier>& memBarriers, const span<buffer_memory_barrier>& bufMemBarriers, const span<image_memory_barrier>& imgMemBarriers);
        RYUJIN_API void push_constants(const pipeline_layout& layout, const shader_stage stages, const u32 offset, const u32 size, const void* data);

        RYUJIN_API operator bool() const noexcept;

        RYUJIN_API u32 queue_index() const noexcept;
    protected:
        command_list(VkCommandBuffer cmdBuffer, vkb::DispatchTable& fns, VkQueue target, u32 queueIndex);

        friend class render_manager;

        VkCommandBuffer _buffer = VK_NULL_HANDLE;
        vkb::DispatchTable* _funcs;
        VkQueue _target = VK_NULL_HANDLE;
        u32 _queueIndex = 0;
    };

    class graphics_command_list : public command_list
    {
    public:
        RYUJIN_API ~graphics_command_list() = default;

        RYUJIN_API void begin_render_pass(const render_pass_begin_info& begin);
        RYUJIN_API void end_render_pass();

        RYUJIN_API void bind_graphics_pipeline(const pipeline& pipeline);
        RYUJIN_API void bind_graphics_descriptor_sets(const pipeline_layout& layout, const span<descriptor_set>& sets, const u32 firstSet = 0, const span<u32>& offsets = {});
        RYUJIN_API void bind_index_buffer(const buffer& buf, const sz offset = 0);
        RYUJIN_API void bind_vertex_buffers(const sz first, const span<buffer>& buffers, const span<sz>& offsets = {});

        RYUJIN_API void draw_arrays(const u32 count, const u32 instances = 1, const u32 firstVertex = 0, const u32 firstInstance = 0);
        RYUJIN_API void draw_indexed_indirect(const buffer& indirect, const sz indirectOffset, const buffer& count, const sz countOffset, const sz maxDrawCount, const sz stride);

        RYUJIN_API void set_viewports(const span<viewport>& viewports);
        RYUJIN_API void set_scissors(const span<scissor_region>& scissors);

    private:
        graphics_command_list(VkCommandBuffer cmdBuffer, vkb::DispatchTable& fns, VkQueue target, u32 queueIndex);

        friend class render_manager;
    };

    class transfer_command_list : public command_list
    {
    public:
        RYUJIN_API ~transfer_command_list() = default;

        RYUJIN_API void copy(const buffer& src, const buffer& dst, const span<buffer_copy_regions>& regions);
        RYUJIN_API void copy(const buffer& src, const image& dst, const image_layout layout, const span<buffer_image_copy_regions>& regions);

    private:
        transfer_command_list(VkCommandBuffer cmdBuffer, vkb::DispatchTable& fns, VkQueue target, u32 queueIndex);

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

        RYUJIN_API descriptor_allocator() = default;
        RYUJIN_API explicit descriptor_allocator(render_manager* manager);

        RYUJIN_API void reset();
        RYUJIN_API result<descriptor_set, error_code> allocate(descriptor_set_layout layout);
        RYUJIN_API void clean_up();

    private:
        friend class render_manager;

        descriptor_pool create_pool(const sz count);
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

        RYUJIN_API explicit descriptor_layout_cache(render_manager& manager);

        RYUJIN_API result<descriptor_set_layout, error_code> allocate(const descriptor_set_layout_create_info& info);

        RYUJIN_API void flush();

        struct descriptor_layout_cache_entry
        {
            u32 count;
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
            sz operator()(const descriptor_set_layout_binding& key) const noexcept;
        };

        struct descriptor_layout_cache_hasher
        {
            sz operator()(const descriptor_layout_cache_entry& key) const noexcept;
        };

        friend class render_manager;

        render_manager& _manager;
        std::unordered_map<descriptor_layout_cache_entry, descriptor_set_layout, descriptor_layout_cache_hasher> _cache;
    };

    class descriptor_writer
    {
    public:
        RYUJIN_API descriptor_writer(render_manager* manager);

        RYUJIN_API descriptor_writer() = default;
        RYUJIN_API descriptor_writer(const descriptor_writer&) = delete;
        RYUJIN_API descriptor_writer(descriptor_writer&& writer) noexcept;
        RYUJIN_API ~descriptor_writer() = default;

        RYUJIN_API descriptor_writer& operator=(const descriptor_writer&) = delete;
        RYUJIN_API descriptor_writer& operator=(descriptor_writer&& rhs) noexcept;
        
        RYUJIN_API descriptor_writer& write_buffer(const descriptor_set set, const u32 binding, const descriptor_type type, const u32 element,
            const span<descriptor_buffer_info>& buffers);

        RYUJIN_API descriptor_writer& write_image(const descriptor_set set, const u32 binding, const descriptor_type type, const u32 element,
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
            i32 fetchIndex = 0;
            VkQueue queue;
            u32 queueIndex;
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
            sz offset;
            sz size;
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

        RYUJIN_API static result<unique_ptr<render_manager>, error_code> create(const unique_ptr<window>& win, vkb::Instance instance, vkb::Device device, VmaAllocator allocator, const bool nameObjects, registry& reg);

        render_manager(const render_manager&) = delete;
        render_manager(render_manager&&) noexcept = delete;
        RYUJIN_API ~render_manager();

        render_manager& operator=(const render_manager&) = delete;
        render_manager& operator=(render_manager&&) noexcept = delete;

        RYUJIN_API error_code pre_render();

        RYUJIN_API error_code start_frame();
        RYUJIN_API error_code render();
        RYUJIN_API error_code end_frame();

        RYUJIN_API u32 get_swapchain_image_count() const noexcept;
        RYUJIN_API result<image_view, error_code> get_swapchain_image(const u32 index) const noexcept;
        RYUJIN_API image_view get_swapchain_image() const noexcept;
        RYUJIN_API data_format get_swapchain_format() const noexcept;
        RYUJIN_API u32 get_swapchain_width() const noexcept;
        RYUJIN_API u32 get_swapchain_height() const noexcept;
        RYUJIN_API u32 get_frame_in_flight() const noexcept;
        RYUJIN_API u32 get_frames_in_flight() const noexcept;

        RYUJIN_API result<buffer, error_code> create(const buffer_create_info& bufferInfo, const allocation_create_info& allocInfo);
        RYUJIN_API result<descriptor_pool, error_code> create(const descriptor_pool_create_info& info);
        RYUJIN_API result<descriptor_set_layout, error_code> create(const descriptor_set_layout_create_info& info);
        RYUJIN_API result<fence, error_code> create(const fence_create_info& info);
        RYUJIN_API result<frame_buffer, error_code> create(const frame_buffer_create_info& framebufferInfo);
        RYUJIN_API result<image, error_code> create(const image_create_info& imageInfo, const allocation_create_info& allocInfo);
        RYUJIN_API result<image_view, error_code> create(const image_view_create_info& info);
        RYUJIN_API result<pipeline, error_code> create(const graphics_pipeline_create_info& info);
        RYUJIN_API result<pipeline_layout, error_code> create(const pipeline_layout_create_info& info);
        RYUJIN_API result<render_pass, error_code> create(const render_pass_create_info& info);
        RYUJIN_API result<image_sampler, error_code> create(const sampler_create_info& info);
        RYUJIN_API result<shader_module, error_code> create(const shader_module_create_info& info);

        RYUJIN_API result<descriptor_set, error_code> allocate_transient(const descriptor_set_layout& layout);

        RYUJIN_API result<buffer_region, error_code> write_to_staging_buffer(const void* data, const sz bytes);
        RYUJIN_API void reset_staging_buffer();

        RYUJIN_API void write(const span<descriptor_write_info>& infos);

        RYUJIN_API void release(const buffer buf, const bool immediate = false);
        RYUJIN_API void release(const descriptor_pool pool, const bool immediate = false);
        RYUJIN_API void release(const descriptor_set_layout layout, const bool immediate = false);
        RYUJIN_API void release(const fence f, const bool immediate = false);
        RYUJIN_API void release(const frame_buffer fbo, const bool immediate = false);
        RYUJIN_API void release(const image img, const bool immediate = false);
        RYUJIN_API void release(const image_view view, const bool immediate = false);

        RYUJIN_API void reset(const descriptor_pool pool);
        RYUJIN_API void reset(const fence f);

        RYUJIN_API graphics_command_list next_graphics_command_list();
        RYUJIN_API transfer_command_list next_transfer_command_list();

        RYUJIN_API semaphore swapchain_image_ready_signal() const noexcept;
        RYUJIN_API semaphore render_complete_signal() const noexcept;
        RYUJIN_API fence flight_complete_fence() const noexcept;

        RYUJIN_API renderable_manager& renderables() noexcept;
        
        template <render_pipeline_type T>
        void use_render_pipeline();

        RYUJIN_API unique_ptr<base_render_pipeline>& get_render_pipeline() noexcept;

        RYUJIN_API void wait(const fence& f);
    private:
        render_manager(const unique_ptr<window>& win, vkb::Instance instance, vkb::Device device, VmaAllocator allocator, const bool nameObjects, registry* reg);

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

        const unique_ptr<window>& _win;
        vkb::Device _device;
        vkb::Instance _instance;
        VmaAllocator _allocator;
        const bool _nameObjects;

        vkb::DispatchTable _funcs = {};
        VkSurfaceKHR _surface = VK_NULL_HANDLE;
        vkb::Swapchain _swapchain = {};

        u32 _currentFrame = 0;
        u32 _framesInFlight = 2;
        u32 _swapchainImageIndex = 0;

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

        unique_ptr<base_render_pipeline> _renderer;

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
        _renderer = ryujin::unique_ptr<base_render_pipeline>(new T());
        _renderer->set_render_manager(this);
    }
}

#endif // render_manager_hpp__
