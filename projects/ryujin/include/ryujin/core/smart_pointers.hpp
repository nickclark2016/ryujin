#ifndef smart_pointers_hpp__
#define smart_pointers_hpp__

#include "nullptr.hpp"
#include "utility.hpp"

namespace ryujin
{
    template <typename T>
    struct default_delete
    {
        inline void operator()(T* ptr) const
        {
            if (ptr)
            {
                delete ptr;
            }
        }
    };

    template <typename T>
    struct default_delete<T[]>
    {
        inline void operator()(T* ptr) const
        {
            if (ptr)
            {
                delete[] ptr;
            }
        }
    };

    template <typename T, typename Deleter = default_delete<T>>
    class unique_ptr
    {
    public:
        using pointer = T*;
        using element_type = T;

        constexpr unique_ptr() noexcept = default;
        constexpr unique_ptr(nullptr_t) noexcept;
        constexpr unique_ptr(pointer p) noexcept;
        unique_ptr(const unique_ptr&) = delete;
        constexpr unique_ptr(unique_ptr&& p) noexcept;
        constexpr ~unique_ptr();

        unique_ptr& operator=(const unique_ptr&) = delete;
        constexpr unique_ptr& operator=(unique_ptr&& rhs) noexcept;
        constexpr unique_ptr& operator=(pointer p) noexcept;

        constexpr pointer release();
        constexpr void reset(pointer p = pointer());
        constexpr void swap(unique_ptr& other);

        constexpr pointer get() const noexcept;
        constexpr Deleter& get_deleter() noexcept;
        constexpr const Deleter& get_deleter() const noexcept;
        constexpr operator bool() const noexcept;

        constexpr pointer operator->() const noexcept;
        constexpr T& operator*() noexcept;
        constexpr const T& operator*() const noexcept;

    private:
        T* _ptr = {};
        Deleter _dtr = {};
    };

    template <typename T, typename Deleter>
    class unique_ptr<T[], Deleter>
    {
    public:
        using pointer = T*;
        using element_type = T;

        constexpr unique_ptr() noexcept = default;
        constexpr unique_ptr(nullptr_t) noexcept;
        constexpr unique_ptr(pointer p) noexcept;
        unique_ptr(const unique_ptr&) = delete;
        constexpr unique_ptr(unique_ptr&& p) noexcept;
        constexpr ~unique_ptr();

        unique_ptr& operator=(const unique_ptr&) = delete;
        constexpr unique_ptr& operator=(unique_ptr&& rhs) noexcept;
        constexpr unique_ptr& operator=(pointer p) noexcept;

        constexpr pointer release();
        constexpr void reset(pointer p = pointer());
        constexpr void swap(unique_ptr& other);

        constexpr pointer get() const noexcept;
        constexpr Deleter& get_deleter() noexcept;
        constexpr const Deleter& get_deleter() const noexcept;
        constexpr operator bool() const noexcept;

        constexpr pointer operator->() const noexcept;
        constexpr T& operator*() noexcept;
        constexpr const T& operator*() const noexcept;

        constexpr T& operator[](const sz idx) noexcept;
        constexpr const T& operator[](const sz idx) const noexcept;

    private:
        T* _ptr = {};
        Deleter _dtr = {};
    };

    template<typename T, typename Deleter>
    inline constexpr unique_ptr<T, Deleter>::unique_ptr(nullptr_t) noexcept
        : _ptr(nullptr)
    {
    }

    template <typename T, typename Deleter>
    inline constexpr unique_ptr<T, Deleter>::unique_ptr(pointer p) noexcept
        : _ptr(p)
    {
    }

    template <typename T, typename Deleter>
    inline constexpr unique_ptr<T, Deleter>::unique_ptr(unique_ptr&& other) noexcept
        : _ptr(other._ptr), _dtr(ryujin::move(other._dtr))
    {
        other._ptr = nullptr;
    }

    template <typename T, typename Deleter>
    inline constexpr unique_ptr<T, Deleter>::~unique_ptr()
    {
        _dtr(_ptr);
    }

