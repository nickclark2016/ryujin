#ifndef utility_hpp__
#define utility_hpp__

#include "as.hpp"
#include "primitives.hpp"
#include "type_traits.hpp"

namespace ryujin
{
    /// \defgroup reference_wrapper Reference Wrapper

    /// <summary>
    /// Wraps a reference into a copyable and assignable type.  This can be used to store
    /// references in types that cannot typically hold references.
    /// </summary>
    /// <typeparam name="T">Type of reference</typeparam>
    /// \ingroup reference_wrapper
    template <typename T>
    class reference_wrapper
    {
    public:
        /// <summary>
        /// Constructs a reference wrapper from a value.
        /// </summary>
        /// <typeparam name="U">Type of the value to construct from</typeparam>
        /// <param name="val">Value to construct from</param>
        template <typename U>
        constexpr reference_wrapper(U&& val);

        /// <summary>
        /// Copy constructor.
        /// </summary>
        /// <param name="other">Reference wrapper to copy reference from</param>
        /// <returns></returns>
        constexpr reference_wrapper(const reference_wrapper& other) noexcept;

        /// <summary>
        /// Reseats the reference to point to the reference held by the other reference.
        /// </summary>
        /// <param name="other">Reference wrapper to copy from</param>
        /// <returns>Reference to this</returns>
        constexpr reference_wrapper& operator=(const reference_wrapper& other) noexcept;

        /// <summary>
        /// Gets the value held by the wrapper.
        /// </summary>
        /// <returns>Reference held by wrapper</returns>
        constexpr operator T& () const noexcept;

        /// <summary>
        /// Gets the value held by the wrapper.
        /// </summary>
        /// <returns>Reference held by wrapper</returns>
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

    /// <summary>
    /// Creates a reference wrapper from a reference.
    /// </summary>
    /// <typeparam name="T">Type of the reference</typeparam>
    /// <param name="t">Reference to wrap</param>
    /// <returns>Reference wrapper wrapping the provided reference</returns>
    /// \ingroup reference_wrapper
    template <typename T>
    inline constexpr reference_wrapper<T> ref(T& t) noexcept
    {
        return reference_wrapper<T>(t);
    }

    template <typename T>
    void ref(const T&&) = delete;

    /// <summary>
    /// Creates a const reference wrapper from a reference.
    /// </summary>
    /// <typeparam name="T">Type of the reference</typeparam>
    /// <param name="t">Reference to wrap</param>
    /// <returns>Reference wrapper wrapping the provided reference as a constant reference</returns>
    /// \ingroup reference_wrapper
    template <typename T>
    inline constexpr reference_wrapper<const T> cref(const T& t) noexcept
    {
        return reference_wrapper<const T>(t);
    }

    template <typename T>
    void cref(const T&&) = delete;

    /// <summary>
    /// Forwards an l-value reference as an r-value.
    /// </summary>
    /// <typeparam name="T">Type of value to forward</typeparam>
    /// <param name="t">Value to forward</param>
    /// <returns>r-value reference</returns>
    template <typename T>
    constexpr T&& forward(remove_reference_t<T>& t) noexcept
    {
        return static_cast<T&&>(t);
    }

    /// <summary>
    /// Forwards an r-value reference as an r-value.
    /// </summary>
    /// <typeparam name="T">Type of value to forward</typeparam>
    /// <param name="t">Value to forward</param>
    /// <returns>r-value reference</returns>
    template <typename T>
    inline constexpr T&& forward(remove_reference_t<T>&& t) noexcept
    {
        static_assert(!is_lvalue_reference_v<T>);
        return static_cast<T&&>(t);
    }

    /// <summary>
    /// Moves a r-value reference.
    /// </summary>
    /// <typeparam name="T">Type fo the value to move</typeparam>
    /// <param name="t">Value to move</param>
    /// <returns>Moved reference</returns>
    template <typename T>
    inline constexpr remove_reference_t<T>&& move(T&& t) noexcept
    {
        return static_cast<remove_reference_t<T>&&>(t);
    }

    /// <summary>
    /// Type used to tag for in-place construction.
    /// </summary>
    struct in_place_t
    {
        /// <summary>
        /// Default constructor.
        /// </summary>
        explicit in_place_t() = default;
    };

    /// <summary>
    /// Default in place tag.
    /// </summary>
    inline constexpr in_place_t in_place{};

    /// <summary>
    /// Type used to tag for in-place construction of a given type.
    /// </summary>
    /// <typeparam name="T">Type to construct in place</typeparam>
    template <typename T>
    struct in_place_type_t
    {
        /// <summary>
        /// Default constructor.
        /// </summary>
        explicit in_place_type_t() = default;
    };

