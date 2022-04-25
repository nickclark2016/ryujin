#ifndef functional_hpp__
#define functional_hpp__

namespace ryujin
{
    template <typename T>
    class reference_wrapper
    {
    public:
        template <typename U>
        constexpr reference_wrapper(U&& val);

        constexpr reference_wrapper(const reference_wrapper& other) noexcept;

        constexpr reference_wrapper& operator=(const reference_wrapper& other) noexcept;

        constexpr operator T& () const noexcept;
        constexpr T& get() const noexcept;
    private:
        T* _ptr;
    };

    template <typename T>
    reference_wrapper(T&)->reference_wrapper<T>;

    template<typename T>
    template<typename U>
    inline constexpr reference_wrapper<T>::reference_wrapper(U&& val)
    {
        T& ref = static_cast<U>(val);
        _ptr = &ref;
    }

    template<typename T>
    inline constexpr reference_wrapper<T>::reference_wrapper(const reference_wrapper& other) noexcept
    {
        _ptr = other._ptr;
    }
    
    template<typename T>
    inline constexpr reference_wrapper<T>& reference_wrapper<T>::operator=(const reference_wrapper& other) noexcept
    {
        _ptr = other._ptr;
        return *this;
    }

    template<typename T>
    inline constexpr reference_wrapper<T>::operator T& () const noexcept
    {
        return *_ptr;
    }

    template<typename T>
    inline constexpr T& reference_wrapper<T>::get() const noexcept
    {
        return *_ptr;
    }

    template <typename T>
    inline constexpr reference_wrapper<T> ref(T& t) noexcept
    {
        return reference_wrapper<T>(t);
    }

    template <typename T>
    void ref(const T&&) = delete;

    template <typename T>
    inline constexpr reference_wrapper<const T> cref(const T& t) noexcept
    {
        return reference_wrapper<const T>(t);
    }

    template <typename T>
    void cref(const T&&) = delete;
}

#endif // functional_hpp__
