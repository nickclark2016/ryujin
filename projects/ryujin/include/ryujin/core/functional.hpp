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

    namespace detail
    {
        template <sz FnSize = 2 * sizeof(void*)>
        union functor_storage_align
        {
            static constexpr sz stack_size = FnSize;
            char inlineMem[FnSize];
            char* heapAllocated = nullptr;
        };

        template <typename Fn, sz FnSize>
        struct is_functor_stack_allocatable
        {
            static constexpr bool value = sizeof(Fn) <= FnSize && alignof(functor_storage_align<FnSize>) % alignof(Fn) == 0;
        };

        template <typename Fn, sz FnSize>
        inline constexpr bool is_functor_stack_allocatable_v = is_functor_stack_allocatable<Fn, FnSize>::value;
    }

    template <typename T>
    class move_only_function;

    template <typename Ret, typename ... Args>
    class move_only_function<Ret(Args...)>
    {
        // Type erase the invoke and delete. No copy construction, so no copy constructor
        typedef Ret(*invoke_fn_t)(void*, Args&&...);
        typedef void(*construct_fn_t)(void*, void*);
        typedef void(*destroy_fn_t)(void*);
    public:
        move_only_function() noexcept;
        move_only_function(const move_only_function&) = delete;
        move_only_function(move_only_function&& fn) noexcept;
        move_only_function(nullptr_t) noexcept;

        template <typename Fn, std::enable_if_t<!std::is_same_v<std::remove_cvref_t<Fn>, move_only_function<Ret(Args...)>>, bool> = true>
        move_only_function(Fn&& f);

        ~move_only_function();

        move_only_function& operator=(const move_only_function&) = delete;
        move_only_function& operator=(move_only_function&& rhs) noexcept;
        move_only_function& operator=(nullptr_t) noexcept;

        template <typename Fn, std::enable_if_t<!std::is_same_v<std::remove_cvref_t<Fn>, move_only_function<Ret(Args...)>>, bool> = true>
        move_only_function& operator=(Fn&& f);

        Ret operator()(Args ... args) const;

        operator bool() const noexcept;

    private:
        template <typename Fn, typename Arg0, typename ... ArgRest>
        inline static Ret invoke_member_fn(Fn fn, Arg0 value, ArgRest&& ... args)
        {
            return (value.*fn)(ryujin::forward<ArgRest>(args)...);
        }

        template <typename Fn>
        inline static Ret invoke_fn(Fn* fn, Args&& ... args)
        {
            if constexpr (std::is_member_function_pointer_v<Fn>)
            {
                return invoke_member_fn(*fn, std::forward<Args>(args)...);
            }
            else
            {
                return (*fn)(ryujin::forward<Args>(args)...);
            }
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

        void* get_fn_ptr() const noexcept;
        void release_held_fn();

        invoke_fn_t _invoker;
        construct_fn_t _constructor;
        destroy_fn_t _destructor;
        detail::functor_storage_align<> _storage = { .heapAllocated = nullptr };
        bool _isInline = false;
    };

    namespace detail
    {
        template <typename>
        struct function_signature_extractor
        {
        };

        template <typename Res, typename Tp, typename ... Args>
        struct function_signature_extractor<Res(Tp::*)(Args...)>
        {
            using type = Res(Args...);
        };

        template <typename Res, typename Tp, typename ... Args>
        struct function_signature_extractor<Res(Tp::*)(Args...) &>
        {
            using type = Res(Args...);
        };

        template <typename Res, typename Tp, typename ... Args>
        struct function_signature_extractor<Res(Tp::*)(Args...) const>
        {
            using type = Res(Args...);
        };

        template <typename Res, typename Tp, typename ... Args>
        struct function_signature_extractor<Res(Tp::*)(Args...) const &>
        {
            using type = Res(Args...);
        };
    }

    template <typename Fn, typename Signature = typename detail::function_signature_extractor<decltype(&Fn::operator())>::type>
    move_only_function(Fn&&)->move_only_function<Signature>;

    template <typename Ret, typename ... Args>
    move_only_function(Ret(*)(Args...))->move_only_function<Ret(Args...)>;

    template <typename Ret, typename Type, typename ... Args>
    move_only_function(Ret(Type::*)(Args...))->move_only_function<Ret(Type, Args...)>;

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
    inline move_only_function<Ret(Args...)>::move_only_function() noexcept
        : _invoker(nullptr), _destructor(nullptr), _constructor(nullptr)
    {
        _isInline = false;
        _storage.heapAllocated = nullptr;
    }

    template<typename Ret, typename ...Args>
    inline move_only_function<Ret(Args...)>::move_only_function(move_only_function&& fn) noexcept
        : _invoker(fn._invoker), _constructor(fn._constructor), _destructor(fn._destructor), _storage(ryujin::move(fn._storage)), _isInline(fn._isInline)
    {
        fn._invoker = nullptr;
        fn._constructor = nullptr;
        fn._destructor = nullptr;
        fn._isInline = false;
        fn._storage.heapAllocated = nullptr;
    }

    template<typename Ret, typename ...Args>
    inline move_only_function<Ret(Args...)>::move_only_function(nullptr_t) noexcept
        : move_only_function()
    {
    }

    template<typename Ret, typename ...Args>
    template<typename Fn, std::enable_if_t<!std::is_same_v<std::remove_cvref_t<Fn>, move_only_function<Ret(Args...)>>, bool>>
    inline move_only_function<Ret(Args...)>::move_only_function(Fn&& f)
        : _invoker(reinterpret_cast<invoke_fn_t>(invoke_fn<Fn>)),
        _constructor(reinterpret_cast<construct_fn_t>(construct_fn<Fn>)),
        _destructor(reinterpret_cast<destroy_fn_t>(destroy_fn<Fn>))
    {
        if constexpr (detail::is_functor_stack_allocatable_v<Fn, decltype(_storage)::stack_size>)
        {
            _constructor(_storage.inlineMem, &f);
            _isInline = true;
        }
        else
        {
            _storage.heapAllocated = new char[sizeof(Fn)];
            _constructor(_storage.heapAllocated, &f);
            _isInline = false;
        }
    }

    template<typename Ret, typename ...Args>
    template <typename Fn, std::enable_if_t<!std::is_same_v<std::remove_cvref_t<Fn>, move_only_function<Ret(Args...)>>, bool>>
    inline move_only_function<Ret(Args...)>& move_only_function<Ret(Args...)>::operator=(Fn&& f)
    {
        release_held_fn();

        if constexpr (detail::is_functor_stack_allocatable_v<Fn, decltype(_storage)::stack_size>)
        {
            _constructor(_storage.inlineMem, &f);
            _isInline = true;
        }
        else
        {
            _storage.heapAllocated = new char[sizeof(Fn)];
            _constructor(_storage.heapAllocated, &f);
            _isInline = false;
        }

        return *this;
    }

    template<typename Ret, typename ...Args>
    inline move_only_function<Ret(Args...)>::~move_only_function()
    {
        release_held_fn();
    }

    template<typename Ret, typename ...Args>
    inline move_only_function<Ret(Args...)>& move_only_function<Ret(Args...)>::operator=(move_only_function&& rhs) noexcept
    {
        if (&rhs == this || get_fn_ptr() == rhs.get_fn_ptr())
        {
            return *this;
        }

        release_held_fn();

        _invoker = rhs._invoker;
        _constructor = rhs._constructor;
        _destructor = rhs._destructor;
        _storage = ryujin::move(rhs._storage);
        _isInline = rhs._isInline;

        rhs._invoker = nullptr;
        rhs._constructor = nullptr;
        rhs._destructor = nullptr;
        rhs._storage.heapAllocated = nullptr;
        rhs._isInline = false;

        return *this;
    }

    template<typename Ret, typename ...Args>
    inline move_only_function<Ret(Args...)>& move_only_function<Ret(Args...)>::operator=(nullptr_t) noexcept
    {
        release_held_fn();

        _invoker = nullptr;
        _constructor = nullptr;
        _destructor = nullptr;

        return *this;
    }

    template<typename Ret, typename ...Args>
    inline Ret move_only_function<Ret(Args...)>::operator()(Args ...args) const
    {
        return _invoker(get_fn_ptr(), ryujin::forward<Args>(args)...);
    }

    template<typename Ret, typename ...Args>
    inline move_only_function<Ret(Args...)>::operator bool() const noexcept
    {
        return get_fn_ptr() != nullptr;
    }
    
    template<typename Ret, typename ...Args>
    inline void* move_only_function<Ret(Args...)>::get_fn_ptr() const noexcept
    {
        if (_isInline)
        {
            return const_cast<char*>(_storage.inlineMem);
        }
        return _storage.heapAllocated;
    }
    
    template<typename Ret, typename ...Args>
    inline void move_only_function<Ret(Args...)>::release_held_fn()
    {
        void* ptr = get_fn_ptr();
        if (ptr)
        {
            _destructor(ptr);
            if (!_isInline)
            {
                delete[] _storage.heapAllocated;
                _isInline = false;
            }
            _storage.heapAllocated = nullptr;
        }
    }
}

#endif // functional_hpp__
