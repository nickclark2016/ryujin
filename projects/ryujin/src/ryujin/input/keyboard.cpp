#include <ryujin/input/keyboard.hpp>

#include <ryujin/core/as.hpp>

namespace ryujin
{
    keyboard::keyboard()
    {
        memset(_states, as<i32>(state::RELEASED), sizeof(_states));
    }

    keyboard::state keyboard::get_state(const key k) const noexcept
    {
        return _states[as<sz>(k)];
    }

    void keyboard::set_state(key k, state s)
    {
        _states[as<sz>(k)] = s;
    }

}