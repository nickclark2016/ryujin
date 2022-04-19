#ifndef linear_allocator_hpp__
#define linear_allocator_hpp__

#include "primitives.hpp"

#include <memory>
#include <new>

namespace ryujin
{
    class linear_allocator
    {
    public:
        linear_allocator(const sz bytes);

        void* allocate(const sz bytes);
        void deallocate(void* ptr);
        void* reallocate(void* ptr, const sz bytes);
        void reset();

        template <typename T>
        T* typed_allocate(const sz count = 1);

        template <typename T>
        T* typed_reallocate(T* ptr, const sz count = 1);

        sz usage() const noexcept;
        sz capacity() const noexcept;

    private:
        std::unique_ptr<unsigned char[]> _data;
        sz _offset;
        sz _lastOffset;
        const sz _length;
    };

    template <sz N>
    class inline_linear_allocator
    {
    public:
        void* allocate(const sz bytes);
        void deallocate(void* ptr);
        void* reallocate(void* ptr, const sz bytes);
        void reset();

        template <typename T>
        T* typed_allocate(const sz count = 1);

        template <typename T>
        T* typed_reallocate(T* ptr, const sz count = 1);

        sz usage() const noexcept;
        sz capacity() const noexcept;

    private:
        unsigned char _data[N];
        sz _offset = 0;
        sz _lastOffset = 0;
        const sz _length = N;
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
    inline T* linear_allocator::typed_allocate(const sz count)
    {
        if (count == 0)
        {
            return nullptr;
        }
        static constexpr auto alignment = alignof(T);
        const sz size = count * sizeof(T);
        auto bufferSize = _length - _offset;

        void* current = &_data[_offset];
        void* adjusted = std::align(alignment, size, current, bufferSize);

        if (adjusted)
        {
            const sz offset = reinterpret_cast<const unsigned char*>(adjusted) - &_data[0];
            _lastOffset = offset;
            _offset = _lastOffset + size;
        }

        return reinterpret_cast<T*>(adjusted);
    }

    template<typename T>
    inline T* linear_allocator::typed_reallocate(T* ptr, const sz count)
    {
        static constexpr auto alignment = alignof(T);
        const sz size = count * sizeof(T);

        if (count == 0 || (size + _offset > _length))
        {
            return nullptr;
        }

        auto bufferSize = _length - _offset;

        const sz offset = reinterpret_cast<const unsigned char*>(ptr) - &_data[0];
        if (offset != _lastOffset)
        {
            return nullptr;
        }

        void* current = &_data[_lastOffset];
        void* adjusted = std::align(alignment, size, current, bufferSize);

        if (adjusted)
        {
            const sz offset = reinterpret_cast<const unsigned char*>(adjusted) - &_data[0];
            _offset = _lastOffset + size;
        }

        return reinterpret_cast<T*>(adjusted);
    }

    template <sz N>
    void* inline_linear_allocator<N>::allocate(const sz bytes)
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

    template <sz N>
    inline void inline_linear_allocator<N>::deallocate(void* ptr)
    {
    }

    template <sz N>
    inline void* inline_linear_allocator<N>::reallocate(void* ptr, const sz bytes)
    {
        const sz offset = reinterpret_cast<const unsigned char*>(ptr) - &_data[0];
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

    template <sz N>
    inline void inline_linear_allocator<N>::reset()
    {
        _lastOffset = 0;
        _offset = 0;
    }

    template <sz N>
    inline sz inline_linear_allocator<N>::usage() const noexcept
    {
        return _offset;
    }

    template <sz N>
    inline sz inline_linear_allocator<N>::capacity() const noexcept
    {
        return _length;
    }
    
    template <sz N>
    template <typename T>
    inline T* inline_linear_allocator<N>::typed_allocate(const sz count)
    {
        static constexpr auto alignment = alignof(T);
        const sz size = count * sizeof(T);
        if (count == 0 || (size + _offset > _length))
        {
            return nullptr;
        }

        auto bufferSize = _length - _offset;

        void* current = &_data[_offset];
        void* adjusted = std::align(alignment, size, current, bufferSize);

        if (adjusted)
        {
            const sz offset = reinterpret_cast<const unsigned char*>(adjusted) - &_data[0];
            _lastOffset = offset;
            _offset = _lastOffset + size;
        }

        return reinterpret_cast<T*>(adjusted);
    }

    template<sz N>
    template<typename T>
    inline T* inline_linear_allocator<N>::typed_reallocate(T* ptr, const sz count)
    {
        static constexpr auto alignment = alignof(T);
        const sz size = count * sizeof(T);
        auto bufferSize = _length - _offset;

        const sz offset = reinterpret_cast<const unsigned char*>(ptr) - &_data[0];
        if (offset != _lastOffset)
        {
            return nullptr;
        }

        if (size + _offset > _lastOffset)
        {
            return nullptr;
        }

        void* current = &_data[_lastOffset];
        void* adjusted = std::align(alignment, size, current, bufferSize);

        if (adjusted)
        {
            const sz offset = reinterpret_cast<const unsigned char*>(adjusted) - &_data[0];
            _offset = _lastOffset + size;
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
