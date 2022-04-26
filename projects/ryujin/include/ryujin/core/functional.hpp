#ifndef functional_hpp__
#define functional_hpp__

#include "smart_pointers.hpp"

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

    template <typename T>
    class no_move_function;

    template <typename Ret, typename ... Args>
    class no_move_function<Ret(Args...)>
    {
        // Type erase the invoke and delete. No copy construction, so no copy constructor
        typedef Ret(*invoke_fn_t)(void*, Args&&...);
        typedef void(*construct_fn_t)(void*, void*);
        typedef void(*destroy_fn_t)(void*);
    public:
        no_move_function() noexcept;
        no_move_function(const no_move_function&) = delete;
        no_move_function(no_move_function&& fn) noexcept;

        template <typename Fn>
        no_move_function(Fn f);

        ~no_move_function();

        no_move_function& operator=(const no_move_function&) = delete;
        no_move_function& operator=(no_move_function&& rhs) noexcept;

        Ret operator()(Args ... args) const;

    private:

        template <typename Fn>
        inline static Ret invoke_fn(Fn* fn, Args&& ... args)
        {
            return (*fn)(ryujin::forward<Args>(args)...);
        }

        template <typename Fn>
        inline static void construct_fn(Fn* dst, Fn* src)
        {
            static_assert(std::is_constructible_v<Fn, decltype(*dst)>, "Cannot construct functor.");
            ::new (dst) Fn(*src);
        }

        template <typename Fn>
        inline static void destroy_fn(Fn* f)
        {
            f->~Fn();
        }

        invoke_fn_t _invoker;
        construct_fn_t _constructor;
        destroy_fn_t _destructor;
        unique_ptr<char[]> _data;
        sz _dataSize;
    };

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
    
    template<typename Ret, typename ...Args>
    inline no_move_function<Ret(Args...)>::no_move_function() noexcept
        : _invoker(nullptr), _destructor(nullptr), _data(nullptr), _dataSize(0)
    {
    }

    template<typename Ret, typename ...Args>
    inline no_move_function<Ret(Args...)>::no_move_function(no_move_function&& fn) noexcept
        : _invoker(fn._invoker), _constructor(fn._constructor), _destructor(fn._destructor), _data(ryujin::move(fn._data)), _dataSize(fn._dataSize)
    {
        fn._invoker = nullptr;
        fn._constructor = nullptr;
        fn._destructor = nullptr;
        fn._data = nullptr;
        fn._dataSize = 0;
    }

    template<typename Ret, typename ...Args>
    inline no_move_function<Ret(Args...)>::~no_move_function()
    {
        if (_data)
        {
            _destructor(_data.get());
        }
    }

    template<typename Ret, typename ...Args>
    inline no_move_function<Ret(Args...)>& no_move_function<Ret(Args...)>::operator=(no_move_function&& rhs) noexcept
    {
        if (&rhs == this || _data.get() == rhs._data.get())
        {
            return *this;
        }

        if (_data)
        {
            _destructor(_data.get());
        }

        _invoker = rhs._invoker;
        _constructor = rhs._constructor;
        _destructor = rhs._destructor;
        _data = ryujin::move(rhs._data);
        _dataSize = rhs._dataSize;

        rhs._invoker = nullptr;
        rhs._constructor = nullptr;
        rhs._destructor = nullptr;
        rhs._data = nullptr; // should be handled by the move, just a sanity check
        rhs._dataSize = 0;

        return *this;
    }

    template<typename Ret, typename ...Args>
    inline Ret no_move_function<Ret(Args...)>::operator()(Args ...args) const
    {
        return _invoker(_data.get(), ryujin::forward<Args>(args)...);
    }
        
    template<typename Ret, typename ...Args>
    template<typename Fn>
    inline no_move_function<Ret(Args...)>::no_move_function(Fn f)
        : _invoker(reinterpret_cast<invoke_fn_t>(invoke_fn<Fn>)),
        _constructor(reinterpret_cast<construct_fn_t>(construct_fn<Fn>)),
        _destructor(reinterpret_cast<destroy_fn_t>(destroy_fn<Fn>)),
        _data(new char[sizeof(Fn)]),
        _dataSize(sizeof(Fn))
    {
        _constructor(_data.get(), &f);
    }
}

#endif // functional_hpp__