    /// <summary>
    /// Default in place type tag.
    /// </summary>
    template <typename T>
    inline constexpr in_place_type_t<T> in_place_type{};

    /// <summary>
    /// Type used to tag for in-place construction of the type at the given index of a parameter pack.
    /// </summary>
    /// <typeparam name="I">
    /// Index in parameter pack to construct type of
    /// </typeparam>
    template <sz I>
    struct in_place_index_t
    {
        /// <summary>
        /// Default constructor.
        /// </summary>
        explicit in_place_index_t() = default;
    };

    /// <summary>
    /// Default in place index tag.
    /// </summary>
    template <sz I>
    inline constexpr in_place_index_t<I> in_place_index{};

    /// <summary>
    /// Implementation of move using swap semantics.
    /// </summary>
    /// <typeparam name="T">Type of values to swap</typeparam>
    /// <param name="a">Value to swap</param>
    /// <param name="b">Value to swap</param>
    template <typename T>
    constexpr void move_swap(T& a, T& b)
    {
        auto tmp = ryujin::move(a);
        a = ryujin::move(b);
        b = ryujin::move(tmp);
    }

    /// <summary>
    /// Implementation of move using swap semantics for an array type.
    /// </summary>
    /// <typeparam name="T">Type of values to swap</typeparam>
    /// <typeparam name="N">Number of elements in the array</typeparam>
    /// <param name="a">Value to swap</param>
    /// <param name="b">Value to swap</param>
    template <typename T, sz N>
    constexpr void move_swap(T(&a)[N], T(&b)[N])
    {
        for (sz i = 0; i < N; ++i)
        {
            move_swap(a[i], b[i]);
        }
    }

    /// \defgroup hashes Hashing Functors

    /// <summary>
    /// Functor type that hashes a value and returns the hash.  Specializations should implement an overload of
    /// operator() that takes a single element of the key type and returns a sz variable representing the hash.  If
    /// two objects are equal, their hash value must be equal.  If two objects are not equal, the chance that their
    /// hashes are equivalent should be minimal (approaching 0).
    /// </summary>
    /// <typeparam name="K">Type of key to hash</typeparam>
    /// \ingroup hashes
    template <typename K>
    struct hash;

    /// <summary>
    /// Specialization of hash for a bool.
    /// </summary>
    /// \ingroup hashes
    template<>
    struct hash<bool>
    {
        inline constexpr sz operator()(const bool b) const noexcept
        {
            return b == true ? 1 : 0;
        }
    };

    /// <summary>
    /// Specialization of hash for an 8 bit integer.
    /// </summary>
    /// \ingroup hashes
    template<>
    struct hash<i8>
    {
        inline constexpr sz operator()(const i8 v) const noexcept
        {
            return v;
        }
    };

    /// <summary>
    /// Specialization of hash for an 8 bit unsigned integer.
    /// </summary>
    /// \ingroup hashes
    template<>
    struct hash<u8>
    {
        inline constexpr sz operator()(const u8 v) const noexcept
        {
            return v;
        }
    };

    /// <summary>
    /// Specialization of hash for an 16 bit integer.
    /// </summary>
    /// \ingroup hashes
    template<>
    struct hash<i16>
    {
        inline constexpr sz operator()(const i16 v) const noexcept
        {
            return v;
        }
    };

    /// <summary>
    /// Specialization of hash for an 16 bit unsigned integer.
    /// </summary>
    /// \ingroup hashes
    template<>
    struct hash<u16>
    {
        inline constexpr sz operator()(const u16 v) const noexcept
        {
            return v;
        }
    };

    /// <summary>
    /// Specialization of hash for an 32 bit integer.
    /// </summary>
    /// \ingroup hashes
    template<>
    struct hash<i32>
    {
        inline constexpr sz operator()(const i32 v) const noexcept
        {
            return v;
        }
    };

    /// <summary>
    /// Specialization of hash for an 32 bit unsigned integer.
    /// </summary>
    /// \ingroup hashes
    template<>
    struct hash<u32>
    {
        inline constexpr sz operator()(const u32 v) const noexcept
        {
            return v;
        }
    };

    /// <summary>
    /// Specialization of hash for an 64 bit integer.
    /// </summary>
    /// \ingroup hashes
    template<>
    struct hash<i64>
    {
        inline constexpr sz operator()(const i64 v) const noexcept
        {
            return v;
        }
    };

    /// <summary>
    /// Specialization of hash for an 64 bit unsigned integer.
    /// </summary>
    /// \ingroup hashes
    template<>
    struct hash<u64>
    {
        inline constexpr sz operator()(const u64 v) const noexcept
        {
            return v;
        }
    };

