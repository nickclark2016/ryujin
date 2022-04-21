#ifndef window_hpp__
#define window_hpp__

#include "../core/primitives.hpp"
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
        void focusCallback(GLFWwindow*, i32);
        void closeCallback(GLFWwindow*);
        void resizeCallback(GLFWwindow*, i32, i32);
        void iconifyCallback(GLFWwindow*, i32);
        void maximizeCallback(GLFWwindow*, i32);
        void keyboardCallback(GLFWwindow*, i32, i32, i32, i32);
        void cursorCallback(GLFWwindow*, f64, f64);
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
            u32 width;
            u32 height;
        };

        static result<std::unique_ptr<window>, error_code> create(const create_info& info) noexcept;

        ~window();

        result<std::tuple<u32, u32>, error_code> size() const noexcept;
        void size(const u32 width, const u32 height) noexcept;
        bool focused() const noexcept;
        void focus() noexcept;
        bool should_close() const noexcept;
        void should_close(bool close) noexcept;
        void show() noexcept;

        void on_focus(const std::function<void(bool)>& fn);
        void on_close(const std::function<void()>& fn);
        void on_resize(const std::function<void(u32, u32)>& fn);
        void on_iconify(const std::function<void()>& fn);
        void on_restore(const std::function<void()>& fn);
        void on_maximize(const std::function<void()>& fn);

        void on_keystroke(const std::function<void(i32, i32, i32, i32)>& fn);

        void on_cursor_move(const std::function<void(f64, f64)>& fn);

        void capture_cursor();
        void release_cursor();

        void after_close(const std::function<void()>& fn);

    private:
        GLFWwindow* _native;
        bool _focused = true;

        GLFWwindowfocusfun _focusCb;
        vector<std::function<void(bool)>> _userFocusCallbacks;

        GLFWwindowclosefun _closeCb;
        vector<std::function<void()>> _userCloseCallbacks;

        GLFWwindowsizefun _resizeCb;
        vector<std::function<void(u32, u32)>> _userResizeCallbacks;

        GLFWwindowiconifyfun _iconifyCb;
        GLFWwindowmaximizefun _maximizeCb;

        vector<std::function<void()>> _userIconifyCallbacks;
        vector<std::function<void()>> _userRestoreCallbacks;
        vector<std::function<void()>> _userMaximizeCallbacks;

        GLFWkeyfun _keyboardCb;
        vector<std::function<void(int, int, int, int)>> _userKeyCallbacks;

        GLFWcursorposfun _cursorPosCb;
        vector<std::function<void(f64, f64)>> _userCursorPosCallbacks;

        std::function<void()> _afterCloseCb = []() {};

        friend void detail::focusCallback(GLFWwindow*, int);
        friend void detail::closeCallback(GLFWwindow*);
        friend void detail::resizeCallback(GLFWwindow*, int, int);
        friend void detail::iconifyCallback(GLFWwindow*, int);
        friend void detail::maximizeCallback(GLFWwindow*, int);
        friend void detail::keyboardCallback(GLFWwindow*, i32, i32, i32, i32);
        friend void detail::cursorCallback(GLFWwindow*, f64, f64);
        
        friend class render_manager;
    };
}

#endif // window_hpp__
