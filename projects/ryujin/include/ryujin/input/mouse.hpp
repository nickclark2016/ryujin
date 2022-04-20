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
		[[nodiscard]] vec2<f64> cursor_position() const noexcept;
		[[nodiscard]] vec2<f64> cursor_position_delta() const noexcept;
        
    private:
        friend class input;

        void pre_poll();
        void set_cursor_position(const f64 x, const f64 y);

        f64 _x = 0, _y = 0;
        f64 _prevX = 0, _prevY = 0;
    };
}

#endif // mouse_hpp__
