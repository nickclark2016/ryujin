#ifndef input_hpp__
#define input_hpp__

#include "keyboard.hpp"
#include "mouse.hpp"

#include "../core/functional.hpp"
#include "../core/optional.hpp"
#include "../core/smart_pointers.hpp"
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
            size_t operator()(const reference_wrapper<const unique_ptr<window>>& win) const noexcept;
            bool operator()(const reference_wrapper<const unique_ptr<window>>& lhs, const reference_wrapper<const unique_ptr<window>>& rhs) const;
        };
    }

    class input
    {
    public:
        input(const input& i);
        input& operator=(const input& i);

        [[nodiscard]] const keyboard& get_keys() const;
        [[nodiscard]] const mouse& get_mouse() const;

        static void poll();
        static input register_window(const unique_ptr<window>& win);

        static input* get_input();

    private:
        static std::unordered_map<reference_wrapper<const unique_ptr<window>>, input, detail::input_hasher, detail::input_hasher> _inputs;
        static optional<reference_wrapper<const unique_ptr<window>>> _active;

        explicit input(const unique_ptr<window>& win);

        reference_wrapper<const unique_ptr<window>> _win;
        keyboard _keys = {};
        mouse _mouse = {};
    };
}

#endif // input_hpp__