    /// <summary>
    /// Specialization of hash for an 64 bit unsigned integer.
    /// </summary>
    /// \ingroup hashes
    template<>
    struct hash<unsigned long int>
    {
        inline constexpr sz operator()(const unsigned long int v) const noexcept
        {
            return v;
        }
    };

    /// <summary>
    /// Specialization of hash for a single precision float.
    /// </summary>
    /// \ingroup hashes
    template <>
    struct hash<f32>
    {
        inline sz operator()(const f32 v) const noexcept
        {
            const u32 bytes = *reinterpret_cast<const u32*>(&v);
            return hash<u32>()(bytes);
        }
    };

    /// <summary>
    /// Specialization of hash for a double precision float.
    /// </summary>
    /// \ingroup hashes
    template <>
    struct hash<f64>
    {
        inline sz operator()(const f64 v) const noexcept
        {
            const u64 bytes = *reinterpret_cast<const u64*>(&v);
            return hash<u64>()(bytes);
        }
    };

    /// <summary>
    /// Specialization of hash for a pointer.
    /// </summary>
    /// \ingroup hashes
    template <typename T>
    struct hash<T*>
    {
        inline constexpr sz operator()(const T* v) const noexcept
        {
            const sz s = static_cast<sz>(v);
            return hash<sz>()(s);
        }
    };

    /// <summary>
    /// Tuple containing two values.
    /// </summary>
    /// <typeparam name="T1">First type contained</typeparam>
    /// <typeparam name="T2">Second type contained</typeparam>
    template <typename T1, typename T2>
    struct pair
    {
        /// <summary>
        /// First element in the pair
        /// </summary>
        T1 first;

        /// <summary>
        /// Second element in the pair.
        /// </summary>
        T2 second;

        /// <summary>
        /// Default constructs the first and second element.
        /// </summary>
        constexpr pair() = default;

        /// <summary>
        /// Copy constructs the first and second elements.
        /// </summary>
        /// <param name="first">First element</param>
        /// <param name="second">Second element</param>
        constexpr pair(const T1& first, const T2& second);

        /// <summary>
        /// Move constructs the first and second elements.
        /// </summary>
        /// <typeparam name="U1">Type of first element</typeparam>
        /// <typeparam name="U2">Type of second element</typeparam>
        /// <param name="first">First element</param>
        /// <param name="second">Second element</param>
        template <typename U1, typename U2>
        constexpr pair(U1&& first, U2&& second);

        /// <summary>
        /// Default copy constructor
        /// </summary>
        /// <param name="">Pair to copy from</param>
        pair(const pair&) = default;

        /// <summary>
        /// Default move constructor
        /// </summary>
        /// <param name="">Pair to move from</param>
        pair(pair&&) noexcept = default;

        template <typename U1, typename U2>
        constexpr pair(const pair<U1, U2>& p);

        /// <summary>
        /// Copies the right hand argument into this pair.
        /// </summary>
        /// <param name="rhs">Argument to copy from</param>
        /// <returns>Reference to this</returns>
        constexpr pair& operator=(const pair& rhs);

        /// <summary>
        /// Moves the right hand argument into this pair.
        /// </summary>
        /// <param name="rhs">Argument to move from</param>
        /// <returns>Reference to this</returns>
        constexpr pair& operator=(pair&& rhs) noexcept;

        /// <summary>
        /// Swaps the contains of this pair and the other pair.
        /// </summary>
        /// <param name="p"></param>
        void swap(pair& p);
    };

    template <typename T1, typename T2>
    pair(T1, T2)->pair<T1, T2>;

    template <typename T1, typename T2>
    inline constexpr pair<T1, T2>::pair(const T1& first, const T2& second)
        : first(first), second(second)
    {
    }

    template<typename T1, typename T2>
    template<typename U1, typename U2>
    inline constexpr pair<T1, T2>::pair(U1&& first, U2&& second)
        : first(ryujin::forward<U1>(first)), second(ryujin::forward<U2>(second))
    {
    }

    template <typename T1, typename T2>
    template <typename U1, typename U2>
    inline constexpr pair<T1, T2>::pair(const pair<U1, U2>& p)
        : first(p.first), second(p.second)
    {
    }

    template <typename T1, typename T2>
    inline constexpr pair<T1, T2>& pair<T1, T2>::operator=(const pair& rhs)
    {
        if (&rhs == this)
        {
            return *this;
        }

        first = rhs.first;
        second = rhs.second;

        return *this;
    }

