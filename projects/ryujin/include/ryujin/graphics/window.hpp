#ifndef window_hpp__
#define window_hpp__

#include "../core/export.hpp"
#include "../core/functional.hpp"
#include "../core/primitives.hpp"
#include "../core/result.hpp"
#include "../core/smart_pointers.hpp"
#include "../core/string.hpp"
#include "../core/utility.hpp"
#include "../core/vector.hpp"

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
            string name;
            u32 width;
            u32 height;
        };

        RYUJIN_API static result<unique_ptr<window>, error_code> create(const create_info& info) noexcept;

        RYUJIN_API ~window();

        RYUJIN_API result<tuple<u32, u32>, error_code> size() const noexcept;
        RYUJIN_API void size(const u32 width, const u32 height) noexcept;
        RYUJIN_API bool focused() const noexcept;
        RYUJIN_API void focus() noexcept;
        RYUJIN_API bool should_close() const noexcept;
        RYUJIN_API void should_close(bool close) noexcept;
        RYUJIN_API void show() noexcept;

        RYUJIN_API void on_focus(move_only_function<void(bool)>&& fn);
        RYUJIN_API void on_close(move_only_function<void()>&& fn);
        RYUJIN_API void on_resize(move_only_function<void(u32, u32)>&& fn);
        RYUJIN_API void on_iconify(move_only_function<void()>&& fn);
        RYUJIN_API void on_restore(move_only_function<void()>&& fn);
        RYUJIN_API void on_maximize(move_only_function<void()>&& fn);

        RYUJIN_API void on_keystroke(move_only_function<void(i32, i32, i32, i32)>&& fn);
        RYUJIN_API void on_cursor_move(move_only_function<void(f64, f64)>&& fn);
        RYUJIN_API void on_scroll(move_only_function<void(f64, f64)>&& fn);
        RYUJIN_API void on_buttonstroke(move_only_function<void(i32, i32, i32)>&& fn);

        RYUJIN_API void capture_cursor();
        RYUJIN_API void release_cursor();

        RYUJIN_API void after_close(move_only_function<void()>&& fn);

        RYUJIN_API bool is_cursor_captured() const;

    private:
        GLFWwindow* _native = nullptr;
        bool _focused = true;

        vector<move_only_function<void(bool)>> _userFocusCallbacks;
        vector<move_only_function<void()>> _userCloseCallbacks;
        vector<move_only_function<void(u32, u32)>> _userResizeCallbacks;
        vector<move_only_function<void()>> _userIconifyCallbacks;
        vector<move_only_function<void()>> _userRestoreCallbacks;
        vector<move_only_function<void()>> _userMaximizeCallbacks;
        vector<move_only_function<void(int, int, int, int)>> _userKeyCallbacks;
        vector<move_only_function<void(f64, f64)>> _userCursorPosCallbacks;
        vector<move_only_function<void(f64, f64)>> _userScrollCallbacks;
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
