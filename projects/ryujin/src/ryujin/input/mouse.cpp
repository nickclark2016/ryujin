#include <ryujin/input/mouse.hpp>

#include <ryujin/core/as.hpp>
#include <ryujin/core/primitives.hpp>

#include <cstring>

namespace ryujin
{
    mouse::mouse()
    {
        memset(_states, as<i32>(state::RELEASED), sizeof(_states));
    }

    vec2<f64> mouse::cursor_position() const noexcept
    {
        return { _x, _y };
    }

    vec2<f64> mouse::cursor_position_delta() const noexcept
    {
        return { _x - _prevX, _y - _prevY };
    }

    vec2<f64> mouse::scroll_offset() const noexcept
    {
        return { _xScrollOff, _yScrollOff };
    }

    vec2<f64> mouse::scroll_offset_delta() const noexcept
    {
        return { _xScrollOff - _prevXScrollOff, _yScrollOff - _prevYScrollOff };
    }

    mouse::state mouse::get_state(const button btn) const noexcept
    {
        return _states[as<sz>(btn)];
    }

    void mouse::pre_poll()
    {
        _prevX = _x;
        _prevY = _y;
        _prevXScrollOff = _xScrollOff;
        _prevYScrollOff = _yScrollOff;
    }

    void mouse::set_cursor_position(const f64 x, const f64 y)
    {
        _x = x;
        _y = y;
    }

    void mouse::set_scroll_offset(const f64 x, const f64 y)
    {
        _xScrollOff = x;
        _yScrollOff = y;
    }

    void mouse::set_state(const mouse::button btn, const mouse::state s)
    {
        _states[as<sz>(btn)] = s;
    }

}