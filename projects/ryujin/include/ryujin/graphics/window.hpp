#ifndef window_hpp__
#define window_hpp__

#include "../core/functional.hpp"
#include "../core/primitives.hpp"
#include "../core/result.hpp"
#include "../core/smart_pointers.hpp"
#include "../core/vector.hpp"

#include <string>
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
        void scrollCallback(GLFWwindow*, f64, f64);
        void mouseButtonCallback(GLFWwindow*, i32, i32, i32);
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

        static result<unique_ptr<window>, error_code> create(const create_info& info) noexcept;

        ~window();

        result<std::tuple<u32, u32>, error_code> size() const noexcept;
        void size(const u32 width, const u32 height) noexcept;
        bool focused() const noexcept;
        void focus() noexcept;
        bool should_close() const noexcept;
        void should_close(bool close) noexcept;
        void show() noexcept;

        void on_focus(move_only_function<void(bool)>&& fn);
        void on_close(move_only_function<void()>&& fn);
        void on_resize(move_only_function<void(u32, u32)>&& fn);
        void on_iconify(move_only_function<void()>&& fn);
        void on_restore(move_only_function<void()>&& fn);
        void on_maximize(move_only_function<void()>&& fn);

        void on_keystroke(move_only_function<void(i32, i32, i32, i32)>&& fn);
        void on_cursor_move(move_only_function<void(f64, f64)>&& fn);
        void on_scroll(move_only_function<void(f64, f64)>&& fn);
        void on_buttonstroke(move_only_function<void(i32, i32, i32)>&& fn);

        void capture_cursor();
        void release_cursor();

        void after_close(move_only_function<void()>&& fn);

        bool is_cursor_captured() const;

    private:
        GLFWwindow* _native;
        bool _focused = true;

        GLFWwindowfocusfun _focusCb;
        vector<move_only_function<void(bool)>> _userFocusCallbacks;

        GLFWwindowclosefun _closeCb;
        vector<move_only_function<void()>> _userCloseCallbacks;

        GLFWwindowsizefun _resizeCb;
        vector<move_only_function<void(u32, u32)>> _userResizeCallbacks;

        GLFWwindowiconifyfun _iconifyCb;
        GLFWwindowmaximizefun _maximizeCb;

        vector<move_only_function<void()>> _userIconifyCallbacks;
        vector<move_only_function<void()>> _userRestoreCallbacks;
        vector<move_only_function<void()>> _userMaximizeCallbacks;

        GLFWkeyfun _keyboardCb;
        vector<move_only_function<void(int, int, int, int)>> _userKeyCallbacks;

        GLFWcursorposfun _cursorPosCb;
        vector<move_only_function<void(f64, f64)>> _userCursorPosCallbacks;

        GLFWscrollfun _scrollCb;
        vector<move_only_function<void(f64, f64)>> _userScrollCallbacks;

        GLFWmousebuttonfun _mouseBtnCb;
        vector<move_only_function<void(i32, i32, i32)>> _userMouseBtnCallbacks;

        move_only_function<void()> _afterCloseCb = []() {};

        friend void detail::focusCallback(GLFWwindow*, int);
        friend void detail::closeCallback(GLFWwindow*);
        friend void detail::resizeCallback(GLFWwindow*, int, int);
        friend void detail::iconifyCallback(GLFWwindow*, int);
        friend void detail::maximizeCallback(GLFWwindow*, int);
        friend void detail::keyboardCallback(GLFWwindow*, i32, i32, i32, i32);
        friend void detail::cursorCallback(GLFWwindow*, f64, f64);
        friend void detail::scrollCallback(GLFWwindow*, f64, f64);
        friend void detail::mouseButtonCallback(GLFWwindow*, i32, i32, i32);
        
        friend class render_manager;
    };
}

#endif // window_hpp__
