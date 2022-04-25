#ifndef optional_hpp__
#define optional_hpp__

#include "utility.hpp"
#include "variant.hpp"

#include <type_traits>

namespace ryujin
{
    struct nullopt_t
    {
        inline constexpr explicit nullopt_t(int) noexcept {};
    };

    inline constexpr nullopt_t nullopt = nullopt_t{ 0 };

    template <typename T>
    class optional
    {
    public:
        constexpr optional() noexcept;
        constexpr optional(nullopt_t) noexcept;
        constexpr optional(const optional& other);
        constexpr optional(optional&& other) noexcept(std::is_nothrow_move_constructible_v<T>);

        template <typename ... Args>
        constexpr explicit optional(in_place_t, Args&& ... args);

        template <typename U = T, std::enable_if_t<std::is_convertible_v<U&&, T>, int> = 0>
        constexpr optional(U&& value);

        constexpr ~optional() noexcept(std::is_nothrow_destructible_v<T>) = default;

        constexpr optional& operator=(nullopt_t) noexcept;
        constexpr optional& operator=(const optional& other);
        constexpr optional& operator=(optional&& other) noexcept(std::is_nothrow_move_assignable_v<T>);

        template <typename U = T, std::enable_if_t<std::is_convertible_v<U&&, T>, int> = 0>
        constexpr optional& operator=(U&& value);

        constexpr T* operator->() noexcept;
        constexpr const T* operator->() const noexcept;
        constexpr T& operator*()& noexcept;
        constexpr const T& operator*() const& noexcept;

        constexpr T&& operator*() && noexcept;
        constexpr const T&& operator*() const&& noexcept;

        constexpr operator bool() const noexcept;
        constexpr bool has_value() const noexcept;

        constexpr T& value() & noexcept;
        constexpr const T& value() const& noexcept;

        constexpr T&& value() && noexcept;
        constexpr const T&& value() const&& noexcept;

        template <typename U, std::enable_if_t<std::is_convertible_v<U&&, T>, int> = 0>
        constexpr T value_or(U&& default_value) const&;

        template <typename U, std::enable_if_t<std::is_convertible_v<U&&, T>, int> = 0>
        constexpr T value_or(U&& default_value)&&;

        constexpr void swap(optional& other) noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_swappable_v<T>);
        constexpr void reset();

        template <typename ... Args>
        constexpr T& emplace(Args&& ... args);

    private:
        variant<nullopt_t, T> _storage;
    };

    template <typename T>
    inline constexpr optional<T>::optional() noexcept
        : _storage(nullopt)
    {
    }

    template <typename T>
    inline constexpr optional<T>::optional(nullopt_t) noexcept
        : optional()
    {
    }

    template <typename T>
    inline constexpr optional<T>::optional(const optional& other)
        : _storage(other._storage)
    {
    }


    template <typename T>
    inline constexpr optional<T>::optional(optional&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
        : _storage(ryujin::move(other._storage))
    {
    }

    template <typename T>
    template <typename ... Args>
    inline constexpr optional<T>::optional(in_place_t, Args&& ... args)
        : _storage(T(ryujin::forward<Args>(args)...))
    {
    }

    template <typename T>
    template <typename U, std::enable_if_t<std::is_convertible_v<U&&, T>, int>>
    inline constexpr optional<T>::optional(U&& value)
        : _storage(T(ryujin::forward<U>(value)))
    {
    }

    template <typename T>
    inline constexpr optional<T>& optional<T>::operator=(nullopt_t) noexcept
    {
        _storage = nullopt;
        return *this;
    }

    template <typename T>
    inline constexpr optional<T>& optional<T>::operator=(const optional& other)
    {
        if (&other == this)
        {
            return *this;
        }
        _storage = other._storage;
        return *this;
    }

    template <typename T>
    inline constexpr optional<T>& optional<T>::operator=(optional&& other) noexcept(std::is_nothrow_move_assignable_v<T>)
    {
        if (&other == this)
        {
            return *this;
        }
        _storage = ryujin::move(other._storage);
        other._storage = nullopt;
        return *this;
    }

    template <typename T>
    template <typename U, std::enable_if_t<std::is_convertible_v<U&&, T>, int>>
    inline constexpr optional<T>& optional<T>::operator=(U&& value)
    {
        _storage = ryujin::move(T(ryujin::forward<U>(value)));
        return *this;
    }

    template <typename T>
    inline constexpr T* optional<T>::operator->() noexcept
    {
        return ryujin::get_if<T>(&_storage);
    }

    template <typename T>
    inline constexpr const T* optional<T>::operator->() const noexcept
    {
        return ryujin::get_if<T>(&_storage);
    }

    template <typename T>
    inline constexpr T& optional<T>::operator*()& noexcept
    {
        return *ryujin::get_if<T>(&_storage);
    }

    template <typename T>
    inline constexpr const T& optional<T>::operator*() const& noexcept
    {
        return *ryujin::get_if<T>(&_storage);
    }

    template <typename T>
    inline constexpr T&& optional<T>::operator*() && noexcept
    {
        return ryujin::get(ryujin::move(_storage));
    }

    template <typename T>
    inline constexpr const T&& optional<T>::operator*() const&& noexcept
    {
        return ryujin::get(ryujin::move(_storage));
    }

    template <typename T>
    inline constexpr optional<T>::operator bool() const noexcept
    {
        return _storage.index() != 0;
    }
    
    template <typename T>
    inline constexpr bool optional<T>::has_value() const noexcept
    {
        return _storage.index() != 0;
    }

    template <typename T>
    inline constexpr T& optional<T>::value()& noexcept
    {
        return ryujin::get<T>(_storage);
    }

    template <typename T>
    inline constexpr const T& optional<T>::value() const & noexcept
    {
        return ryujin::get<T>(_storage);
    }

    template <typename T>
    inline constexpr T&& optional<T>::value() && noexcept
    {
        return ryujin::get<T>(ryujin::move(_storage));
    }

    template <typename T>
    inline constexpr const T&& optional<T>::value() const&& noexcept
    {
        return ryujin::get<T>(ryujin::move(_storage));
    }

    template <typename T>
    template <typename U, std::enable_if_t<std::is_convertible_v<U&&, T>, int>>
    inline constexpr T optional<T>::value_or(U&& default_value) const&
    {
        if (has_value())
        {
            return value();
        }
        return T(forward<U>(default_value));
    }

    template <typename T>
    template <typename U, std::enable_if_t<std::is_convertible_v<U&&, T>, int>>
    inline constexpr T optional<T>::value_or(U&& default_value)&&
    {
        if (has_value())
        {
            return value();
        }
        return T(forward<U>(default_value));
    }

    template <typename T>
    inline constexpr void optional<T>::swap(optional& other) noexcept(std::is_nothrow_move_constructible_v<T>&& std::is_nothrow_swappable_v<T>)
    {
        _storage.swap(other._storage);
    }

    template <typename T>
    inline constexpr void optional<T>::reset()
    {
        _storage = nullopt;
    }

    template <typename T>
    template <typename ... Args>
    inline constexpr T& optional<T>::emplace(Args&& ... args)
    {
        _storage = T(std::forward<Args>(args)...);
        return ryujin::get<T>(_storage);
    }

    template <typename T>
    inline constexpr optional<std::decay_t<T>> make_optional(T&& value)
    {
        return optional<T>(ryujin::forward<T>(value));
    }

    template <typename T, typename ... Args>
    inline constexpr optional<T> make_optional(Args&& ... args)
    {
        return optional<T>(ryujin::forward<Args>(args)...);
    }
}

#endif // optional_hpp__
