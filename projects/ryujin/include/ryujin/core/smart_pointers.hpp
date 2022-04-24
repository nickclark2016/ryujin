#ifndef smart_pointers_hpp__
#define smart_pointers_hpp__

#include "nullptr.hpp"

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

        constexpr unique_ptr() noexcept;
        constexpr unique_ptr(nullptr_t) noexcept;
        constexpr unique_ptr(pointer p) noexcept;
        unique_ptr(const unique_ptr&) = delete;
        constexpr unique_ptr(unique_ptr&& p) noexcept;
        ~unique_ptr();

        unique_ptr& operator=(const unique_ptr&) = delete;
        unique_ptr& operator=(unique_ptr&& rhs) noexcept;
        unique_ptr& operator=(pointer p) noexcept;

        pointer release();
        void reset(pointer p = pointer());
        void swap(unique_ptr& other);

        pointer get() const noexcept;
        Deleter& get_deleter() noexcept;
        const Deleter& get_deleter() const noexcept;
        operator bool() const noexcept;

        pointer operator->() const noexcept;
        T& operator*() noexcept;
        const T& operator*() const noexcept;

    private:
        T* _ptr;
        Deleter _dtr;
    };

    template <typename T, typename ... Args>
    unique_ptr<T> make_unique(Args&& ... args)
    {

    }
}

#endif // smart_pointers_hpp__