    template <typename T, typename Deleter>
    inline constexpr unique_ptr<T, Deleter>& unique_ptr<T, Deleter>::operator=(unique_ptr&& rhs) noexcept
    {
        if (&rhs == this)
        {
            return *this;
        }

        _dtr(_ptr);
        _ptr = rhs._ptr;
        _dtr = ryujin::move(rhs._dtr);
        rhs._ptr = nullptr;

        return *this;
    }
    
    
    template <typename T, typename Deleter>
    inline constexpr unique_ptr<T, Deleter>& unique_ptr<T, Deleter>::operator=(pointer p) noexcept
    {
        if (p == _ptr)
        {
            return *this;
        }

        _dtr(_ptr);
        _ptr = p;

        return *this;
    }

    template <typename T, typename Deleter>
    inline constexpr typename unique_ptr<T, Deleter>::pointer unique_ptr<T, Deleter>::release()
    {
        auto p = _ptr;
        _ptr = nullptr;
        return p;
    }
    
    template <typename T, typename Deleter>
    inline constexpr void unique_ptr<T, Deleter>::reset(pointer p)
    {
        if (p == _ptr)
        {
            return;
        }

        _dtr(_ptr);
        _ptr = p;
    }
    
    template <typename T, typename Deleter>
    inline constexpr void unique_ptr<T, Deleter>::swap(unique_ptr& other)
    {
        ryujin::move_swap(_ptr, other._ptr);
        ryujin::move_swap(_dtr, other._dtr);
    }

    template <typename T, typename Deleter>
    inline constexpr typename unique_ptr<T, Deleter>::pointer unique_ptr<T, Deleter>::get() const noexcept
    {
        return _ptr;
    }

    template <typename T, typename Deleter>
    inline constexpr Deleter& unique_ptr<T, Deleter>::get_deleter() noexcept
    {
        return _dtr;
    }

    template <typename T, typename Deleter>
    inline constexpr const Deleter& unique_ptr<T, Deleter>::get_deleter() const noexcept
    {
        return _dtr;
    }

    template <typename T, typename Deleter>
    inline constexpr unique_ptr<T, Deleter>::operator bool() const noexcept
    {
        return _ptr != nullptr;
    }

    template <typename T, typename Deleter>
    inline constexpr typename unique_ptr<T, Deleter>::pointer unique_ptr<T, Deleter>::operator->() const noexcept
    {
        return _ptr;
    }
    
    template <typename T, typename Deleter>
    inline constexpr T& unique_ptr<T, Deleter>::operator*() noexcept
    {
        return *_ptr;
    }
    
    template <typename T, typename Deleter>
    inline constexpr const T& unique_ptr<T, Deleter>::operator*() const noexcept
    {
        return *_ptr;
    }

    template <typename T, typename ... Args, std::enable_if_t<!std::is_array_v<T>, int> = 0>
    inline constexpr unique_ptr<T> make_unique(Args&& ... args)
    {
        T* ptr = new T(ryujin::forward<Args>(args)...);
        return unique_ptr<T>(ptr);
    }

    template <typename T, typename Deleter>
    inline constexpr void swap(unique_ptr<T, Deleter>& a, unique_ptr<T, Deleter>& b)
    {
        a.swap(b);
    }

    template <typename T, typename Deleter>
    inline constexpr auto operator<=>(const unique_ptr<T, Deleter>& a, const unique_ptr<T, Deleter>& b)
    {
        return a.get() <=> b.get();
    }

    template<typename T, typename Deleter>
    inline constexpr unique_ptr<T[], Deleter>::unique_ptr(nullptr_t) noexcept
        : _ptr(nullptr)
    {
    }

    template <typename T, typename Deleter>
    inline constexpr unique_ptr<T[], Deleter>::unique_ptr(pointer p) noexcept
        : _ptr(p)
    {
    }

    template <typename T, typename Deleter>
    inline constexpr unique_ptr<T[], Deleter>::unique_ptr(unique_ptr&& other) noexcept
        : _ptr(other._ptr), _dtr(ryujin::move(other._dtr))
    {
        other._ptr = nullptr;
    }

