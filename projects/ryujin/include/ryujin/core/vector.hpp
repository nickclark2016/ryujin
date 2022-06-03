#ifndef vector_hpp__
#define vector_hpp__

#include "allocator.hpp"
#include "algorithm.hpp"
#include "iterator.hpp"
#include "memory.hpp"
#include "primitives.hpp"
#include "result.hpp"
#include "utility.hpp"

namespace ryujin
{
    template <typename Type, typename Allocator = ryujin::allocator<Type>>
    class vector
    {
    public:
        using iterator = Type*;
        using const_iterator = const Type*;

        constexpr vector() noexcept;
        constexpr vector(const vector& v);
        constexpr vector(vector&& v) noexcept;
        constexpr vector(const sz count, const Type& value = Type());
        constexpr ~vector();

        constexpr vector& operator=(const vector& rhs);
        constexpr vector& operator=(vector&& rhs) noexcept;

        constexpr iterator begin() noexcept;
        constexpr const_iterator begin() const noexcept;
        constexpr const_iterator cbegin() const noexcept;

        constexpr iterator end() noexcept;
        constexpr const_iterator end() const noexcept;
        constexpr const_iterator cend() const noexcept;

        constexpr void clear();
        constexpr iterator erase(iterator it);
        constexpr iterator erase(iterator start, iterator stop);
        constexpr void pop_back();

        constexpr bool empty() const noexcept;
        constexpr sz size() const noexcept;
        constexpr sz capacity() const noexcept;
        constexpr Type* data() noexcept;
        constexpr const Type* data() const noexcept;

        constexpr void resize(const sz newSize, const Type& value = Type());
        constexpr void reserve(const sz newCapacity);

        constexpr void insert(const_iterator pos, const Type& value);
        constexpr void insert(const_iterator pos, Type&& value);
        constexpr void push_back(const Type& value);
        constexpr void push_back(Type&& value);

        constexpr Type& operator[](const sz idx) noexcept;
        constexpr const Type& operator[](const sz idx) const noexcept;

        constexpr Type& at(const sz idx) noexcept;
        constexpr const Type& at(const sz idx) const noexcept;

        constexpr Type& front() noexcept;
        constexpr const Type& front() const noexcept;
        constexpr Type& back() noexcept;
        constexpr const Type& back() const noexcept;

    private:
        Type* _data = {};
        sz _capacity = 0;
        sz _size = 0;
        Allocator _alloc = {};

        constexpr void _make_hole(const sz idx, const sz size);
        constexpr bool _needs_resize() const noexcept;
        constexpr void _resize_buffer(const sz newSize = 0) noexcept;
    };

    template <typename Type, typename Allocator>
    inline constexpr auto operator<=>(const vector<Type, Allocator>& lhs, const vector<Type, Allocator>& rhs)
    {
        if (&lhs == &rhs)
        {
            return 0;
        }

        if (lhs.size() != rhs.size())
        {
            return lhs.size() - rhs.size();
        }

        for (sz i = 0; i < lhs.size(); ++i)
        {
            auto cmp = lhs[i] <=> rhs[i];
            if (cmp)
            {
                return cmp;
            }
        }

        return 0;
    }
    
    template <typename Type, sz Capacity = 32>
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

        using pointer = Type*;
        using const_pointer = const Type*;
        using iterator = Type*;
        using const_iterator = const Type*;
        using reference = Type&;
        using const_reference = const Type&;

        constexpr static_vector();
        constexpr static_vector(const static_vector& other);
        constexpr static_vector(static_vector&& other) noexcept;
        ~static_vector() = default;

        constexpr static_vector& operator=(const static_vector& rhs);
        constexpr static_vector& operator=(static_vector&& rhs) noexcept;

        constexpr result<iterator, error_code> push_back(const Type& value);
        constexpr result<iterator, error_code> push_back(Type&& value) noexcept;
        constexpr result<iterator, error_code> insert(const_iterator position, const Type& value);
        constexpr result<iterator, error_code> insert(const_iterator position, Type&& value) noexcept;

        template <typename It>
        constexpr result<iterator, error_code> insert(const_iterator position, It start, It end) noexcept;
        
        constexpr error_code pop_back() noexcept;
        constexpr result<iterator, error_code> erase(const_iterator position);
        constexpr result<iterator, error_code> erase(const_iterator start, const_iterator end);
        constexpr void clear();

