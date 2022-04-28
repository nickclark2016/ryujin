#ifndef functional_hpp__
#define functional_hpp__

#include "smart_pointers.hpp"
#include "type_traits.hpp"

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

        template <typename Ret, bool Nx, typename ... Args>
        class move_only_function_base
        {
        protected:
            // Type erase the invoke and delete. No copy construction, so no copy constructor
            using invoke_fn_t = Ret(*)(void*, Args&&...);
            using construct_fn_t = void(*)(void*, void*);
            using destroy_fn_t = void(*)(void*);

            invoke_fn_t _invoker;
            construct_fn_t _constructor;
            destroy_fn_t _destructor;
            detail::functor_storage_align<> _storage = { .heapAllocated = nullptr };
            bool _isInline = false;

            template <typename Fn, typename Arg0, typename ... ArgRest>
            inline static Ret invoke_member_fn(Fn fn, Arg0 value, ArgRest&& ... args) noexcept(Nx)
            {
                return (value.*fn)(ryujin::forward<ArgRest>(args)...);
            }

            template <typename Fn>
            inline static Ret invoke_fn(Fn* fn, Args&& ... args) noexcept(Nx)
            {
                if constexpr (is_member_function_pointer_v<Fn>)
                {
                    return invoke_member_fn(*fn, ryujin::forward<Args>(args)...);
                }
                else
                {
                    return (*fn)(ryujin::forward<Args>(args)...);
                }
            }

            template <typename Fn>
            inline static void construct_fn(Fn* dst, Fn* src)
            {
                static_assert(is_constructible_v<Fn, decltype(*dst)>, "Cannot construct functor.");
                ::new (dst) Fn(*src);
            }

            template <typename Fn>
            inline static void destroy_fn(Fn* f)
            {
                f->~Fn();
            }

            inline void* get_fn_ptr() const noexcept
            {
                if (_isInline)
                {
                    return const_cast<char*>(_storage.inlineMem);
                }
                return _storage.heapAllocated;
            }

            inline void release_held_fn()
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

            inline void construct_empty()
            {
                _invoker = nullptr;
                _constructor = nullptr;
                _destructor = nullptr;
                _isInline = false;
                _storage.heapAllocated = true;
            }

            template <typename Fn, enable_if_t<!is_same_v<remove_cvref_t<Fn>, move_only_function_base<Ret, Nx, Args...>>, bool> = true>
            inline void construct_from_functor(Fn&& f)
            {
                _invoker = reinterpret_cast<invoke_fn_t>(invoke_fn<remove_cvref_t<Fn>>);
                _constructor = reinterpret_cast<construct_fn_t>(construct_fn<remove_cvref_t<Fn>>);
                _destructor = reinterpret_cast<destroy_fn_t>(destroy_fn<remove_cvref_t<Fn>>);

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

            inline void construct_move(move_only_function_base&& rhs)
            {
                this->_invoker = rhs._invoker;
                this->_constructor = rhs._constructor;
                this->_destructor = rhs._destructor;
                this->_storage = ryujin::move(rhs._storage);
                this->_isInline = rhs._isInline;

                rhs._invoker = nullptr;
                rhs._constructor = nullptr;
                rhs._destructor = nullptr;
                rhs._isInline = false;
                rhs._storage.heapAllocated = nullptr;
            }

            inline void move_assign(move_only_function_base&& rhs)
            {
                if (&rhs == this || get_fn_ptr() == rhs.get_fn_ptr())
                {
                    return;
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
            }

            template <typename Fn, enable_if_t<!is_same_v<remove_cvref_t<Fn>, move_only_function_base<Ret, Nx, Args...>>, bool> = true>
            inline void move_assign_from_functor(Fn&& f)
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
            }

        public:
            inline move_only_function_base() noexcept
                : _invoker(nullptr), _destructor(nullptr), _constructor(nullptr)
            {
                _isInline = false;
                _storage.heapAllocated = nullptr;
            }

            move_only_function_base(const move_only_function_base&) = delete;

            inline move_only_function_base(move_only_function_base&& fn) noexcept
                : _invoker(fn._invoker), _constructor(fn._constructor), _destructor(fn._destructor), _storage(ryujin::move(fn._storage)), _isInline(fn._isInline)
            {
                fn._invoker = nullptr;
                fn._constructor = nullptr;
                fn._destructor = nullptr;
                fn._isInline = false;
                fn._storage.heapAllocated = nullptr;
            }

            inline ~move_only_function_base()
            {
                release_held_fn();
            }

            template <typename Fn, enable_if_t<!is_same_v<remove_cvref_t<Fn>, move_only_function_base<Ret, Nx, Args...>>, bool> = true>
            inline move_only_function_base(Fn&& f)
            {
                construct_from_functor(ryujin::forward<Fn>(f));
            }

            inline move_only_function_base& operator=(move_only_function_base&& rhs) noexcept
            {
                move_assign(ryujin::forward<move_only_function_base>(rhs));

                return *this;
            }

            move_only_function_base& operator=(const move_only_function_base&) = delete;

            inline move_only_function_base& operator=(nullptr_t) noexcept
            {
                release_held_fn();

                _invoker = nullptr;
                _constructor = nullptr;
                _destructor = nullptr;

                return *this;
            }

            template <typename Fn, enable_if_t<!is_same_v<remove_cvref_t<Fn>, move_only_function_base<Ret, Nx, Args...>>, bool> = true>
            inline move_only_function_base& operator=(Fn&& f)
            {
                move_assign_from_functor(ryujin::forward<Fn>(f));

                return *this;
            }

            inline operator bool() const noexcept
            {
                return get_fn_ptr() != nullptr;
            }
        };

        template <typename ...>
        class move_only_function_impl;

        template <typename Ret, typename ... Args>
        class move_only_function_impl<Ret(Args...)> : public move_only_function_base<Ret, false, Args...>
        {
        public:
            inline Ret operator()(Args... args)
            {
                return this->_invoker(this->get_fn_ptr(), ryujin::forward<Args>(args)...);
            }
        };

        template <typename Ret, typename ... Args>
        class move_only_function_impl<Ret(Args...) noexcept> : public move_only_function_base<Ret, true, Args...>
        {
        public:
            inline Ret operator()(Args... args) noexcept
            {
                return this->_invoker(this->get_fn_ptr(), ryujin::forward<Args>(args)...);
            }
        };

        template <typename Ret, typename ... Args>
        class move_only_function_impl<Ret(Args...) &> : public move_only_function_base<Ret, false, Args...>
        {
        public:
            inline Ret operator()(Args... args) &
            {
                return this->_invoker(this->get_fn_ptr(), ryujin::forward<Args>(args)...);
            }
        };

        template <typename Ret, typename ... Args>
        class move_only_function_impl<Ret(Args...) & noexcept> : public move_only_function_base<Ret, true, Args...>
        {
        public:
            inline Ret operator()(Args... args) & noexcept
            {
                return this->_invoker(this->get_fn_ptr(), ryujin::forward<Args>(args)...);
            }
        };

        template <typename Ret, typename ... Args>
        class move_only_function_impl<Ret(Args...) &&> : public move_only_function_base<Ret, false, Args...>
        {
        public:
            inline Ret operator()(Args... args) &&
            {
                return this->_invoker(this->get_fn_ptr(), ryujin::forward<Args>(args)...);
            }
        };

        template <typename Ret, typename ... Args>
        class move_only_function_impl<Ret(Args...) && noexcept> : public move_only_function_base<Ret, true, Args...>
        {
        public:
            inline Ret operator()(Args... args) && noexcept
            {
                return this->_invoker(this->get_fn_ptr(), ryujin::forward<Args>(args)...);
            }
        };

        template <typename Ret, typename ... Args>
        class move_only_function_impl<Ret(Args...) const> : public move_only_function_base<Ret, false, Args...>
        {
        public:
            inline Ret operator()(Args... args) const
            {
                return this->_invoker(this->get_fn_ptr(), ryujin::forward<Args>(args)...);
            }
        };

        template <typename Ret, typename ... Args>
        class move_only_function_impl<Ret(Args...) const noexcept> : public move_only_function_base<Ret, true, Args...>
        {
        public:
            inline Ret operator()(Args... args) const noexcept
            {
                return this->_invoker(this->get_fn_ptr(), ryujin::forward<Args>(args)...);
            }
        };

        template <typename Ret, typename ... Args>
        class move_only_function_impl<Ret(Args...) const &> : public move_only_function_base<Ret, false, Args...>
        {
        public:
            inline Ret operator()(Args... args) const &
            {
                return this->_invoker(this->get_fn_ptr(), ryujin::forward<Args>(args)...);
            }
        };

        template <typename Ret, typename ... Args>
        class move_only_function_impl<Ret(Args...) const & noexcept> : public move_only_function_base<Ret, true, Args...>
        {
        public:
            inline Ret operator()(Args... args) const & noexcept
            {
                return this->_invoker(this->get_fn_ptr(), ryujin::forward<Args>(args)...);
            }
        };

        template <typename Ret, typename ... Args>
        class move_only_function_impl<Ret(Args...) const &&> : public move_only_function_base<Ret, false, Args...>
        {
        public:
            inline Ret operator()(Args... args) const &&
            {
                return this->_invoker(this->get_fn_ptr(), ryujin::forward<Args>(args)...);
            }
        };

        template <typename Ret, typename ... Args>
        class move_only_function_impl<Ret(Args...) const && noexcept> : public move_only_function_base<Ret, true, Args...>
        {
        public:
            inline Ret operator()(Args... args) const && noexcept
            {
                return this->_invoker(this->get_fn_ptr(), ryujin::forward<Args>(args)...);
            }
        };
    }

    template <typename ...>
    class move_only_function;

    namespace detail
    {
        template <typename T>
        struct is_function_object : public false_type {};

        template <typename T>
        struct is_function_object<move_only_function<T>> : public true_type {};
    }

    template <typename Ret, typename ... Args>
    class move_only_function<Ret(Args...)> : public detail::move_only_function_impl<Ret(Args...)>
    {
    public:
        inline move_only_function()
        {
            this->construct_empty();
        }
                
        inline move_only_function(move_only_function&& fn) noexcept
        {
            this->construct_move(ryujin::forward<move_only_function>(fn));
        }
        
        inline move_only_function(nullptr_t) noexcept
            : move_only_function()
        {
        }

        template <typename Fn, enable_if_t<!is_same_v<remove_cvref_t<Fn>, move_only_function<Ret(Args...)>> && !detail::is_function_object<remove_cvref_t<Fn>>::value, bool> = true>
        inline move_only_function(Fn&& f)
        {
            this->construct_from_functor(ryujin::forward<Fn>(f));
        }

        inline move_only_function& operator=(move_only_function&& rhs) noexcept
        {
            this->move_assign(ryujin::forward<move_only_function&&>(rhs));
            return *this;
        }

        template <typename Fn, enable_if_t<!is_same_v<remove_cvref_t<Fn>, move_only_function<Ret(Args...)>> && !detail::is_function_object<remove_cvref_t<Fn>>::value, bool> = true>
        inline move_only_function& operator=(Fn&& f) noexcept
        {
            this->move_assign_from_functor(ryujin::forward<Fn>(f));
            return *this;
        }
    };

    template <typename Ret, typename ... Args>
    class move_only_function<Ret(Args...) noexcept> : public detail::move_only_function_impl<Ret(Args...) noexcept>
    {
    public:
        inline move_only_function()
        {
            this->construct_empty();
        }

        inline move_only_function(move_only_function&& fn) noexcept
        {
            this->construct_move(ryujin::forward<move_only_function>(fn));
        }

        inline move_only_function(nullptr_t) noexcept
            : move_only_function()
        {
        }

        template <typename Fn, enable_if_t<!is_same_v<remove_cvref_t<Fn>, move_only_function<Ret(Args...) noexcept>>, bool> = true>
        inline move_only_function(Fn&& f)
        {
            this->construct_from_functor(ryujin::forward<Fn>(f));
        }

        inline move_only_function& operator=(move_only_function&& rhs) noexcept
        {
            this->move_assign(ryujin::forward<move_only_function&&>(rhs));
            return *this;
        }

        template <typename Fn, enable_if_t<!is_same_v<remove_cvref_t<Fn>, move_only_function<Ret(Args...) noexcept>>, bool> = true>
        inline move_only_function& operator=(Fn&& f) noexcept
        {
            this->move_assign_from_functor(ryujin::forward<Fn>(f));
            return *this;
        }
    };

    template <typename Ret, typename ... Args>
    class move_only_function<Ret(Args...) &> : public detail::move_only_function_impl<Ret(Args...) &>
    {
    public:
        inline move_only_function()
        {
            this->construct_empty();
        }

        inline move_only_function(move_only_function&& fn) noexcept
        {
            this->construct_move(ryujin::forward<move_only_function>(fn));
        }

        inline move_only_function(nullptr_t) noexcept
            : move_only_function()
        {
        }

        template <typename Fn, enable_if_t<!is_same_v<remove_cvref_t<Fn>, move_only_function<Ret(Args...)>>, bool> = true>
        inline move_only_function(Fn&& f)
        {
            this->construct_from_functor(ryujin::forward<Fn>(f));
        }

        inline move_only_function& operator=(move_only_function&& rhs) noexcept
        {
            this->move_assign(ryujin::forward<move_only_function&&>(rhs));
            return *this;
        }

        template <typename Fn, enable_if_t<!is_same_v<remove_cvref_t<Fn>, move_only_function<Ret(Args...)>>, bool> = true>
        inline move_only_function& operator=(Fn&& f) noexcept
        {
            this->move_assign_from_functor(ryujin::forward<Fn>(f));
            return *this;
        }
    };

    template <typename Ret, typename ... Args>
    class move_only_function<Ret(Args...) & noexcept> : public detail::move_only_function_impl<Ret(Args...) & noexcept>
    {
    public:
        inline move_only_function()
        {
            this->construct_empty();
        }

        inline move_only_function(move_only_function&& fn) noexcept
        {
            this->construct_move(ryujin::forward<move_only_function>(fn));
        }

        inline move_only_function(nullptr_t) noexcept
            : move_only_function()
        {
        }

        template <typename Fn, enable_if_t<!is_same_v<remove_cvref_t<Fn>, move_only_function<Ret(Args...) noexcept>>, bool> = true>
        inline move_only_function(Fn&& f)
        {
            this->construct_from_functor(ryujin::forward<Fn>(f));
        }

        inline move_only_function& operator=(move_only_function&& rhs) noexcept
        {
            this->move_assign(ryujin::forward<move_only_function&&>(rhs));
            return *this;
        }

        template <typename Fn, enable_if_t<!is_same_v<remove_cvref_t<Fn>, move_only_function<Ret(Args...) noexcept>>, bool> = true>
        inline move_only_function& operator=(Fn&& f) noexcept
        {
            this->move_assign_from_functor(ryujin::forward<Fn>(f));
            return *this;
        }
    };

    template <typename Ret, typename ... Args>
    class move_only_function<Ret(Args...) &&> : public detail::move_only_function_impl<Ret(Args...) &&>
    {
    public:
        inline move_only_function()
        {
            this->construct_empty();
        }

        inline move_only_function(move_only_function&& fn) noexcept
        {
            this->construct_move(ryujin::forward<move_only_function>(fn));
        }

        inline move_only_function(nullptr_t) noexcept
            : move_only_function()
        {
        }

        template <typename Fn, enable_if_t<!is_same_v<remove_cvref_t<Fn>, move_only_function<Ret(Args...)>>, bool> = true>
        inline move_only_function(Fn&& f)
        {
            this->construct_from_functor(ryujin::forward<Fn>(f));
        }

        inline move_only_function& operator=(move_only_function&& rhs) noexcept
        {
            this->move_assign(ryujin::forward<move_only_function&&>(rhs));
            return *this;
        }

        template <typename Fn, enable_if_t<!is_same_v<remove_cvref_t<Fn>, move_only_function<Ret(Args...)>>, bool> = true>
        inline move_only_function& operator=(Fn&& f) noexcept
        {
            this->move_assign_from_functor(ryujin::forward<Fn>(f));
            return *this;
        }
    };

    template <typename Ret, typename ... Args>
    class move_only_function<Ret(Args...) && noexcept> : public detail::move_only_function_impl<Ret(Args...) && noexcept>
    {
    public:
        inline move_only_function()
        {
            this->construct_empty();
        }

        inline move_only_function(move_only_function&& fn) noexcept
        {
            this->construct_move(ryujin::forward<move_only_function>(fn));
        }

        inline move_only_function(nullptr_t) noexcept
            : move_only_function()
        {
        }

        template <typename Fn, enable_if_t<!is_same_v<remove_cvref_t<Fn>, move_only_function<Ret(Args...) noexcept>>, bool> = true>
        inline move_only_function(Fn&& f)
        {
            this->construct_from_functor(ryujin::forward<Fn>(f));
        }

        inline move_only_function& operator=(move_only_function&& rhs) noexcept
        {
            this->move_assign(ryujin::forward<move_only_function&&>(rhs));
            return *this;
        }

        template <typename Fn, enable_if_t<!is_same_v<remove_cvref_t<Fn>, move_only_function<Ret(Args...) noexcept>>, bool> = true>
        inline move_only_function& operator=(Fn&& f) noexcept
        {
            this->move_assign_from_functor(ryujin::forward<Fn>(f));
            return *this;
        }
    };

    template <typename Ret, typename ... Args>
    class move_only_function<Ret(Args...) const> : public detail::move_only_function_impl<Ret(Args...) const>
    {
    public:
        inline move_only_function()
        {
            this->construct_empty();
        }

        inline move_only_function(move_only_function&& fn) noexcept
        {
            this->construct_move(ryujin::forward<move_only_function>(fn));
        }

        inline move_only_function(nullptr_t) noexcept
            : move_only_function()
        {
        }

        template <typename Fn, enable_if_t<!is_same_v<remove_cvref_t<Fn>, move_only_function<Ret(Args...)>> && !detail::is_function_object<Fn>::value, bool> = true>
        inline move_only_function(Fn&& f)
        {
            this->construct_from_functor(ryujin::forward<Fn>(f));
        }

        inline move_only_function& operator=(move_only_function&& rhs) noexcept
        {
            this->move_assign(ryujin::forward<move_only_function&&>(rhs));
            return *this;
        }

        template <typename Fn, enable_if_t<!is_same_v<remove_cvref_t<Fn>, move_only_function<Ret(Args...)>> && !detail::is_function_object<Fn>::value, bool> = true>
        inline move_only_function& operator=(Fn&& f) noexcept
        {
            this->move_assign_from_functor(ryujin::forward<Fn>(f));
            return *this;
        }
    };

    template <typename Ret, typename ... Args>
    class move_only_function<Ret(Args...) const noexcept> : public detail::move_only_function_impl<Ret(Args...) const noexcept>
    {
    public:
        inline move_only_function()
        {
            this->construct_empty();
        }

        inline move_only_function(move_only_function&& fn) noexcept
        {
            this->construct_move(ryujin::forward<move_only_function>(fn));
        }

        inline move_only_function(nullptr_t) noexcept
            : move_only_function()
        {
        }

        template <typename Fn, enable_if_t<!is_same_v<remove_cvref_t<Fn>, move_only_function<Ret(Args...) noexcept>>, bool> = true>
        inline move_only_function(Fn&& f)
        {
            this->construct_from_functor(ryujin::forward<Fn>(f));
        }

        inline move_only_function& operator=(move_only_function&& rhs) noexcept
        {
            this->move_assign(ryujin::forward<move_only_function&&>(rhs));
            return *this;
        }

        template <typename Fn, enable_if_t<!is_same_v<remove_cvref_t<Fn>, move_only_function<Ret(Args...) noexcept>>, bool> = true>
        inline move_only_function& operator=(Fn&& f) noexcept
        {
            this->move_assign_from_functor(ryujin::forward<Fn>(f));
            return *this;
        }
    };

    template <typename Ret, typename ... Args>
    class move_only_function<Ret(Args...) const &> : public detail::move_only_function_impl<Ret(Args...) const &>
    {
    public:
        inline move_only_function()
        {
            this->construct_empty();
        }

        inline move_only_function(move_only_function&& fn) noexcept
        {
            this->construct_move(ryujin::forward<move_only_function>(fn));
        }

        inline move_only_function(nullptr_t) noexcept
            : move_only_function()
        {
        }

        template <typename Fn, enable_if_t<!is_same_v<remove_cvref_t<Fn>, move_only_function<Ret(Args...)>>, bool> = true>
        inline move_only_function(Fn&& f)
        {
            this->construct_from_functor(ryujin::forward<Fn>(f));
        }

        inline move_only_function& operator=(move_only_function&& rhs) noexcept
        {
            this->move_assign(ryujin::forward<move_only_function&&>(rhs));
            return *this;
        }

        template <typename Fn, enable_if_t<!is_same_v<remove_cvref_t<Fn>, move_only_function<Ret(Args...)>>, bool> = true>
        inline move_only_function& operator=(Fn&& f) noexcept
        {
            this->move_assign_from_functor(ryujin::forward<Fn>(f));
            return *this;
        }
    };

    template <typename Ret, typename ... Args>
    class move_only_function<Ret(Args...) const & noexcept> : public detail::move_only_function_impl<Ret(Args...) const & noexcept>
    {
    public:
        inline move_only_function()
        {
            this->construct_empty();
        }

        inline move_only_function(move_only_function&& fn) noexcept
        {
            this->construct_move(ryujin::forward<move_only_function>(fn));
        }

        inline move_only_function(nullptr_t) noexcept
            : move_only_function()
        {
        }

        template <typename Fn, enable_if_t<!is_same_v<remove_cvref_t<Fn>, move_only_function<Ret(Args...) noexcept>>, bool> = true>
        inline move_only_function(Fn&& f)
        {
            this->construct_from_functor(ryujin::forward<Fn>(f));
        }

        inline move_only_function& operator=(move_only_function&& rhs) noexcept
        {
            this->move_assign(ryujin::forward<move_only_function&&>(rhs));
            return *this;
        }

        template <typename Fn, enable_if_t<!is_same_v<remove_cvref_t<Fn>, move_only_function<Ret(Args...) noexcept>>, bool> = true>
        inline move_only_function& operator=(Fn&& f) noexcept
        {
            this->move_assign_from_functor(ryujin::forward<Fn>(f));
            return *this;
        }
    };

    template <typename Ret, typename ... Args>
    class move_only_function<Ret(Args...) const &&> : public detail::move_only_function_impl<Ret(Args...) const &&>
    {
    public:
        inline move_only_function()
        {
            this->construct_empty();
        }

        inline move_only_function(move_only_function&& fn) noexcept
        {
            this->construct_move(ryujin::forward<move_only_function>(fn));
        }

        inline move_only_function(nullptr_t) noexcept
            : move_only_function()
        {
        }

        template <typename Fn, enable_if_t<!is_same_v<remove_cvref_t<Fn>, move_only_function<Ret(Args...)>>, bool> = true>
        inline move_only_function(Fn&& f)
        {
            this->construct_from_functor(ryujin::forward<Fn>(f));
        }

        inline move_only_function& operator=(move_only_function&& rhs) noexcept
        {
            this->move_assign(ryujin::forward<move_only_function&&>(rhs));
            return *this;
        }

        template <typename Fn, enable_if_t<!is_same_v<remove_cvref_t<Fn>, move_only_function<Ret(Args...)>>, bool> = true>
        inline move_only_function& operator=(Fn&& f) noexcept
        {
            this->move_assign_from_functor(ryujin::forward<Fn>(f));
            return *this;
        }
    };

    template <typename Ret, typename ... Args>
    class move_only_function<Ret(Args...) const && noexcept> : public detail::move_only_function_impl<Ret(Args...) const && noexcept>
    {
    public:
        inline move_only_function()
        {
            this->construct_empty();
        }

        inline move_only_function(move_only_function&& fn) noexcept
        {
            this->construct_move(ryujin::forward<move_only_function>(fn));
        }

        inline move_only_function(nullptr_t) noexcept
            : move_only_function()
        {
        }

        template <typename Fn, enable_if_t<!is_same_v<remove_cvref_t<Fn>, move_only_function<Ret(Args...) noexcept>>, bool> = true>
        inline move_only_function(Fn&& f)
        {
            this->construct_from_functor(ryujin::forward<Fn>(f));
        }

        inline move_only_function& operator=(move_only_function&& rhs) noexcept
        {
            this->move_assign(ryujin::forward<move_only_function&&>(rhs));
            return *this;
        }

        template <typename Fn, enable_if_t<!is_same_v<remove_cvref_t<Fn>, move_only_function<Ret(Args...) noexcept>>, bool> = true>
        inline move_only_function& operator=(Fn&& f) noexcept
        {
            this->move_assign_from_functor(ryujin::forward<Fn>(f));
            return *this;
        }
    };

    namespace detail
    {
        template <typename>
        struct function_signature_extractor;

        template <typename Res, typename Tp, typename ... Args>
        struct function_signature_extractor<Res(Tp::*)(Args...)>
        {
            using type = Res(Args...);
        };

        template <typename Res, typename Tp, typename ... Args>
        struct function_signature_extractor<Res(Tp::*)(Args...) noexcept>
        {
            using type = Res(Args...) noexcept;
        };

        template <typename Res, typename Tp, typename ... Args>
        struct function_signature_extractor<Res(Tp::*)(Args...) &>
        {
            using type = Res(Args...) &;
        };

        template <typename Res, typename Tp, typename ... Args>
        struct function_signature_extractor<Res(Tp::*)(Args...) & noexcept>
        {
            using type = Res(Args...) & noexcept;
        };

        template <typename Res, typename Tp, typename ... Args>
        struct function_signature_extractor<Res(Tp::*)(Args...) &&>
        {
            using type = Res(Args...) &&;
        };

        template <typename Res, typename Tp, typename ... Args>
        struct function_signature_extractor<Res(Tp::*)(Args...) && noexcept>
        {
            using type = Res(Args...) && noexcept;
        };

        template <typename Res, typename Tp, typename ... Args>
        struct function_signature_extractor<Res(Tp::*)(Args...) const>
        {
            using type = Res(Args...) const;
        };

        template <typename Res, typename Tp, typename ... Args>
        struct function_signature_extractor<Res(Tp::*)(Args...) const noexcept>
        {
            using type = Res(Args...) const noexcept;
        };

        template <typename Res, typename Tp, typename ... Args>
        struct function_signature_extractor<Res(Tp::*)(Args...) const &>
        {
            using type = Res(Args...) const &;
        };

        template <typename Res, typename Tp, typename ... Args>
        struct function_signature_extractor<Res(Tp::*)(Args...) const & noexcept>
        {
            using type = Res(Args...) const & noexcept;
        };

        template <typename Res, typename Tp, typename ... Args>
        struct function_signature_extractor<Res(Tp::*)(Args...) const &&>
        {
            using type = Res(Args...) const &&;
        };

        template <typename Res, typename Tp, typename ... Args>
        struct function_signature_extractor<Res(Tp::*)(Args...) const && noexcept>
        {
            using type = Res(Args...) const && noexcept;
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
}

#endif // functional_hpp__
