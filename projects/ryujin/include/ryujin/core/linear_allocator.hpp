#ifndef linear_allocator_hpp__
#define linear_allocator_hpp__

#include <cstddef>
#include <memory>
#include <new>

namespace ryujin
{
    class linear_allocator
    {
    public:
        linear_allocator(const std::size_t bytes);

        void* allocate(const std::size_t bytes);
        void deallocate(void* ptr);
        void* reallocate(void* ptr, const std::size_t bytes);
        void reset();

        template <typename T>
        T* typed_allocate(const std::size_t count = 1);

        template <typename T>
        T* typed_reallocate(T* ptr, const std::size_t count = 1);

        std::size_t usage() const noexcept;
        std::size_t capacity() const noexcept;

    private:
        std::unique_ptr<unsigned char[]> _data;
        std::size_t _offset;
        std::size_t _lastOffset;
        const std::size_t _length;
    };

    template <std::size_t N>
    class inline_linear_allocator
    {
    public:
        void* allocate(const std::size_t bytes);
        void deallocate(void* ptr);
        void* reallocate(void* ptr, const std::size_t bytes);
        void reset();

        template <typename T>
        T* typed_allocate(const std::size_t count = 1);

        template <typename T>
        T* typed_reallocate(T* ptr, const std::size_t count = 1);

        std::size_t usage() const noexcept;
        std::size_t capacity() const noexcept;

    private:
        unsigned char _data[N];
        std::size_t _offset = 0;
        std::size_t _lastOffset = 0;
        const std::size_t _length = N;
    };

    template <typename T>
    class linear_allocator_lock
    {
    public:
        linear_allocator_lock(T& allocator);
        ~linear_allocator_lock();

        linear_allocator_lock(const linear_allocator_lock&) = delete;
        linear_allocator_lock(linear_allocator_lock&&) noexcept = delete;
        linear_allocator_lock<T>& operator=(const linear_allocator_lock&) = delete;
        linear_allocator_lock<T>& operator=(linear_allocator_lock&&) noexcept = delete;

    private:
        T& _allocator;
    };

    template <typename T>
    linear_allocator_lock(T) -> linear_allocator_lock<T>;

    template <typename T>
    inline T* linear_allocator::typed_allocate(const std::size_t count)
    {
        if (count == 0)
        {
            return nullptr;
        }
        static constexpr auto alignment = alignof(T);
        const std::size_t sz = count * sizeof(T);
        auto bufferSize = _length - _offset;

        void* current = &_data[_offset];
        void* adjusted = std::align(alignment, sz, current, bufferSize);

        if (adjusted)
        {
            const std::size_t offset = reinterpret_cast<const unsigned char*>(adjusted) - &_data[0];
            _lastOffset = offset;
            _offset = _lastOffset + sz;
        }

        return reinterpret_cast<T*>(adjusted);
    }

    template<typename T>
    inline T* linear_allocator::typed_reallocate(T* ptr, const std::size_t count)
    {
        static constexpr auto alignment = alignof(T);
        const std::size_t sz = count * sizeof(T);

        if (count == 0 || (sz + _offset > _length))
        {
            return nullptr;
        }

        auto bufferSize = _length - _offset;

        const std::size_t offset = reinterpret_cast<const unsigned char*>(ptr) - &_data[0];
        if (offset != _lastOffset)
        {
            return nullptr;
        }

        void* current = &_data[_lastOffset];
        void* adjusted = std::align(alignment, sz, current, bufferSize);

        if (adjusted)
        {
            const std::size_t offset = reinterpret_cast<const unsigned char*>(adjusted) - &_data[0];
            _offset = _lastOffset + sz;
        }

        return reinterpret_cast<T*>(adjusted);
    }

    template <std::size_t N>
    void* inline_linear_allocator<N>::allocate(const std::size_t bytes)
    {
        if (bytes == 0 || _offset + bytes > _length)
        {
            return nullptr;
        }

        void* ptr = &_data[_offset];
        _lastOffset = _offset;
        _offset += bytes;

        return ptr;
    }

    template <std::size_t N>
    inline void inline_linear_allocator<N>::deallocate(void* ptr)
    {
    }

    template <std::size_t N>
    inline void* inline_linear_allocator<N>::reallocate(void* ptr, const std::size_t bytes)
    {
        const std::size_t offset = reinterpret_cast<const unsigned char*>(ptr) - &_data[0];
        if (offset != _lastOffset)
        {
            return nullptr;
        }

        if (bytes + _offset > _length)
        {
            return nullptr;
        }

        _offset = _lastOffset + bytes;

        return ptr;
    }

    template <std::size_t N>
    inline void inline_linear_allocator<N>::reset()
    {
        _lastOffset = 0;
        _offset = 0;
    }

    template <std::size_t N>
    inline std::size_t inline_linear_allocator<N>::usage() const noexcept
    {
        return _offset;
    }

    template <std::size_t N>
    inline std::size_t inline_linear_allocator<N>::capacity() const noexcept
    {
        return _length;
    }
    
    template <std::size_t N>
    template <typename T>
    inline T* inline_linear_allocator<N>::typed_allocate(const std::size_t count)
    {
        static constexpr auto alignment = alignof(T);
        const std::size_t sz = count * sizeof(T);
        if (count == 0 || (sz + _offset > _length))
        {
            return nullptr;
        }

        auto bufferSize = _length - _offset;

        void* current = &_data[_offset];
        void* adjusted = std::align(alignment, sz, current, bufferSize);

        if (adjusted)
        {
            const std::size_t offset = reinterpret_cast<const unsigned char*>(adjusted) - &_data[0];
            _lastOffset = offset;
            _offset = _lastOffset + sz;
        }

        return reinterpret_cast<T*>(adjusted);
    }

    template<std::size_t N>
    template<typename T>
    inline T* inline_linear_allocator<N>::typed_reallocate(T* ptr, const std::size_t count)
    {
        static constexpr auto alignment = alignof(T);
        const std::size_t sz = count * sizeof(T);
        auto bufferSize = _length - _offset;

        const std::size_t offset = reinterpret_cast<const unsigned char*>(ptr) - &_data[0];
        if (offset != _lastOffset)
        {
            return nullptr;
        }

        if (sz + _offset > _lastOffset)
        {
            return nullptr;
        }

        void* current = &_data[_lastOffset];
        void* adjusted = std::align(alignment, sz, current, bufferSize);

        if (adjusted)
        {
            const std::size_t offset = reinterpret_cast<const unsigned char*>(adjusted) - &_data[0];
            _offset = _lastOffset + sz;
        }

        return reinterpret_cast<T*>(adjusted);
    }

    template <typename T>
    inline linear_allocator_lock<T>::linear_allocator_lock(T& allocator)
        : _allocator(allocator)
    {}

    template <typename T>
    inline linear_allocator_lock<T>::~linear_allocator_lock()
    {
        _allocator.reset();
    }
}

#endif // linear_allocator_hpp__