    template <typename T1, typename T2>
    inline constexpr pair<T1, T2>& pair<T1, T2>::operator=(pair&& rhs) noexcept
    {
        if (&rhs == this)
        {
            return *this;
        }

        first = ryujin::move(rhs.first);
        second = ryujin::move(rhs.second);

        return *this;
    }

    /// <summary>
    /// Compares the pairs by the first value first, then the second value.
    /// </summary>
    /// <typeparam name="T1"></typeparam>
    /// <typeparam name="T2"></typeparam>
    /// <param name="lhs"></param>
    /// <param name="rhs"></param>
    /// <returns></returns>
    template <typename T1, typename T2>
    inline constexpr auto operator<=>(const pair<T1, T2>& lhs, const pair<T1, T2>& rhs) noexcept
    {
        const auto fe = lhs.first <=> rhs.first;
        if (fe == 0)
        {
            return lhs.second <=> rhs.second;
        }
        return fe;
    }

    /// <summary>
    /// Constructs a pair from two r-values.
    /// </summary>
    /// <typeparam name="T1">Type of first value in pair</typeparam>
    /// <typeparam name="T2">Type of second value in pair</typeparam>
    /// <param name="first">First value in pair</param>
    /// <param name="second">Second value in pair</param>
    /// <returns>Pair containing the provided values</returns>
    template <typename T1, typename T2>
    inline constexpr pair<T1, T2> make_pair(T1&& first, T2&& second)
    {
        pair<T1, T2> p(ryujin::forward<T1>(first), ryujin::forward<T2>(second));
        return p;
    }

    template <sz N, typename T1, typename T2>
    inline constexpr const auto& get(const pair<T1, T2>& p)
    {
        if constexpr (N == 0)
        {
            return p.first;
        }
        else if constexpr (N == 1)
        {
            return p.second;
        }
    }

    template <sz N, typename T1, typename T2>
    inline constexpr auto& get(pair<T1, T2>& p)
    {
        if constexpr (N == 0)
        {
            return p.first;
        }
        else if constexpr (N == 1)
        {
            return p.second;
        }
    }

    namespace detail
    {
        template <typename ... Ts>
        struct tuple_impl;

        template <>
        struct tuple_impl<>
        {};

        template <typename Head, typename ... Rest>
        struct tuple_impl<Head, Rest...> : public tuple_impl<Rest...>
        {
            using type = Head;
            using base = tuple_impl<Rest...>;

            type value;

            inline constexpr tuple_impl()
                : base(), value()
            {
            }

            inline constexpr tuple_impl(const Head& value, const Rest& ... rest)
                : base(ryujin::forward<Rest>(rest)...), value(value)
            {
            }

            template <typename H, typename ... R>
            inline constexpr tuple_impl(H&& value, R&& ... rest)
                : base(ryujin::forward<R>(rest)...), value(ryujin::forward<H>(value))
            {
            }

            inline constexpr base& get_rest() noexcept
            {
                return *this;
            }

            inline constexpr const base& get_rest() const noexcept
            {
                return *this;
            }
        };

        template <sz I, typename ... Ts>
        auto& get(tuple_impl<Ts...>& tup) noexcept
        {
            static_assert(I < sizeof...(Ts), "Invalid size index");
            if constexpr (I == 0)
            {
                return tup.value;
            }
            else
            {
                return get<I - 1>(tup.get_rest());
            }
        };

        template <sz I, typename ... Ts>
        const auto& get(const tuple_impl<Ts...>& tup) noexcept
        {
            static_assert(I < sizeof...(Ts), "Invalid size index");
            if constexpr (I == 0)
            {
                return tup.value;
            }
            else
            {
                return get<I - 1>(tup.get_rest());
            }
        };

        template <typename ... Ts>
        void swap(tuple_impl<Ts...>& lhs, tuple_impl<Ts...>& rhs)
        {
            return move_swap(lhs, rhs);
        }
    }

    template <typename ... Ts>
    class tuple;

    namespace detail
    {
        template <typename T>
        struct unwrap_reference_wrapper
        {
            using type = T;
        };

        template <typename T>
        struct unwrap_reference_wrapper<reference_wrapper<T>>
        {
            using type = T;
        };

        template <typename T>
        using unwrapped_decay_t = typename unwrap_reference_wrapper<decay_t<T>>::type;
    }

    template <typename ... Ts>
    inline constexpr tuple<Ts...> make_tuple(Ts&& ... ts)
    {
        return tuple<detail::unwrapped_decay_t<Ts>...>(ryujin::forward<Ts>(ts)...);
    }

    template <sz I, typename ... Ts>
    struct tuple_element;

