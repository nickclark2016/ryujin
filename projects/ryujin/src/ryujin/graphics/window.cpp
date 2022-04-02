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

		void focusCallback(GLFWwindow* win, int focused)
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
		}
		
		void resizeCallback(GLFWwindow* win, int width, int height)
		{
			window* userWin = reinterpret_cast<window*>(glfwGetWindowUserPointer(win));

			for (auto& cb : userWin->_userResizeCallbacks)
			{
				cb(static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height));
			}
		}
	}

	result<std::unique_ptr<window>, window::error_code> window::create(const window::create_info& info) noexcept
	{
		using result_type = result<std::unique_ptr<window>, error_code>;

		const bool isInit = detail::initGlfw();
		if (!isInit)
		{
			return result_type::from_error(error_code::LOAD_FAILURE);
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		GLFWwindow* nativeWindowHandle = glfwCreateWindow(static_cast<int>(info.width), static_cast<int>(info.height), info.name.c_str(), nullptr, nullptr);
		if (nativeWindowHandle == nullptr)
		{
			return result_type::from_error(error_code::WINDOW_CREATE_FAILURE);
		}

		std::unique_ptr<window> win(new window);
		win->_native = nativeWindowHandle;
		
		glfwSetWindowUserPointer(win->_native, win.get());
		glfwSetWindowFocusCallback(win->_native, detail::focusCallback);
		glfwSetWindowCloseCallback(win->_native, detail::closeCallback);
		glfwSetWindowSizeCallback(win->_native, detail::resizeCallback);

		return result_type::from_success(std::move(win));
	}

	window::~window()
	{
		glfwSetWindowFocusCallback(_native, nullptr);
		glfwSetWindowCloseCallback(_native, nullptr);
		glfwSetWindowSizeCallback(_native, nullptr);
	}
	
	result<std::tuple<std::uint32_t, std::uint32_t>, window::error_code> window::size() const noexcept
	{
		using result_type = result<std::tuple<std::uint32_t, std::uint32_t>, error_code>;

		int w, h;
		glfwGetWindowSize(_native, &w, &h);

		auto res = std::make_tuple(static_cast<std::uint32_t>(w), static_cast<std::uint32_t>(h));
		return result_type::from_success(std::move(res));
	}
	
	void window::size(const std::uint32_t width, const std::uint32_t height) noexcept
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
	
	void window::on_focus(const std::function<void(bool)>& fn)
	{
		_userFocusCallbacks.push_back(fn);
	}
	
	void window::on_close(const std::function<void()>& fn)
	{
		_userCloseCallbacks.push_back(fn);
	}

	void window::on_resize(const std::function<void(std::uint32_t, std::uint32_t)>& fn)
	{
		_userResizeCallbacks.push_back(fn);
	}
}