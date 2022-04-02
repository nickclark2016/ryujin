#ifndef vector_hpp__
#define vector_hpp__

#include "result.hpp"

#include <memory>
#include <vector>

namespace ryujin
{
    template <typename Type, typename Allocator = std::allocator<Type>>
    class vector : public std::vector<Type, Allocator>
    {
    public:
        using iterator = Type*;
        using const_iterator = const Type*;

        iterator begin() noexcept;
        iterator begin() const noexcept;
        const_iterator cbegin() const noexcept;

        iterator end() noexcept;
        iterator end() const noexcept;
        const_iterator cend() const noexcept;

        iterator erase(iterator it);
        iterator erase(iterator start, iterator stop);
    };
    
    template <typename Type, std::size_t Capacity = 32>
    class static_vector
    {
    public:
        enum class error_code
        {
            SUCCESS,
            OUT_OF_MEMORY,
            INVALID_OPERATION,
            INDEX_OUT_OF_BOUNDS
        };

        using iterator = Type*;
        using const_iterator = const Type*;
        using reference = Type&;
        using const_reference = const Type&;

        constexpr static_vector();
        constexpr static_vector(const static_vector& other);
        constexpr static_vector(static_vector&& other) noexcept;
        constexpr ~static_vector() = default;

        constexpr static_vector& operator=(const static_vector& rhs);
        constexpr static_vector& operator=(static_vector&& rhs) noexcept;

        constexpr std::size_t size() const noexcept;
        constexpr std::size_t capacity() const noexcept;
        constexpr bool empty() const noexcept;

        constexpr result<iterator, error_code> push_back(const Type& value);
        constexpr result<iterator, error_code> push_back(Type&& value) noexcept;

        constexpr error_code pop_back() noexcept;
        
        constexpr result<iterator, error_code> insert(const_iterator position, const Type& value);
        constexpr result<iterator, error_code> insert(const_iterator position, Type&& value) noexcept;

        template <typename It>
        constexpr result<iterator, error_code> insert(const_iterator position, It start, It end) noexcept;
        
        constexpr result<iterator, error_code> erase(const_iterator position);
        constexpr result<iterator, error_code> erase(const_iterator start, const_iterator end);
        constexpr void clear();

        constexpr iterator begin() noexcept;
        constexpr iterator begin() const noexcept;
        constexpr const_iterator cbegin() const noexcept;

        constexpr iterator end() noexcept;
        constexpr iterator end() const noexcept;
        constexpr const_iterator cend() const noexcept;

        constexpr reference operator[](const std::size_t idx);
        constexpr const_reference operator[](const std::size_t idx) const;

    private:
        Type _data[Capacity];

        std::size_t _size = 0;

        error_code _make_hole(const std::size_t idx, const std::size_t sz);
    };

    template<typename Type, typename Allocator>
    inline vector<Type, Allocator>::template iterator vector<Type, Allocator>::begin() noexcept
    {
        return this->data();
    }
    
    template<typename Type, typename Allocator>
    inline vector<Type, Allocator>::template iterator vector<Type, Allocator>::begin() const noexcept
    {
        return this->data();
    }
    
    template<typename Type, typename Allocator>
    inline vector<Type, Allocator>::template const_iterator vector<Type, Allocator>::cbegin() const noexcept
    {
        return this->data();
    }
    
    template<typename Type, typename Allocator>
    inline vector<Type, Allocator>::template iterator vector<Type, Allocator>::end() noexcept
    {
        return this->data() + this->size();
    }
    
    template<typename Type, typename Allocator>
    inline vector<Type, Allocator>::template iterator vector<Type, Allocator>::end() const noexcept
    {
        return this->data() + this->size();
    }
    
    template<typename Type, typename Allocator>
    inline vector<Type, Allocator>::template const_iterator vector<Type, Allocator>::cend() const noexcept
    {
        return this->data() + this->size();
    }
    
    template<typename Type, typename Allocator>
    inline vector<Type, Allocator>::template iterator vector<Type, Allocator>::erase(iterator it)
    {
        const auto idx = it - begin();
        const auto res = std::vector<Type, Allocator>::erase(std::vector<Type, Allocator>::begin() + idx);
        return begin() + (res - std::vector<Type, Allocator>::begin());
    }
    
    template<typename Type, typename Allocator>
    inline vector<Type, Allocator>::template iterator vector<Type, Allocator>::erase(iterator start, iterator stop)
    {
        const auto startIdx = start - begin();
        const auto stopIdx = stop - begin();
        const auto res = std::vector<Type, Allocator>::erase(std::vector<Type, Allocator>::begin() + startIdx, std::vector<Type, Allocator>::begin() + stopIdx);
        return begin() + (res - std::vector<Type, Allocator>::begin());
    }
    