    template <sz I, typename Head, typename ... Ts>
    struct tuple_element<I, tuple<Head, Ts...>> : tuple_element<I - 1, tuple<Ts...>>
    {
    };

    template <typename Head, typename ... Ts>
    struct tuple_element<0, tuple<Head, Ts...>>
    {
        using type = Head;
    };

    template <typename ... Ts>
    struct tuple_size;

    template <typename ... Ts>
    struct tuple_size<tuple<Ts...>> : ryujin::integral_constant<sz, sizeof...(Ts)>
    {
    };

    template <typename ... Ts>
    class tuple : detail::tuple_impl<Ts...>
    {
    public:
        constexpr tuple() = default;

        inline constexpr tuple(const Ts& ... args)
            : detail::tuple_impl<Ts...>(ryujin::forward<Ts>(args)...)
        {};

        template <typename ... Us>
        constexpr tuple(Us&& ... us)
            : detail::tuple_impl<Ts...>(ryujin::forward<Us>(us)...)
        {};

        tuple(const tuple&) = default;
        tuple(tuple&&) noexcept = default;

        ~tuple() = default;

        tuple& operator=(const tuple&) = default;
        tuple& operator=(tuple&&) noexcept = default;

        inline constexpr void swap(tuple<Ts...>& rhs) noexcept
        {
            detail::swap(*this, rhs);
        }

        template <sz I>
        inline constexpr typename tuple_element<I, tuple<Ts...>>::type& get()
        {
            return detail::get<I>(*this);
        }

        template <sz I>
        inline constexpr const typename tuple_element<I, tuple<Ts...>>::type& get() const
        {
            return detail::get<I>(*this);
        }

    private:
        template <sz I, typename ... Types>
        friend auto& get(tuple<Types...>& t) noexcept;

        template <sz I, typename ... Types>
        friend const auto& get(const tuple<Types...>& t) noexcept;
    };

    template <typename ... Ts>
    tuple(Ts...)->tuple<Ts...>;

    template<sz I, typename ... Ts>
    auto& get(tuple<Ts...>& t) noexcept
    {
        return detail::get<I, Ts...>(t);
    }

    template<sz I, typename ... Ts>
    const auto& get(const tuple<Ts...>& t) noexcept
    {
        return detail::get<I, Ts...>(t);
    }

    template <typename T, T ... Integers>
    struct integer_sequence
    {
        static constexpr sz size() noexcept;
    };

    template <typename T, T ... Integers>
    inline constexpr sz integer_sequence<T, Integers...>::size() noexcept
    {
        return sizeof...(Integers);
    }

    namespace detail
    {
        template <typename T, sz N, T ... Integers>
        struct gen_seq : gen_seq<T, N - 1, T(N - 1), Integers...> {};

        template <typename T, T... Integers>
        struct gen_seq<T, 0, Integers...>
        {
            using type = integer_sequence<T, Integers...>;
        };
    }

    template <sz ... Integers>
    using index_sequence = integer_sequence<sz, Integers...>;

    template <typename T, T N>
    using make_integer_sequence = typename detail::gen_seq<T, T(N)>::type;

    template <sz N>
    using make_index_sequence = make_integer_sequence<sz, N>;

    template <typename ... Ts>
    using index_sequence_for = make_index_sequence<sizeof...(Ts)>;

    inline constexpr void* memcpy(void* dst, const void* src, sz len)
    {
        char* d = as<char*>(dst);
        const char* s = as<const char*>(src);
        while (len--)
        {
            *d++ = *s++;
        }
        return dst;
    }

    inline constexpr void* memset(void* dst, int val, sz len)
    {
        char* d = as<char*>(dst);
        while (len--)
        {
            *d++ = val;
        }
        return dst;
    }
}

#ifdef RYUJIN_PROVIDE_STRUCTURED_BINDINGS
#include <tuple>

namespace std
{
    template <typename T1, typename T2>
    class tuple_size<ryujin::pair<T1, T2>> : public ryujin::integral_constant<ryujin::sz, 2>
    {};

    template <ryujin::sz I, typename T1, typename T2>
    class tuple_element<I, ryujin::pair<T1, T2>> : public ryujin::tuple_element<I, ryujin::tuple<T1, T2>>
    {};

    template <typename ... Ts>
    class tuple_size<ryujin::tuple<Ts...>> : public ryujin::integral_constant<ryujin::sz, sizeof...(Ts)>
    {};

    template <ryujin::sz I, typename ... Ts>
    class tuple_element<I, ryujin::tuple<Ts...>> : public ryujin::tuple_element<I, ryujin::tuple<Ts...>>
    {};
}
#endif

#endif // utility_hpp__
