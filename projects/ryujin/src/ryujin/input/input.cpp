#include <ryujin/input/input.hpp>

#include <GLFW/glfw3.h>

namespace ryujin
{
    namespace detail
    {
        size_t input_hasher::operator()(const std::reference_wrapper<const std::unique_ptr<window>>& win) const noexcept
        {
            return std::hash<window*>()(win.get().get());
        }

        bool input_hasher::operator()(const std::reference_wrapper<const std::unique_ptr<window>>& lhs, const std::reference_wrapper<const std::unique_ptr<window>>& rhs) const
        {
            return lhs.get().get() == rhs.get().get();
        }
    }

    std::unordered_map<std::reference_wrapper<const std::unique_ptr<window>>, input, detail::input_hasher, detail::input_hasher> input::_inputs;

    input::input(const input& i)
        : _win(i._win)
    {
    }

    input& input::operator=(const input& i)
    {
        _win = i._win;
        return *this;
    }

    void input::poll()
    {
        glfwPollEvents();
    }

    input input::register_window(const std::unique_ptr<window>& win)
    {
        std::reference_wrapper<const std::unique_ptr<window>> w = std::cref(win);
        input in(win);

        if (_inputs.find(w) == _inputs.end())
        {
            _inputs.insert({ w, in });
        }

        win->on_close([w]() {
                _inputs.erase(w);
            });

        return in;
    }
    
    input::input(const std::unique_ptr<window>& win)
        : _win(std::cref(win))
    {
    }
}