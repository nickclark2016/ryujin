#include <ryujin/core/linear_allocator.hpp>

namespace ryujin
{
    linear_allocator::linear_allocator(const std::size_t bytes)
        : _data(std::make_unique<unsigned char[]>(bytes)), _offset(0), _lastOffset(0), _length(bytes)
    {
    }

    void* linear_allocator::allocate(const std::size_t bytes)
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
    
    void* linear_allocator::reallocate(void* ptr, const std::size_t bytes)
    {
        const std::size_t offset = reinterpret_cast<const unsigned char*>(ptr) - &_data[0];
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
    
    std::size_t linear_allocator::usage() const noexcept
    {
        return _offset;
    }
    
    std::size_t linear_allocator::capacity() const noexcept
    {
        return _length;
    }
}