#ifndef span_hpp__
#define span_hpp__

#include "as.hpp"
#include "optional.hpp"
#include "primitives.hpp"

#include <cstddef>

namespace ryujin
{
    template <typename T>
    class span
    {
    public:
        constexpr span();
        constexpr span(const span& other);
        constexpr span(span&& other) noexcept;

        constexpr span(const T& value);
        constexpr span(const T* begin, const T* end);
        constexpr span(const T* begin, const sz length);
        constexpr span(const optional<T>& opt);

        template <sz N>
        constexpr span(const T(&d)[N]);

        span& operator=(const span& rhs);
        span& operator=(span&& rhs) noexcept;

        constexpr const T* data() const noexcept;
        constexpr sz length() const noexcept;

        constexpr const T& operator[](const sz idx) const noexcept;
    private:

        const T* _ptr;
        sz _length;
    };

    template <typename T>
    span(const optional<T>&)->span<T>;

    template <typename T>
    span(const T*, const T*) -> span<T>;

    template <typename T>
    span(const T*, const sz) -> span<T>;

    template <typename T, sz N>
    span(const T[N]) -> span<T>;
    
    template<typename T>
    inline constexpr span<T>::span()
        : span<T>((T*)nullptr, as<sz>(0))
    {
    }

    template<typename T>
    inline constexpr span<T>::span(const span& span)
        : span<T>(span._ptr, span._length)
    {
    }

    template<typename T>
    inline constexpr span<T>::span(span&& span) noexcept
        : span<T>(span._ptr, span._length)
    {
        span._ptr = nullptr;
        span._length = 0;
    }

    template<typename T>
    inline constexpr span<T>::span(const T& value)
        : _ptr(&value), _length(as<sz>(1))
    {
    }

    template<typename T>
    inline constexpr span<T>::span(const T* begin, const T* end)
        : _ptr(begin), _length(end - begin)
    {
    }
    
    template<typename T>
    inline constexpr span<T>::span(const T* begin, const sz length)
        : _ptr(begin), _length(length)
    {
    }

    template<typename T>
    inline constexpr span<T>::span(const optional<T>& opt)
        : _ptr(opt.has_value() ? &opt.value() : nullptr), _length(opt.has_value() ? 1 : 0)
    {
    }
    
    template<typename T>
    inline span<T>& span<T>::operator=(const span& rhs)
    {
        _ptr = rhs._ptr;
        _length = rhs._length;
        return *this;
    }

    template<typename T>
    inline span<T>& span<T>::operator=(span&& rhs) noexcept
    {
        _ptr = rhs._ptr;
        _length = rhs._length;
        rhs._ptr = nullptr;
        rhs._length = 0;
        return *this;
    }

    template<typename T>
    inline constexpr const T* span<T>::data() const noexcept
    {
        return _ptr;
    }
    
    template<typename T>
    inline constexpr sz span<T>::length() const noexcept
    {
        return _length;
    }

    template<typename T>
    inline constexpr const T& span<T>::operator[](const sz idx) const noexcept
    {
        return _ptr[idx];
    }
    
    template<typename T>
    template<sz N>
    inline constexpr span<T>::span(const T(&d)[N])
        : _ptr(d), _length(N)
    {}
}

#endif // span_hpp__
