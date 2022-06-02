#ifndef mouse_hpp__
#define mouse_hpp__

#include "../core/export.hpp"
#include "../core/primitives.hpp"
#include "../math/vec2.hpp"

namespace ryujin
{
    class input;

    class mouse
    {
    public:
        enum class button : u32
        {
            LEFT = 0,
            RIGHT = 1,
            MIDDLE = 2,
            MOUSE_4 = 3,
            MOUSE_5 = 4,
            MOUSE_6 = 5,
            MOUSE_7 = 6,
            MOUSE_8 = 7
        };

        enum class state : u32
        {
            RELEASED = 0,
            PRESSED = 1
        };

        mouse();

		RYUJIN_API [[nodiscard]] vec2<f64> cursor_position() const noexcept;
		RYUJIN_API [[nodiscard]] vec2<f64> cursor_position_delta() const noexcept;
        RYUJIN_API [[nodiscard]] vec2<f64> scroll_offset() const noexcept;
        RYUJIN_API [[nodiscard]] vec2<f64> scroll_offset_delta() const noexcept;
        RYUJIN_API [[nodiscard]] state get_state(const button btn) const noexcept;
        
    private:
        friend class input;

        void pre_poll();
        void set_cursor_position(const f64 x, const f64 y);
        void set_scroll_offset(const f64 x, const f64 y);
        void set_state(const button btn, const state s);

        // cursor
        f64 _x = 0, _y = 0;
        f64 _prevX = 0, _prevY = 0;

        // scroll
        f64 _xScrollOff = 0, _yScrollOff = 0;
        f64 _prevXScrollOff = 0, _prevYScrollOff = 0;

        // buttons
        state _states[static_cast<int>(button::MOUSE_8) + 1] = { state::RELEASED };
    };
}

#endif // mouse_hpp__
