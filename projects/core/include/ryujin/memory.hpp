#ifndef memory_hpp__
#define memory_hpp__

#include "nullptr.hpp"
#include "utility.hpp"

#include <new>

namespace ryujin
{
    template <typename T, typename ... Args>
    inline constexpr T* construct_at(T* p, Args&& ... args) noexcept(noexcept(::new((void*)0) T(declval<Args>()...)))
    {
        return ::new((void*)p) T(ryujin::forward<Args>(args)...);
    }

    template <typename T>
    inline constexpr void destroy_at(T* p)
    {
        if constexpr (is_array_v<T>)
        {
            for (auto& x : *p)
            {
                destroy_at(&x);
            }
        }
        else
        {
            p->~T();
        }
    }

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

    namespace detail
    {
        template <typename T, typename Deleter>
        struct alignas(alignof(T)) control_block
        {
            char data[sizeof(T)];
            T* externalAllocatedPtr = nullptr;
            sz owning = 0;
            Deleter dtr;
        };
    }

    template <typename T, typename Deleter = default_delete<T>>
    class shared_ptr
    {
    public:
        using pointer = T*;
        using element_type = remove_extent_t<T>;

        shared_ptr();
        shared_ptr(nullptr_t);
        shared_ptr(const shared_ptr& ptr);
        shared_ptr(shared_ptr&& ptr) noexcept;

        template <typename U, enable_if_t<is_convertible_v<U*, T*>, int> = 0>
        shared_ptr(U* ptr);

        ~shared_ptr();

        shared_ptr& operator=(const shared_ptr& rhs) noexcept;
        shared_ptr& operator=(shared_ptr&& rhs) noexcept;
        shared_ptr& operator=(nullptr_t) noexcept;

        template <typename U, enable_if_t<is_convertible_v<U*, T*>, int> = 0>
        shared_ptr& operator=(U* ptr);

        void reset();

        template <typename U, enable_if_t<is_convertible_v<U*, T*>, int> = 0>
        void reset(U* ptr);

        void swap(shared_ptr& s);

        element_type* get() const noexcept;
        element_type* operator->() const noexcept;
        element_type& operator*() noexcept;
        const element_type& operator*() const noexcept;
    private:
        detail::control_block<T, Deleter>* _ctrl = {};

        void _decrement_ownership();
        void _increment_ownership();

        template <typename ... Args>
        friend shared_ptr<T> make_shared(Args&& ... args);
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

    template <typename T, typename ... Args, enable_if_t<!is_array_v<T>, int> = 0>
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