    template<typename Type, std::size_t Capacity>
    inline constexpr static_vector<Type, Capacity>::static_vector()
        : _data{ Type() }
    {
    }

    template<typename Type, std::size_t Capacity>
    inline constexpr static_vector<Type, Capacity>::static_vector(const static_vector& other)
        : _size(other._size)
    {
        for (std::size_t i = 0; i < _size; ++i)
        {
            _data[i] = other._data[i];
        }
    }

    template<typename Type, std::size_t Capacity>
    inline constexpr static_vector<Type, Capacity>::static_vector(static_vector&& other) noexcept
        : _size(other._size)
    {
        std::move(other._data, other._data + other._size, _data);
    }
    
    template<typename Type, std::size_t Capacity>
    inline constexpr static_vector<Type, Capacity>& static_vector<Type, Capacity>::operator=(const static_vector& rhs)
    {
        for (std::size_t i = _size; i < rhs._size; ++i)
        {
            _data[i].~Type();
        }

        for (std::size_t i = 0; i < rhs._size; ++i)
        {
            _data[i] = rhs._data[i];
        }

        _size = rhs._size;

        return *this;
    }

    template<typename Type, std::size_t Capacity>
    inline constexpr static_vector<Type, Capacity>& static_vector<Type, Capacity>::operator=(static_vector&& rhs) noexcept
    {
        for (std::size_t i = _size; i < rhs._size; ++i)
        {
            _data[i].~Type();
        }

        std::move(std::begin(rhs._data), std::end(rhs._data), &_data[0]);

        _size = rhs._size;

        return *this;
    }
    
    template<typename Type, std::size_t Capacity>
    inline constexpr std::size_t static_vector<Type, Capacity>::size() const noexcept
    {
        return _size;
    }
    
    template<typename Type, std::size_t Capacity>
    inline constexpr std::size_t static_vector<Type, Capacity>::capacity() const noexcept
    {
        return Capacity;
    }

    template<typename Type, std::size_t Capacity>
    inline constexpr bool static_vector<Type, Capacity>::empty() const noexcept
    {
        return _size == 0;
    }
    
    template<typename Type, std::size_t Capacity>
    inline constexpr result<typename static_vector<Type, Capacity>::iterator, typename static_vector<Type, Capacity>::error_code> static_vector<Type, Capacity>::push_back(const Type& value)
    {
        if (_size >= Capacity)
        {
            return result<iterator, error_code>::from_error(error_code::OUT_OF_MEMORY);
        }

        iterator it = _data + _size;
        _data[_size++] = value;

        return result<iterator, error_code>::from_success(it);
    }
    
    template<typename Type, std::size_t Capacity>
    inline constexpr result<typename static_vector<Type, Capacity>::iterator, typename static_vector<Type, Capacity>::error_code> static_vector<Type, Capacity>::push_back(Type&& value) noexcept
    {
        if (_size >= Capacity)
        {
            return result<iterator, error_code>::from_error(error_code::OUT_OF_MEMORY);
        }

        iterator it = _data + _size;
        _data[_size++] = std::move(value);

        return result<iterator, error_code>::from_success(it);
    }
    
    template<typename Type, std::size_t Capacity>
    inline constexpr static_vector<Type, Capacity>::error_code static_vector<Type, Capacity>::pop_back() noexcept
    {
        if (_size == 0)
        {
            return error_code::INVALID_OPERATION;
        }

        _data[--_size].~Type();
        return error_code::SUCCESS;
    }

    template<typename Type, std::size_t Capacity>
    inline constexpr result<typename static_vector<Type, Capacity>::iterator, typename static_vector<Type, Capacity>::error_code> static_vector<Type, Capacity>::insert(const_iterator position, const Type& value)
    {
        if (_size >= Capacity)
        {
            return result<iterator, error_code>::from_error(error_code::OUT_OF_MEMORY);
        }

        const size_t idx = position - begin();
        if (idx > _size)
        {
            return result<iterator, error_code>::from_error(error_code::INVALID_OPERATION);
        }

        _make_hole(idx, 1);
        _data[idx] = value;

        ++_size;

        return result<iterator, error_code>::from_success(begin() + idx);
    }

    template<typename Type, std::size_t Capacity>
    inline constexpr result<typename static_vector<Type, Capacity>::iterator, typename static_vector<Type, Capacity>::error_code> static_vector<Type, Capacity>::insert(const_iterator position, Type&& value) noexcept
    {
        if (_size >= Capacity)
        {
            return result<iterator, error_code>::from_error(error_code::OUT_OF_MEMORY);
        }

        const size_t idx = position - begin();
        if (idx > _size)
        {
            return result<iterator, error_code>::from_error(error_code::INVALID_OPERATION);
        }

        _make_hole(idx, 1);
        _data[idx] = std::move(value);

        ++_size;

        return result<iterator, error_code>::from_success(begin() + idx);
    }

