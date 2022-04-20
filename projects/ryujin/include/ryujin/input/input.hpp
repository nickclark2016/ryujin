#ifndef input_hpp__
#define input_hpp__

#include "keyboard.hpp"
#include "mouse.hpp"

#include "../graphics/window.hpp"

#include <memory>
#include <optional>
#include <unordered_map>

namespace ryujin
{
    namespace detail
    {
        struct input_hasher
        {
            size_t operator()(const std::reference_wrapper<const std::unique_ptr<window>>& win) const noexcept;
            bool operator()(const std::reference_wrapper<const std::unique_ptr<window>>& lhs, const std::reference_wrapper<const std::unique_ptr<window>>& rhs) const;
        };
    }

    class input
    {
    public:
        input(const input& i);
        input& operator=(const input& i);

        const keyboard& get_keys() const;
        const mouse& get_mouse() const;

        static void poll();
        static input register_window(const std::unique_ptr<window>& win);

        static input* get_input();

    private:
        static std::unordered_map<std::reference_wrapper<const std::unique_ptr<window>>, input, detail::input_hasher, detail::input_hasher> _inputs;
        static std::optional<std::reference_wrapper<const std::unique_ptr<window>>> _active;

        explicit input(const std::unique_ptr<window>& win);

        std::reference_wrapper<const std::unique_ptr<window>> _win;
        keyboard _keys;
        ryujin::mouse _mouse;
    };
}

#endif // input_hpp__