        constexpr iterator begin() noexcept;
        constexpr const_iterator begin() const noexcept;
        constexpr const_iterator cbegin() const noexcept;

        constexpr iterator end() noexcept;
        constexpr const_iterator end() const noexcept;
        constexpr const_iterator cend() const noexcept;

        constexpr reference operator[](const sz idx) noexcept;
        constexpr const_reference operator[](const sz idx) const noexcept;
        constexpr pointer data() noexcept;
        constexpr const_pointer data() const noexcept;

        constexpr sz size() const noexcept;
        constexpr sz capacity() const noexcept;
        constexpr bool empty() const noexcept;

    private:
        Type _data[Capacity];

        sz _size = 0;

        constexpr error_code _make_hole(const sz idx, const sz sz);
    };

    template<typename Type, typename Allocator>
    inline constexpr vector<Type, Allocator>::vector() noexcept
    {
    }

    template<typename Type, typename Allocator>
    inline constexpr vector<Type, Allocator>::vector(const vector& v)
        : _alloc(v._alloc)
    {
        _resize_buffer(v._capacity);
        _size = v._size;

        for (sz i = 0; i < _size; ++i)
        {
            ryujin::construct_at(_data + i, v[i]);
        }
    }

    template<typename Type, typename Allocator>
    inline constexpr vector<Type, Allocator>::vector(vector&& v) noexcept
        : _data(v._data), _capacity(v._capacity), _size(v._size), _alloc(ryujin::move(v._alloc))
    {
        v._data = nullptr;
        v._capacity = 0;
        v._size = 0;
    }

    template<typename Type, typename Allocator>
    inline constexpr vector<Type, Allocator>::vector(const sz count, const Type& value)
        : _size(count)
    {
        _resize_buffer(count);
        for (sz i = 0; i < _size; ++i)
        {
            ryujin::construct_at(_data + i, value);
        }
    }

    template<typename Type, typename Allocator>
    inline constexpr vector<Type, Allocator>::~vector()
    {
        if (_data)
        {
            clear();
            _alloc.deallocate(_data, _capacity);
        }
    }

    template<typename Type, typename Allocator>
    inline constexpr vector<Type, Allocator>& vector<Type, Allocator>::operator=(const vector& rhs)
    {
        if (&rhs == this)
        {
            return *this;
        }

        clear();
        if (size() < rhs.size())
        {
            _resize_buffer(rhs.capacity());
        }

        for (sz i = 0; i < rhs.size(); ++i)
        {
            _data[i] = rhs[i];
        }
        _size = rhs.size();

        return *this;
    }

    template<typename Type, typename Allocator>
    inline constexpr vector<Type, Allocator>& vector<Type, Allocator>::operator=(vector&& rhs) noexcept
    {
        if (&rhs == this)
        {
            return *this;
        }

        clear();

        _data = rhs._data;
        _size = rhs._size;
        _capacity = rhs._capacity;
        rhs._data = nullptr;
        rhs._size = 0;
        rhs._capacity = 0;

        return *this;
    }

    template<typename Type, typename Allocator>
    inline constexpr typename vector<Type, Allocator>::iterator vector<Type, Allocator>::begin() noexcept
    {
        return this->data();
    }
    
    template<typename Type, typename Allocator>
    inline constexpr typename vector<Type, Allocator>::const_iterator vector<Type, Allocator>::begin() const noexcept
    {
        return this->data();
    }
    
    template<typename Type, typename Allocator>
    inline constexpr typename vector<Type, Allocator>::const_iterator vector<Type, Allocator>::cbegin() const noexcept
    {
        return this->data();
    }
    
    template<typename Type, typename Allocator>
    inline constexpr typename vector<Type, Allocator>::iterator vector<Type, Allocator>::end() noexcept
    {
        return this->data() + this->size();
    }
    
    template<typename Type, typename Allocator>
    inline constexpr typename vector<Type, Allocator>::const_iterator vector<Type, Allocator>::end() const noexcept
    {
        return this->data() + this->size();
    }
    
    template<typename Type, typename Allocator>
    inline constexpr typename vector<Type, Allocator>::const_iterator vector<Type, Allocator>::cend() const noexcept
    {
        return this->data() + this->size();
    }

    template<typename Type, typename Allocator>
    inline constexpr bool vector<Type, Allocator>::empty() const noexcept
    {
        return _size == 0;
    }

    template<typename Type, typename Allocator>
    inline constexpr sz vector<Type, Allocator>::size() const noexcept
    {
        return _size;
    }

    template<typename Type, typename Allocator>
    inline constexpr sz vector<Type, Allocator>::capacity() const noexcept
    {
        return _capacity;
    }

    template<typename Type, typename Allocator>
    inline constexpr Type* vector<Type, Allocator>::data() noexcept
    {
        return _data;
    }

    template<typename Type, typename Allocator>
    inline constexpr const Type* vector<Type, Allocator>::data() const noexcept
    {
        return _data;
    }

    template<typename Type, typename Allocator>
    inline constexpr void vector<Type, Allocator>::clear()
    {
        erase(begin(), end());
    }

    template<typename Type, typename Allocator>
    inline constexpr typename vector<Type, Allocator>::iterator vector<Type, Allocator>::erase(iterator it)
    {
        return erase(it, it + 1);
    }

    template<typename Type, typename Allocator>
    inline constexpr vector<Type, Allocator>::iterator vector<Type, Allocator>::erase(iterator start, iterator stop)
    {
        const sz s = start - begin();
        const sz e = stop - begin();
        const sz count = e - s;

        for (sz i = s; i < _size; ++i)
        {
            if (i < _size - count)
            {
                _data[i] = ryujin::move(_data[i + count]);

                ryujin::destroy_at(_data + i + count);
            }
            else
            {
                ryujin::destroy_at(_data + i);
            }
        }

        _size -= count;

        return begin() + s;
    }

    template<typename Type, typename Allocator>
    void constexpr vector<Type, Allocator>::pop_back()
    {
        erase(end());
    }

    template<typename Type, typename Allocator>
    inline constexpr void vector<Type, Allocator>::resize(const sz newSize, const Type& value)
    {
        if (newSize <= _capacity)
        {
            return;
        }
        _resize_buffer(newSize);
        for (sz i = _size; i < newSize; ++i)
        {
            ryujin::construct_at(_data + i, value);
        }
        _size = newSize;
    }

    template<typename Type, typename Allocator>
    inline constexpr void vector<Type, Allocator>::reserve(const sz newCapacity)
    {
        if (newCapacity <= _capacity)
        {
            return;
        }
        _resize_buffer(newCapacity);
    }

    template<typename Type, typename Allocator>
    inline constexpr void vector<Type, Allocator>::insert(const_iterator pos, const Type& value)
    {
        const sz idx = pos - _data;
        if (_needs_resize())
        {
            _resize_buffer();
        }
        _make_hole(idx, 1);
        ryujin::construct_at(_data + idx, value);
        ++_size;
    }

    template<typename Type, typename Allocator>
    inline constexpr void vector<Type, Allocator>::insert(const_iterator pos, Type&& value)
    {
        const sz idx = pos - _data;
        if (_needs_resize())
        {
            _resize_buffer();
        }
        _make_hole(idx, 1);
        ryujin::construct_at(_data + idx, ryujin::forward<Type>(value));
        ++_size;
    }

    template<typename Type, typename Allocator>
    inline constexpr void vector<Type, Allocator>::push_back(const Type& value)
    {
        insert(end(), value);
    }

    template<typename Type, typename Allocator>
    inline constexpr void vector<Type, Allocator>::push_back(Type&& value)
    {
        insert(end(), ryujin::forward<Type>(value));
    }

    template<typename Type, typename Allocator>
    inline constexpr Type& vector<Type, Allocator>::operator[](const sz idx) noexcept
    {
        return _data[idx];
    }

    template<typename Type, typename Allocator>
    inline constexpr const Type& vector<Type, Allocator>::operator[](const sz idx) const noexcept
    {
        return _data[idx];
    }

    template<typename Type, typename Allocator>
    inline constexpr Type& vector<Type, Allocator>::at(const sz idx) noexcept
    {
        return _data[idx];
    }

    template<typename Type, typename Allocator>
    inline constexpr const Type& vector<Type, Allocator>::at(const sz idx) const noexcept
    {
        return _data[idx];
    }

    template<typename Type, typename Allocator>
    inline constexpr Type& vector<Type, Allocator>::front() noexcept
    {
        return at(0);
    }

    template<typename Type, typename Allocator>
    inline constexpr const Type& vector<Type, Allocator>::front() const noexcept
    {
        return at(0);
    }

    template<typename Type, typename Allocator>
    inline constexpr Type& vector<Type, Allocator>::back() noexcept
    {
        return at(_size - 1);
    }

    template<typename Type, typename Allocator>
    inline constexpr const Type& vector<Type, Allocator>::back() const noexcept
    {
        return at(_size - 1);
    }

    template<typename Type, typename Allocator>
    inline constexpr void vector<Type, Allocator>::_make_hole(const sz idx, const sz size)
    {
        if (idx == _size)
        {
            return;
        }

        // iterate from end to hole
        for (sz i = _size; i > idx; --i)
        {
            _data[i - 1 + size] = ryujin::move(_data[i - 1]);
        }

        for (sz i = idx; i < idx + size; ++i)
        {
            _data[i].~Type();
        }
    }

    template<typename Type, typename Allocator>
    inline constexpr bool vector<Type, Allocator>::_needs_resize() const noexcept
    {
        return _size == _capacity;
    }

    template<typename Type, typename Allocator>
    inline constexpr void vector<Type, Allocator>::_resize_buffer(const sz newSize) noexcept
    {
        const sz requested = newSize == 0 ? (_capacity == 0 ? 8 : _size * 2) : newSize;
        Type* allocation = _alloc.allocate(requested);
        for (sz i = 0; i < _size; ++i)
        {
            ::new(allocation + i) Type(ryujin::move(_data[i]));
        }
        if (_capacity)
        {
            _alloc.deallocate(_data, _capacity);
        }
        _capacity = requested;
        _data = allocation;
    }
    
    template<typename Type, sz Capacity>
    inline constexpr static_vector<Type, Capacity>::static_vector()
        : _data{ Type() }
    {
    }

    template<typename Type, sz Capacity>
    inline constexpr static_vector<Type, Capacity>::static_vector(const static_vector& other)
        : _size(other._size)
    {
        for (sz i = 0; i < _size; ++i)
        {
            _data[i] = other._data[i];
        }
    }

    template<typename Type, sz Capacity>
    inline constexpr static_vector<Type, Capacity>::static_vector(static_vector&& other) noexcept
        : _size(other._size)
    {
        ryujin::move(other._data, other._data + other._size, _data);
    }
    
    template<typename Type, sz Capacity>
    inline constexpr static_vector<Type, Capacity>& static_vector<Type, Capacity>::operator=(const static_vector& rhs)
    {
        if (this == &rhs)
            return *this;

        for (sz i = _size; i < rhs._size; ++i)
        {
            _data[i].~Type();
        }

        for (sz i = 0; i < rhs._size; ++i)
        {
            _data[i] = rhs._data[i];
        }

        _size = rhs._size;

        return *this;
    }

    template<typename Type, sz Capacity>
    inline constexpr static_vector<Type, Capacity>& static_vector<Type, Capacity>::operator=(static_vector&& rhs) noexcept
    {
        for (sz i = _size; i < rhs._size; ++i)
        {
            _data[i].~Type();
        }

        ryujin::move(ryujin::begin(rhs._data), ryujin::end(rhs._data), &_data[0]);

        _size = rhs._size;

        return *this;
    }
    
    template<typename Type, sz Capacity>
    inline constexpr sz static_vector<Type, Capacity>::size() const noexcept
    {
        return _size;
    }
    
    template<typename Type, sz Capacity>
    inline constexpr sz static_vector<Type, Capacity>::capacity() const noexcept
    {
        return Capacity;
    }

    template<typename Type, sz Capacity>
    inline constexpr bool static_vector<Type, Capacity>::empty() const noexcept
    {
        return _size == 0;
    }
    
    template<typename Type, sz Capacity>
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
    
    template<typename Type, sz Capacity>
    inline constexpr result<typename static_vector<Type, Capacity>::iterator, typename static_vector<Type, Capacity>::error_code> static_vector<Type, Capacity>::push_back(Type&& value) noexcept
    {
        if (_size >= Capacity)
        {
            return result<iterator, error_code>::from_error(error_code::OUT_OF_MEMORY);
        }

        iterator it = _data + _size;
        _data[_size++] = ryujin::move(value);

        return result<iterator, error_code>::from_success(it);
    }
    
    template<typename Type, sz Capacity>
    inline constexpr typename static_vector<Type, Capacity>::error_code static_vector<Type, Capacity>::pop_back() noexcept
    {
        if (_size == 0)
        {
            return error_code::INVALID_OPERATION;
        }

        _data[--_size].~Type();
        return error_code::SUCCESS;
    }

    template<typename Type, sz Capacity>
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

    template<typename Type, sz Capacity>
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
        _data[idx] = ryujin::move(value);

        ++_size;

        return result<iterator, error_code>::from_success(begin() + idx);
    }

    template<typename Type, sz Capacity>
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

        for (sz i = 0; i < count; ++i)
        {
            _data[idx + i] = *it;
            ++it;
        }

        _size += count;

        return result<iterator, error_code>::from_success(begin() + idx);
    }

    template<typename Type, sz Capacity>
    inline constexpr result<typename static_vector<Type, Capacity>::iterator, typename static_vector<Type, Capacity>::error_code> static_vector<Type, Capacity>::erase(const_iterator position)
    {
        if (position == end())
        {
            return result<iterator, error_code>::from_success(end());
        }
        return erase(position, position + 1);
    }

    template<typename Type, sz Capacity>
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
        
        for (sz i = s; i < e; ++i)
        {
            if (i < _size - count)
            {
                _data[i] = ryujin::move(_data[i + count]);
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

    template<typename Type, sz Capacity>
    inline constexpr void static_vector<Type, Capacity>::clear()
    {
        erase(begin(), end());
    }

    template<typename Type, sz Capacity>
    inline constexpr typename static_vector<Type, Capacity>::iterator static_vector<Type, Capacity>::begin() noexcept
    {
        return &_data[0];
    }

    template<typename Type, sz Capacity>
    inline constexpr typename static_vector<Type, Capacity>::const_iterator static_vector<Type, Capacity>::begin() const noexcept
    {
        return &_data[0];
    }

    template<typename Type, sz Capacity>
    inline constexpr typename static_vector<Type, Capacity>::const_iterator static_vector<Type, Capacity>::cbegin() const noexcept
    {
        return &_data[0];
    }

    template<typename Type, sz Capacity>
    inline constexpr typename static_vector<Type, Capacity>::iterator static_vector<Type, Capacity>::end() noexcept
    {
        return _data + _size;
    }

    template<typename Type, sz Capacity>
    inline constexpr typename static_vector<Type, Capacity>::const_iterator static_vector<Type, Capacity>::end() const noexcept
    {
        return _data + _size;
    }

    template<typename Type, sz Capacity>
    inline constexpr typename static_vector<Type, Capacity>::const_iterator static_vector<Type, Capacity>::cend() const noexcept
    {
        return _data + _size;
    }

    template<typename Type, sz Capacity>
    inline constexpr typename static_vector<Type, Capacity>::reference static_vector<Type, Capacity>::operator[](const sz idx) noexcept
    {
        return _data[idx];
    }

    template<typename Type, sz Capacity>
    inline constexpr typename static_vector<Type, Capacity>::const_reference static_vector<Type, Capacity>::operator[](const sz idx) const noexcept
    {
        return _data[idx];
    }

    template<typename Type, sz Capacity>
    inline constexpr typename static_vector<Type, Capacity>::pointer static_vector<Type, Capacity>::data() noexcept
    {
        return _data;
    }
    
    template<typename Type, sz Capacity>
    inline constexpr typename static_vector<Type, Capacity>::const_pointer static_vector<Type, Capacity>::data() const noexcept
    {
        return _data;
    }
    
    template<typename Type, sz Capacity>
    inline constexpr typename static_vector<Type, Capacity>::error_code static_vector<Type, Capacity>::_make_hole(const sz idx, const sz size)
    {
        if (_size + size > Capacity)
        {
            return error_code::OUT_OF_MEMORY;
        }

        // iterate from end to hole
        for (sz i = _size; i > idx; --i)
        {
            _data[i - 1 + size] = ryujin::move(_data[i - 1]);
        }
        
        for (sz i = idx; i < idx + size; ++i)
        {
            _data[i].~Type();
        }

        return error_code::SUCCESS;
    }
}

#endif // vector_hpp__
