#include <ryujin/input/input.hpp>

#include <ryujin/core/as.hpp>

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
    std::optional<std::reference_wrapper<const std::unique_ptr<window>>> input::_active;

    input::input(const input& i)
        : _win(i._win)
    {
    }

    input& input::operator=(const input& i)
    {
        _win = i._win;
        return *this;
    }

    const keyboard& input::keys() const
    {
        return _keys;
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

        win->on_focus([w](bool focused) {
                if (focused)
                {
                    _active = w;
                }
                else if (w.get() == _active->get())
                {
                    _active = std::nullopt;
                }
            });

        win->on_keystroke([w](int key, int scan, int action, int mods) {
                const auto it = _inputs.find(w);
                if (it == _inputs.end()) return;
                auto& in = it->second;
                const auto k = as<keyboard::key>(key);
                const auto state = as<keyboard::state>(action);
                in._keys.set_state(k, state);
            });

        win->on_close([w]() {
                _inputs.erase(w);
            });

        if (!_active.has_value() && w.get()->focused())
        {
            _active = w;
        }

        return in;
    }

    input* input::get_input()
    {
        if (_active)
        {
            const auto it = _inputs.find(*_active);
            if (it != _inputs.end())
            {
                return &it->second;
            }
        }
        return nullptr;
    }
    
    input::input(const std::unique_ptr<window>& win)
        : _win(std::cref(win))
    {
    }
}