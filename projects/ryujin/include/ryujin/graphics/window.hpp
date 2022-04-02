#ifndef window_hpp__
#define window_hpp__

#include "../core/result.hpp"
#include "../core/vector.hpp"

#include <functional>
#include <memory>
#include <string_view>
#include <tuple>

#include <GLFW/glfw3.h>

namespace ryujin
{
    namespace detail
    {
        void focusCallback(GLFWwindow*, int);
        void closeCallback(GLFWwindow*);
        void resizeCallback(GLFWwindow*, int, int);
    } // namespace detail

    class render_manager;

    class window
    {
    public:
        enum class error_code
        {
            LOAD_FAILURE,
            WINDOW_CREATE_FAILURE,
            MEMORY_ALLOCATION_FAILURE,
            API_FAILURE
        };

        struct create_info
        {
            std::string name;
            std::uint32_t width;
            std::uint32_t height;
        };

        static result<std::unique_ptr<window>, error_code> create(const create_info& info) noexcept;

        ~window();

        result<std::tuple<std::uint32_t, std::uint32_t>, error_code> size() const noexcept;
        void size(const std::uint32_t width, const std::uint32_t height) noexcept;
        bool focused() const noexcept;
        void focus() noexcept;
        bool should_close() const noexcept;
        void should_close(bool close) noexcept;
        void show() noexcept;

        void on_focus(const std::function<void(bool)>& fn);
        void on_close(const std::function<void()>& fn);
        void on_resize(const std::function<void(std::uint32_t, std::uint32_t)>& fn);

    private:
        GLFWwindow* _native;
        bool _focused;

        GLFWwindowfocusfun _focusCb;
        vector<std::function<void(bool)>> _userFocusCallbacks;

        GLFWwindowclosefun _closeCb;
        vector<std::function<void()>> _userCloseCallbacks;

        GLFWwindowsizefun _resizeCb;
        vector<std::function<void(std::uint32_t, std::uint32_t)>> _userResizeCallbacks;

        friend void detail::focusCallback(GLFWwindow*, int);
        friend void detail::closeCallback(GLFWwindow*);
        friend void detail::resizeCallback(GLFWwindow*, int, int);
        
        friend class render_manager;
    };
}

#endif // window_hpp__
