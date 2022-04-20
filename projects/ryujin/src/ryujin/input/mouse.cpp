#include <ryujin/input/mouse.hpp>

namespace ryujin
{
    vec2<f64> mouse::cursor_position() const noexcept
    {
        return { _x, _y };
    }

    vec2<f64> mouse::cursor_position_delta() const noexcept
    {
        return { _x - _prevX, _y - _prevY };
    }

    void mouse::pre_poll()
    {
        _prevX = _x;
        _prevY = _y;
    }

    void mouse::set_cursor_position(const f64 x, const f64 y)
    {
        _prevX = _x;
        _prevY = _y;
        _x = x;
        _y = y;
    }

}