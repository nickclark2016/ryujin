#ifndef utility_hpp__
#define utility_hpp__

#include "primitives.hpp"
#include "type_traits.hpp"

#include <type_traits>

namespace ryujin
{
    /// <summary>
    /// Forwards an l-value reference as an r-value.
    /// </summary>
    /// <typeparam name="T">Type of value to forward</typeparam>
    /// <param name="t">Value to forward</param>
    /// <returns>r-value reference</returns>
    template <typename T>
    constexpr T&& forward(std::remove_reference_t<T>& t) noexcept
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
    inline constexpr T&& forward(std::remove_reference_t<T>&& t) noexcept
    {
        static_assert(!std::is_lvalue_reference_v<T>);
        return static_cast<T&&>(t);
    }

    /// <summary>
    /// Moves a r-value reference.
    /// </summary>
    /// <typeparam name="T">Type fo the value to move</typeparam>
    /// <param name="t">Value to move</param>
    /// <returns>Moved reference</returns>
    template <typename T>
    inline constexpr std::remove_reference_t<T>&& move(T&& t) noexcept
    {
        return static_cast<std::remove_reference_t<T>&&>(t);
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
        /// Default constructs the first and second element
        /// </summary>
        constexpr pair() = default;

        /// <summary>
        /// 
        /// </summary>
        /// <param name="first"></param>
        /// <param name="second"></param>
        constexpr pair(const T1& first, const T2& second);
        constexpr pair(T1&& first, T2&& second);

        pair(const pair&) = default;
        pair(pair&&) noexcept = default;

        constexpr pair& operator=(const pair& rhs);
        constexpr pair& operator=(pair&& rhs) noexcept;

        void swap(pair& p);
    };

    template <typename T1, typename T2>
    pair(T1, T2) -> pair<T1, T2>;

    template <typename T1, typename T2>
    inline constexpr pair<T1, T2>::pair(const T1& first, const T2& second)
        : first(first), second(second)
    {
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
        pair<T1, T2> p = {
            .first = first,
            .second = second
        };
        return p;
    }
}

#endif // utility_hpp__
