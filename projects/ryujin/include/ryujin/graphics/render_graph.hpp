#ifndef render_graph_hpp__
#define render_graph_hpp__

#include "types.hpp"

#include "../core/functional.hpp"
#include "../core/optional.hpp"
#include "../core/slot_map.hpp"
#include "../core/string.hpp"
#include "../core/vector.hpp"

namespace ryujin
{
    class window;

    class pass;
    class render_graph;
    class render_target;
    class shader;

    template <typename T>
    class render_resource_handle
    {
    public:
        T* get() const noexcept;

        T* operator->() const noexcept;
        T& operator*() noexcept;
        const T& operator*() const noexcept;

        operator bool() const noexcept;
    private:
        friend class render_graph;
        
        render_resource_handle() = default;

        const render_graph* _graph;
        slot_map_key _key;
    };

    class render_target_builder
    {
    public:
        explicit render_target_builder(const string& name);

        render_target_builder& width(const u32 w);
        render_target_builder& height(const u32 h);
        render_target_builder& format(const data_format fmt);
        render_target_builder& use_as_color_attachment();
        render_target_builder& use_as_depth_attachment();
        render_target_builder& use_as_input_attachment();

        bool validate() const noexcept;

    private:
        friend class render_graph;

        string _name;
        u32 _width = 0;
        u32 _height = 0;
        data_format _fmt = data_format::UNDEFINED;
        bool _color = false;
        bool _depth = false;
        bool _input = false;
    };

    class render_target
    {
    public:
        const string& name() const noexcept;
        u32 width() const noexcept;
        u32 height() const noexcept;
        data_format format() const noexcept;

    private:
        friend class render_graph;

        render_target();

        struct impl;

        impl* _impl = nullptr;
    };

    class command_buffer
    {
    };

    struct render_target_usage
    {
        render_resource_handle<render_target> tgt;
        attachment_load_op load;
        attachment_store_op store;
        clear_value clear; // ignored if load is CLEAR
    };

    class shader
    {
    public:
        shader(const shader&) = delete;
        shader(shader&& other) noexcept;
        ~shader();

        shader& operator=(const shader&) = delete;
        shader& operator=(shader&& rhs) noexcept;

        const string& name() const noexcept;

        bool has_vertex_stage() const noexcept;
        bool has_fragment_stage() const noexcept;

        bool depth_testing_enabled() const noexcept;
        bool depth_writing_enabled() const noexcept;

    private:
        struct impl;

        impl* _impl;
    };

    class shader_builder
    {
    public:
        shader_builder(const string& name);

        shader_builder& set_vertex_source(const vector<byte>& spirv);
        shader_builder& set_fragment_source(const vector<byte>& spirv);
        shader_builder& enable_depth_testing();
        shader_builder& enable_depth_writing();

        render_resource_handle<shader> build_for(const pass& p);
    };

    class pass
    {
    public:
        const string& name() const noexcept;

        void set_color_target(const u32 index, const render_target& tgt);
        void set_depth_target(const render_target& tgt);

    private:
        struct impl;
        
        impl* _impl = nullptr;
    };

    class pass_builder
    {
    public:
        pass_builder(const string& name);

        pass_builder& add_color_target(const render_target_usage& usage);
        pass_builder& set_depth_target(const render_target_usage& usage);
        pass_builder& depends_on(const render_resource_handle<pass>& p);
        pass_builder& on_pass_execute(move_only_function<void(command_buffer&)>&& commands);

    private:
        friend class render_graph;

        string _name;
        vector<render_target_usage> _colors;
        optional<render_target_usage> _depth;
        vector<render_resource_handle<pass>> _dependsOn;
        move_only_function<void(command_buffer&)> _commands;
    };

    class render_graph
    {
    public:
        render_graph(const unique_ptr<window>& win, const u32 framesInFlight = 2);
        render_graph(const render_graph&) = delete;
        render_graph(render_graph&& other) noexcept;
        ~render_graph();

        render_graph& operator=(const render_graph& rhs) = delete;
        render_graph& operator=(render_graph&& rhs) noexcept;

        render_resource_handle<pass> add_pass(const pass_builder& bldr);
        render_resource_handle<render_target> add_render_target(const render_target_builder& bldr);
        render_resource_handle<render_target> get_back_buffer() const noexcept;

        bool execute();

    private:
        struct impl;

        impl* _impl = nullptr;

        bool load_instance();
        bool query_physical_device();
        bool load_device();
        bool build_swapchain(const unique_ptr<window>& win);
        bool build_frame_in_flight_data();

        template <typename T>
        T* get(slot_map_key k) const noexcept;

        shader* get_shader(slot_map_key k) const noexcept;
        pass* get_pass(slot_map_key k) const noexcept;
        render_target* get_render_target(slot_map_key k) const noexcept;

        void build_render_target(render_target& tgt);

        void release_resources();
    };
    
    template<typename T>
    inline T* render_resource_handle<T>::get() const noexcept
    {
        return _graph->get<T>(_key);
    }

    template<typename T>
    inline T* render_resource_handle<T>::operator->() const noexcept
    {
        return get();
    }

    template<typename T>
    inline T& render_resource_handle<T>::operator*() noexcept
    {
        return *get();
    }

    template<typename T>
    inline const T& render_resource_handle<T>::operator*() const noexcept
    {
        return *get();
    }

    template<typename T>
    inline render_resource_handle<T>::operator bool() const noexcept
    {
        return _graph != nullptr && _key != invalid_slot_map_key;
    }
    
    template<typename T>
    inline T* render_graph::get(slot_map_key k) const noexcept
    {
        if constexpr (is_same_v<T, shader>)
        {
            return get_shader(k);
        }
        else if constexpr (is_same_v<T, pass>)
        {
            return get_pass(k);
        }
        else if constexpr (is_same_v<T, render_target>)
        {
            return get_pass(k);
        }
        else
        {
            return nullptr;
        }
    }
}

#endif // render_graph_hpp__
