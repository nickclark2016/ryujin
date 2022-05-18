#include <ryujin/graphics/window.hpp>

#include <GLFW/glfw3.h>

namespace ryujin
{
	namespace detail
	{
		bool initGlfw() noexcept {
			static bool initGlfw = glfwInit();
			return initGlfw;
		}

		void focusCallback(GLFWwindow* win, i32 focused)
		{
			window* userWin = reinterpret_cast<window*>(glfwGetWindowUserPointer(win));
			userWin->_focused = focused == GLFW_TRUE;
			
			for (auto& cb : userWin->_userFocusCallbacks)
			{
				cb(focused == GLFW_TRUE);
			}
		}
		
		void closeCallback(GLFWwindow* win)
		{
			window* userWin = reinterpret_cast<window*>(glfwGetWindowUserPointer(win));

			for (auto& cb : userWin->_userCloseCallbacks)
			{
				cb();
			}

			userWin->_afterCloseCb();
		}
		
		void resizeCallback(GLFWwindow* win, i32 width, i32 height)
		{
			window* userWin = reinterpret_cast<window*>(glfwGetWindowUserPointer(win));

			for (auto& cb : userWin->_userResizeCallbacks)
			{
				cb(static_cast<u32>(width), static_cast<u32>(height));
			}
		}

		void iconifyCallback(GLFWwindow* win, i32 minimized)
		{
			window* userWin = reinterpret_cast<window*>(glfwGetWindowUserPointer(win));
			const bool isIconified = minimized == GLFW_TRUE;

			if (isIconified)
			{
				for (auto& cb : userWin->_userIconifyCallbacks)
				{
					cb();
				}
			}
			else
			{
				for (auto& cb : userWin->_userRestoreCallbacks)
				{
					cb();
				}
			}
		}

		void maximizeCallback(GLFWwindow* win, i32 maximized)
		{
			window* userWin = reinterpret_cast<window*>(glfwGetWindowUserPointer(win));
			const bool isMaximized = maximized == GLFW_TRUE;

			if (isMaximized)
			{
				for (auto& cb : userWin->_userMaximizeCallbacks)
				{
					cb();
				}
			}
			else
			{
				for (auto& cb : userWin->_userRestoreCallbacks)
				{
					cb();
				}
			}
		}
	
		void keyboardCallback(GLFWwindow* win, i32 key, i32 scan, i32 action, i32 mods)
		{
			window* userWin = reinterpret_cast<window*>(glfwGetWindowUserPointer(win));
			for (auto& cb : userWin->_userKeyCallbacks)
			{
				cb(key, scan, action, mods);
			}
		}

		void cursorCallback(GLFWwindow* win, f64 x, f64 y)
		{
			window* userWin = reinterpret_cast<window*>(glfwGetWindowUserPointer(win));
			for (auto& cb : userWin->_userCursorPosCallbacks)
			{
				cb(x, y);
			}
		}

		void scrollCallback(GLFWwindow* win, f64 x, f64 y)
		{
			window* userWin = reinterpret_cast<window*>(glfwGetWindowUserPointer(win));
			for (auto& cb : userWin->_userScrollCallbacks)
			{
				cb(x, y);
			}
		}

		void mouseButtonCallback(GLFWwindow* win, i32 button, i32 action, i32 mods)
		{
			window* userWin = reinterpret_cast<window*>(glfwGetWindowUserPointer(win));
			for (auto& cb : userWin->_userMouseBtnCallbacks)
			{
				cb(button, action, mods);
			}
		}
	}

