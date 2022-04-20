#ifndef mouse_hpp__
#define mouse_hpp__

#include "../core/primitives.hpp"
#include "../math/vec2.hpp"

namespace ryujin
{
    class input;

    class mouse
    {
    public:
        vec2<f64> cursor_position() const noexcept;
        vec2<f64> cursor_position_delta() const noexcept;
        
    private:
        friend class input;

        void pre_poll();
        void set_cursor_position(const f64 x, const f64 y);

        f64 _x, _y;
        f64 _prevX, _prevY;
    };
}

#endif // mouse_hpp__
