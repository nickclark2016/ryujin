#include <ryujin/input/input.hpp>

#include <ryujin/core/as.hpp>

#include <GLFW/glfw3.h>

namespace ryujin
{
    namespace detail
    {
        size_t input_hasher::operator()(const reference_wrapper<const unique_ptr<window>>& win) const noexcept
        {
            return std::hash<window*>()(win.get().get());
        }

        bool input_hasher::operator()(const reference_wrapper<const unique_ptr<window>>& lhs, const reference_wrapper<const unique_ptr<window>>& rhs) const
        {
            return lhs.get().get() == rhs.get().get();
        }
    }

    std::unordered_map<reference_wrapper<const unique_ptr<window>>, input, detail::input_hasher, detail::input_hasher> input::_inputs = {};
    optional<reference_wrapper<const unique_ptr<window>>> input::_active = nullopt;

    input::input(const input& i)
        : _win(i._win)
    {
    }

    input& input::operator=(const input& i)
    {
        _win = i._win;
        return *this;
    }

    const keyboard& input::get_keys() const
    {
        return _keys;
    }

    const mouse& input::get_mouse() const
    {
        return _mouse;
    }

    void input::poll()
    {
        for (auto& [_, input] : _inputs)
        {
            input._mouse.pre_poll();
        }
        glfwPollEvents();
    }

    input input::register_window(const unique_ptr<window>& win)
    {
        reference_wrapper<const unique_ptr<window>> w = ryujin::cref(win);
        input in(win);

        if (!_inputs.contains(w))
        {
            _inputs.insert({ w, in });
        }

        win->on_focus([w](const bool focused) {
                if (focused)
                {
                    _active = w;
                }
                else if (w.get() == _active->get())
                {
                    _active = nullopt;
                }
            });

        win->on_keystroke([w](const int key, int, const int action, int) {
                const auto it = _inputs.find(w);
                if (it == _inputs.end()) return;
                auto& i = it->second;
                const auto k = as<keyboard::key>(key);
                const auto state = as<keyboard::state>(action);
                i._keys.set_state(k, state);
            });

        win->on_cursor_move([w](const f64 x, const f64 y) {
                const auto it = _inputs.find(w);
                if (it == _inputs.end()) return;
                auto& i = it->second;
                i._mouse.set_cursor_position(x, y);
            });

        win->on_scroll([w](const f64 x, const f64 y) {
                const auto it = _inputs.find(w);
                if (it == _inputs.end()) return;
                auto& i = it->second;
                i._mouse.set_scroll_offset(x, y);
            });

        win->on_buttonstroke([w](const i32 btn, const i32 action, const i32 mods) {
                const auto it = _inputs.find(w);
                if (it == _inputs.end()) return;
                auto& i = it->second;
                const auto k = as<mouse::button>(btn);
                const auto state = as<mouse::state>(action);
                i._mouse.set_state(k, state);
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
    
    input::input(const unique_ptr<window>& win)
        : _win(std::cref(win))
    {
    }
}