    template <typename T, enable_if_t<is_array_v<T>, int> = 0>
    inline constexpr unique_ptr<T> make_unique(sz count)
    {
        using base = remove_extent_t<T>;
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

    template <typename T, typename Deleter>
    inline shared_ptr<T, Deleter>::shared_ptr()
    {
    }

    template <typename T, typename Deleter>
    inline shared_ptr<T, Deleter>::shared_ptr(const shared_ptr& ptr)
        : _ctrl(ptr._ctrl)
    {
        _increment_ownership();
    }

    template <typename T, typename Deleter>
    inline shared_ptr<T, Deleter>::shared_ptr(shared_ptr&& ptr) noexcept
        : _ctrl(ptr._ctrl)
    {
        ptr._ctrl = nullptr;
    }

    template <typename T, typename Deleter>
    inline shared_ptr<T, Deleter>::shared_ptr(nullptr_t)
    {
    }

    template<typename T, typename Deleter>
    template<typename U, enable_if_t<is_convertible_v<U*, T*>, int>>
    inline shared_ptr<T, Deleter>::shared_ptr(U* ptr)
    {
        if (ptr)
        {
            detail::control_block<T, Deleter>* control = new detail::control_block<T, Deleter>;
            control->externalAllocatedPtr = static_cast<T*>(ptr);
            ++control->owning;

            _ctrl = control;
        }
    }

    template<typename T, typename Deleter>
    inline shared_ptr<T, Deleter>::~shared_ptr()
    {
        _decrement_ownership();
        _ctrl = nullptr;
    }

    template<typename T, typename Deleter>
    inline shared_ptr<T, Deleter>& shared_ptr<T, Deleter>::operator=(const shared_ptr& rhs) noexcept
    {
        if (&rhs == this || rhs._ctrl == _ctrl)
        {
            return *this;
        }

        _decrement_ownership();
        _ctrl = rhs._ctrl;
        ++_ctrl->owning;

        return *this;
    }

    template<typename T, typename Deleter>
    inline shared_ptr<T, Deleter>& shared_ptr<T, Deleter>::operator=(shared_ptr&& rhs) noexcept
    {
        if (&rhs == this || rhs._ctrl == _ctrl)
        {
            return *this;
        }

        _decrement_ownership();
        _ctrl = rhs._ctrl;

        return *this;
    }

    template<typename T, typename Deleter>
    inline shared_ptr<T, Deleter>& shared_ptr<T, Deleter>::operator=(nullptr_t) noexcept
    {
        if (_ctrl == nullptr)
        {
            return *this;
        }

        _decrement_ownership();

        return *this;
    }

    template<typename T, typename Deleter>
    template <typename U, enable_if_t<is_convertible_v<U*, T*>, int>>
    inline shared_ptr<T, Deleter>& shared_ptr<T, Deleter>::operator=(U* ptr)
    {
        if (ptr == this->_ctrl->externalAllocatedPtr)
        {
            return *this;
        }

        _decrement_ownership();

        detail::control_block<T, Deleter>* control = new detail::control_block<T, Deleter>;
        control->externalAllocatedPtr = ptr;
        ++control->owning;

        return *this;
    }

    template<typename T, typename Deleter>
    inline void shared_ptr<T, Deleter>::reset()
    {
        _decrement_ownership();
    }

    template<typename T, typename Deleter>
    template<typename U, enable_if_t<is_convertible_v<U*, T*>, int>>
    inline void shared_ptr<T, Deleter>::reset(U* ptr)
    {
        if (ptr == this->_ctrl->externalAllocatedPtr)
        {
            return;
        }

        _decrement_ownership();

        detail::control_block<T, Deleter>* control = new detail::control_block<T, Deleter>;
        control->externalAllocatedPtr = ptr;
        ++control->owning;
    }

    template<typename T, typename Deleter>
    inline void shared_ptr<T, Deleter>::swap(shared_ptr& s)
    {
        ryujin::move_swap(_ctrl, s._ctrl);
    }

    template<typename T, typename Deleter>
    inline typename shared_ptr<T, Deleter>::element_type* shared_ptr<T, Deleter>::get() const noexcept
    {
        if (_ctrl)
        {
            return _ctrl->externalAllocatedPtr ? _ctrl->externalAllocatedPtr : reinterpret_cast<element_type*>(_ctrl->data);
        }
        return nullptr;
    }

    template<typename T, typename Deleter>
    inline typename shared_ptr<T, Deleter>::element_type* shared_ptr<T, Deleter>::operator->() const noexcept
    {
        return get();
    }

    template<typename T, typename Deleter>
    inline const typename shared_ptr<T, Deleter>::element_type& shared_ptr<T, Deleter>::operator*() const noexcept
    {
        return *get();
    }

    template<typename T, typename Deleter>
    inline typename shared_ptr<T, Deleter>::element_type& shared_ptr<T, Deleter>::operator*() noexcept
    {
        return *get();
    }

    template<typename T, typename Deleter>
    inline void shared_ptr<T, Deleter>::_decrement_ownership()
    {
        if (_ctrl)
        {
            --_ctrl->owning;
            if (_ctrl->owning == 0)
            {
                if (_ctrl->externalAllocatedPtr)
                {
                    _ctrl->dtr(_ctrl->externalAllocatedPtr);
                }
                else
                {
                    ryujin::destroy_at(reinterpret_cast<T*>(_ctrl->data));
                }
                delete _ctrl;
                _ctrl = nullptr;
            }
        }
    }

    template<typename T, typename Deleter>
    inline void shared_ptr<T, Deleter>::_increment_ownership()
    {
        if (_ctrl)
        {
            ++_ctrl->owning;
        }
    }

    template <typename T, typename ... Args>
    inline shared_ptr<T> make_shared(Args&& ... args)
    {
        shared_ptr<T> ptr;
        ptr._ctrl = new detail::control_block<T, default_delete<T>>();
        ryujin::construct_at(reinterpret_cast<T*>(ptr._ctrl->data)> T(ryujin::forward<Args>(args)...));
        ptr._ctrl->owning = 1;
        ptr._ctrl->externalAllocatedPtr = nullptr;

        return ptr;
    }

    template <typename T, typename Deleter>
    inline constexpr auto operator<=>(const shared_ptr<T, Deleter>& lhs, const shared_ptr<T, Deleter>& rhs)
    {
        return lhs.get() <=> rhs.get();
    }

    /// <summary>
    /// Specialization of hash for a unique pointer.
    /// </summary>
    /// \ingroup hashes
    template <typename T, typename Deleter>
    struct hash<unique_ptr<T, Deleter>>
    {
        inline constexpr sz operator()(const unique_ptr<T, Deleter>& v) noexcept
        {
            return hash<sz>()(v.get());
        }
    };

    /// <summary>
    /// Specialization of hash for a shared pointer.
    /// </summary>
    /// \ingroup hashes
    template <typename T, typename Deleter>
    struct hash<shared_ptr<T, Deleter>>
    {
        inline constexpr sz operator()(const shared_ptr<T, Deleter>& v) noexcept
        {
            return hash<sz>()(v.get());
        }
    };
}

#endif // memory_hpp__