	result<unique_ptr<window>, window::error_code> window::create(const window::create_info& info) noexcept
	{
		using result_type = result<unique_ptr<window>, error_code>;

		const bool isInit = detail::initGlfw();
		if (!isInit)
		{
			return result_type::from_error(error_code::LOAD_FAILURE);
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		GLFWwindow* nativeWindowHandle = glfwCreateWindow(static_cast<int>(info.width), static_cast<int>(info.height), info.name.c_str(), nullptr, nullptr);
		if (nativeWindowHandle == nullptr)
		{
			return result_type::from_error(error_code::WINDOW_CREATE_FAILURE);
		}

		unique_ptr<window> win(new window);
		win->_native = nativeWindowHandle;
		
		glfwSetWindowUserPointer(win->_native, win.get());
		glfwSetWindowFocusCallback(win->_native, detail::focusCallback);
		glfwSetWindowCloseCallback(win->_native, detail::closeCallback);
		glfwSetWindowSizeCallback(win->_native, detail::resizeCallback);
		glfwSetWindowIconifyCallback(win->_native, detail::iconifyCallback);
		glfwSetWindowMaximizeCallback(win->_native, detail::maximizeCallback);
		
		// Input functions
		glfwSetKeyCallback(win->_native, detail::keyboardCallback);
		glfwSetCursorPosCallback(win->_native, detail::cursorCallback);
		glfwSetScrollCallback(win->_native, detail::scrollCallback);
		glfwSetMouseButtonCallback(win->_native, detail::mouseButtonCallback);

		return result_type::from_success(ryujin::move(win));
	}

	window::~window()
	{
		_userCloseCallbacks.clear();
		_userCursorPosCallbacks.clear();
		_userFocusCallbacks.clear();
		_userIconifyCallbacks.clear();
		_userKeyCallbacks.clear();
		_userMaximizeCallbacks.clear();
		_userResizeCallbacks.clear();
		_userRestoreCallbacks.clear();
		_userScrollCallbacks.clear();

		glfwSetWindowFocusCallback(_native, nullptr);
		glfwSetWindowCloseCallback(_native, nullptr);
		glfwSetWindowSizeCallback(_native, nullptr);
	}
	
	result<tuple<u32, u32>, window::error_code> window::size() const noexcept
	{
		using result_type = result<tuple<u32, u32>, error_code>;

		i32 w, h;
		glfwGetWindowSize(_native, &w, &h);

		auto res = ryujin::make_tuple(static_cast<u32>(w), static_cast<u32>(h));
		return result_type::from_success(std::move(res));
	}
	
	void window::size(const u32 width, const u32 height) noexcept
	{
		glfwSetWindowSize(_native, static_cast<int>(width), static_cast<int>(height));
	}

	bool window::focused() const noexcept
	{
		return _focused;
	}
	
	void window::focus() noexcept
	{
		glfwFocusWindow(_native);
	}
	
	bool window::should_close() const noexcept
	{
		return glfwWindowShouldClose(_native) == GLFW_TRUE;
	}

	void window::should_close(bool close) noexcept
	{
		glfwSetWindowShouldClose(_native, close ? GLFW_TRUE : GLFW_FALSE);
	}
	
	void window::show() noexcept
	{
		glfwShowWindow(_native);
	}
	
	void window::on_focus(move_only_function<void(bool)>&& fn)
	{
		_userFocusCallbacks.push_back(ryujin::move(fn));
	}
	
	void window::on_close(move_only_function<void()>&& fn)
	{
		_userCloseCallbacks.push_back(ryujin::move(fn));
	}

	void window::on_resize(move_only_function<void(u32, u32)>&& fn)
	{
		_userResizeCallbacks.push_back(ryujin::move(fn));
	}

	void window::on_iconify(move_only_function<void()>&& fn)
	{
		_userIconifyCallbacks.push_back(ryujin::move(fn));
	}

	void window::on_restore(move_only_function<void()>&& fn)
	{
		_userRestoreCallbacks.push_back(ryujin::move(fn));
	}

	void window::on_maximize(move_only_function<void()>&& fn)
	{
		_userMaximizeCallbacks.push_back(ryujin::move(fn));
	}
	
	void window::on_keystroke(move_only_function<void(int, int, int, int)>&& fn)
	{
		_userKeyCallbacks.push_back(ryujin::move(fn));
	}
	
	void window::on_cursor_move(move_only_function<void(f64, f64)>&& fn)
	{
		_userCursorPosCallbacks.push_back(ryujin::move(fn));
	}

	void window::on_scroll(move_only_function<void(f64, f64)>&& fn)
	{
		_userScrollCallbacks.push_back(ryujin::move(fn));
	}

	void window::on_buttonstroke(move_only_function<void(i32, i32, i32)>&& fn)
	{
		_userMouseBtnCallbacks.push_back(ryujin::move(fn));
	}

	void window::after_close(move_only_function<void()>&& fn)
	{
		_afterCloseCb = ryujin::move(fn);
	}

	bool window::is_cursor_captured() const
	{
		return glfwGetInputMode(_native, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;
	}

	void window::capture_cursor()
	{
		glfwSetInputMode(_native, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	
	void window::release_cursor()
	{
		glfwSetInputMode(_native, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}