    template <typename T, typename Deleter>
    inline constexpr unique_ptr<T[], Deleter>::~unique_ptr()
    {
        _dtr(_ptr);
    }

    template <typename T, typename Deleter>
    inline constexpr unique_ptr<T[], Deleter>& unique_ptr<T[], Deleter>::operator=(unique_ptr&& rhs) noexcept
    {
        if (&rhs == this)
        {
            return *this;
        }

        _dtr(_ptr);
        _ptr = rhs._ptr;
        _dtr = ryujin::move(rhs._dtr);
        rhs._ptr = nullptr;

        return *this;
    }


    template <typename T, typename Deleter>
    inline constexpr unique_ptr<T[], Deleter>& unique_ptr<T[], Deleter>::operator=(pointer p) noexcept
    {
        if (p == _ptr)
        {
            return *this;
        }

        _dtr(_ptr);
        _ptr = p;

        return *this;
    }

    template <typename T, typename Deleter>
    inline constexpr typename unique_ptr<T[], Deleter>::pointer unique_ptr<T[], Deleter>::release()
    {
        auto p = _ptr;
        _ptr = nullptr;
        return p;
    }

    template <typename T, typename Deleter>
    inline constexpr void unique_ptr<T[], Deleter>::reset(pointer p)
    {
        if (p == _ptr)
        {
            return;
        }

        _dtr(_ptr);
        _ptr = p;
    }

    template <typename T, typename Deleter>
    inline constexpr void unique_ptr<T[], Deleter>::swap(unique_ptr& other)
    {
        ryujin::move_swap(_ptr, other._ptr);
        ryujin::move_swap(_dtr, other._dtr);
    }

    template <typename T, typename Deleter>
    inline constexpr typename unique_ptr<T[], Deleter>::pointer unique_ptr<T[], Deleter>::get() const noexcept
    {
        return _ptr;
    }

    template <typename T, typename Deleter>
    inline constexpr Deleter& unique_ptr<T[], Deleter>::get_deleter() noexcept
    {
        return _dtr;
    }

    template <typename T, typename Deleter>
    inline constexpr const Deleter& unique_ptr<T[], Deleter>::get_deleter() const noexcept
    {
        return _dtr;
    }

    template <typename T, typename Deleter>
    inline constexpr unique_ptr<T[], Deleter>::operator bool() const noexcept
    {
        return _ptr != nullptr;
    }

    template <typename T, typename Deleter>
    inline constexpr typename unique_ptr<T[], Deleter>::pointer unique_ptr<T[], Deleter>::operator->() const noexcept
    {
        return _ptr;
    }

    template <typename T, typename Deleter>
    inline constexpr T& unique_ptr<T[], Deleter>::operator*() noexcept
    {
        return *_ptr;
    }

    template <typename T, typename Deleter>
    inline constexpr const T& unique_ptr<T[], Deleter>::operator*() const noexcept
    {
        return *_ptr;
    }

    template <typename T, typename Deleter>
    inline constexpr T& unique_ptr<T[], Deleter>::operator[](const sz idx) noexcept
    {
        return _ptr[idx];
    }

    template <typename T, typename Deleter>
    inline constexpr const T& unique_ptr<T[], Deleter>::operator[](const sz idx) const noexcept
    {
        return _ptr[idx];
    }

    template <typename T, std::enable_if_t<std::is_array_v<T>, int> = 0>
    inline constexpr unique_ptr<T> make_unique(sz count)
    {
        using base = std::remove_extent_t<T>;
        return unique_ptr<base[]>(new base[count]());
    }

    template <typename T, typename Deleter>
    inline constexpr void swap(unique_ptr<T[], Deleter>& a, unique_ptr<T[], Deleter>& b)
    {
        a.swap(b);
    }

    template <typename T, typename Deleter>
    inline constexpr auto operator<=>(const unique_ptr<T[], Deleter>& a, const unique_ptr<T[], Deleter>& b)
    {
        return a.get() <=> b.get();
    }
}

#endif // smart_pointers_hpp__
