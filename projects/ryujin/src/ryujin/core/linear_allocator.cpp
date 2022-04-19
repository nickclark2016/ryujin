#include <ryujin/core/linear_allocator.hpp>

#include <ryujin/core/primitives.hpp>

namespace ryujin
{
    linear_allocator::linear_allocator(const sz bytes)
        : _data(std::make_unique<unsigned char[]>(bytes)), _offset(0), _lastOffset(0), _length(bytes)
    {
    }

    void* linear_allocator::allocate(const sz bytes)
    {
        if (bytes == 0 || _offset + bytes >= _length)
        {
            return nullptr;
        }
        
        void* ptr = &_data[_offset];
        _lastOffset = _offset;
        _offset += bytes;

        return ptr;
    }
    
    void linear_allocator::deallocate(void* ptr)
    {
        // NO OP
    }
    
    void* linear_allocator::reallocate(void* ptr, const sz bytes)
    {
        const sz offset = reinterpret_cast<const unsigned char*>(ptr) - &_data[0];
        if (offset != _lastOffset)
        {
            return nullptr;
        }
        _offset = _lastOffset + bytes;

        return ptr;
    }

    void linear_allocator::reset()
    {
        _lastOffset = 0;
        _offset = 0;
    }
    
    sz linear_allocator::usage() const noexcept
    {
        return _offset;
    }
    
    sz linear_allocator::capacity() const noexcept
    {
        return _length;
    }
}