    template<typename Type, std::size_t Capacity>
    template<typename It>
    inline constexpr  result<typename static_vector<Type, Capacity>::iterator, typename static_vector<Type, Capacity>::error_code> static_vector<Type, Capacity>::insert(const_iterator position, It start, It end) noexcept
    {
        const size_t count = end - start;

        if (_size + count > Capacity)
        {
            return result<iterator, error_code>::from_error(error_code::OUT_OF_MEMORY);
        }

        const size_t idx = position - begin();
        if (idx > _size)
        {
            return result<iterator, error_code>::from_error(error_code::INVALID_OPERATION);
        }

        _make_hole(idx, count);
        
        auto it = start;

        for (std::size_t i = 0; i < count; ++i)
        {
            _data[idx + i] = *it;
            ++it;
        }

        _size += count;

        return result<iterator, error_code>::from_success(begin() + idx);
    }

    template<typename Type, std::size_t Capacity>
    inline constexpr result<typename static_vector<Type, Capacity>::iterator, typename static_vector<Type, Capacity>::error_code> static_vector<Type, Capacity>::erase(const_iterator position)
    {
        if (position == end())
        {
            return result<iterator, error_code>::from_success(end());
        }
        return erase(position, position + 1);
    }

    template<typename Type, std::size_t Capacity>
    inline constexpr result<typename static_vector<Type, Capacity>::iterator, typename static_vector<Type, Capacity>::error_code> static_vector<Type, Capacity>::erase(const_iterator start, const_iterator end)
    {
        const size_t s = start - begin();
        const size_t e = end - begin();

        const size_t count = e - s;

        if (s > e || e > _size)
        {
            return result<iterator, error_code>::from_error(error_code::INVALID_OPERATION);
        }

        if (s == e)
        {
            return result<iterator, error_code>::from_success(begin() + e);
        }
        
        for (std::size_t i = s; i < e; ++i)
        {
            if (i < _size - count)
            {
                _data[i] = std::move(_data[i + count]);
                _data[i + count].~Type();
            }
            else
            {
                _data[i].~Type();
            }
        }

        _size -= count;

        return result<iterator, error_code>::from_success(begin() + s);
    }

    template<typename Type, std::size_t Capacity>
    inline constexpr void static_vector<Type, Capacity>::clear()
    {
        erase(begin(), end());
    }

    template<typename Type, std::size_t Capacity>
    inline constexpr typename static_vector<Type, Capacity>::iterator static_vector<Type, Capacity>::begin() noexcept
    {
        return &_data[0];
    }

    template<typename Type, std::size_t Capacity>
    inline constexpr static_vector<Type, Capacity>::iterator static_vector<Type, Capacity>::begin() const noexcept
    {
        return &_data[0];
    }

    template<typename Type, std::size_t Capacity>
    inline constexpr static_vector<Type, Capacity>::const_iterator static_vector<Type, Capacity>::cbegin() const noexcept
    {
        return &_data[0];
    }

    template<typename Type, std::size_t Capacity>
    inline constexpr static_vector<Type, Capacity>::iterator static_vector<Type, Capacity>::end() noexcept
    {
        return _data + _size;
    }

    template<typename Type, std::size_t Capacity>
    inline constexpr static_vector<Type, Capacity>::iterator static_vector<Type, Capacity>::end() const noexcept
    {
        return _data + _size;
    }

    template<typename Type, std::size_t Capacity>
    inline constexpr static_vector<Type, Capacity>::const_iterator static_vector<Type, Capacity>::cend() const noexcept
    {
        return _data + _size;
    }

    template<typename Type, std::size_t Capacity>
    inline constexpr static_vector<Type, Capacity>::reference static_vector<Type, Capacity>::operator[](const std::size_t idx)
    {
        return _data[idx];
    }

    template<typename Type, std::size_t Capacity>
    inline constexpr static_vector<Type, Capacity>::const_reference static_vector<Type, Capacity>::operator[](const std::size_t idx) const
    {
        return _data[idx];
    }
    
    template<typename Type, std::size_t Capacity>
    inline static_vector<Type, Capacity>::error_code static_vector<Type, Capacity>::_make_hole(const std::size_t idx, const std::size_t size)
    {
        if (_size + size > Capacity)
        {
            return error_code::OUT_OF_MEMORY;
        }

        // iterate from end to hole
        for (std::size_t i = _size; i > idx; --i)
        {
            _data[i - 1 + size] = std::move(_data[i - 1]);
        }
        
        for (std::size_t i = idx; i < idx + size; ++i)
        {
            _data[i].~Type();
        }

        return error_code::SUCCESS;
    }
}

#endif // vector_hpp__