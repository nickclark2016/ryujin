#ifndef string_hpp__
#define string_hpp__

#include "allocator.hpp"
#include "as.hpp"
#include "primitives.hpp"
#include "utility.hpp"

#include <cassert>
#include <memory>

/// TODO: Capacity opt
/// TODO: substr
/// TODO: start_with, ends_with, first_index_of, last_index_of, rfirst_index_of, rlast_index_of, replace, reserve, contains, find, rfind, split
/// TODO: hash

namespace ryujin
{
	template<typename Type, typename Allocator = allocator<Type>>
	class basic_string
	{
	public:
		using iterator = Type*;
		using const_iterator = const Type*;

		constexpr basic_string();
		constexpr ~basic_string();
		explicit constexpr basic_string(sz size) noexcept;

		explicit constexpr basic_string(const basic_string& s) noexcept;
		constexpr basic_string(basic_string&& s) noexcept;

		constexpr basic_string(const Type* data) noexcept;
		constexpr basic_string(const Type* data, sz size) noexcept;

		template<sz N>
		constexpr basic_string(const Type(&data)[N]) noexcept;

		constexpr bool operator==(const basic_string& rhs) const noexcept;
		constexpr bool operator!=(const basic_string& rhs) const noexcept;

		constexpr basic_string& operator=(const basic_string& rhs) noexcept;
		constexpr basic_string& operator=(basic_string&& rhs) noexcept;

		constexpr basic_string& operator=(const Type* data) noexcept;
		template<sz N>
		constexpr basic_string& operator=(const Type(&data)[N]) noexcept;

		constexpr basic_string& operator+=(const basic_string& rhs) noexcept;
		constexpr basic_string& operator+=(const Type* rhs) noexcept;

		template<sz N>
		constexpr basic_string& operator+=(const Type(&data)[N]) noexcept;

		constexpr Type& operator[](sz index) const noexcept;

		constexpr iterator begin() noexcept;
		constexpr const_iterator begin() const noexcept;
		constexpr const_iterator cbegin() const noexcept;

		constexpr iterator end() noexcept;
		constexpr const_iterator end() const noexcept;
		constexpr const_iterator cend() const noexcept;

		constexpr void clear();
		constexpr iterator erase(iterator it);
		constexpr iterator erase(iterator start, iterator stop);

		constexpr sz length() const noexcept;
		constexpr bool empty() const noexcept;

	private:
		Type* _data = nullptr;
		sz _size = 0;

		Allocator _alloc;

		static constexpr Type _empty[1] = { 0 };

		constexpr void _set_empty() noexcept;
	};

	template<typename Type>
	inline constexpr static sz strlen(const Type* data) noexcept
	{
		sz i = 0;
		while (data[i] != 0) ++i;
		return i;
	}

	template<typename Type>
	inline constexpr static i32 strcmp(const Type* lhs, const Type* rhs) noexcept
	{
		if (lhs == rhs)
		{
			return 0;
		}

		sz index = 0;

		sz size = ryujin::strlen(lhs);
		sz rhsSize = ryujin::strlen(rhs);

		if (size < rhsSize)
		{
			return -1;
		}
		if (size > rhsSize)
		{
			return 1;
		}

		while (index < size)
		{
			if (lhs[index] != rhs[index])
			{
				return lhs[index] - rhs[index];
			}
			index++;
		}

		return 0;
	}

	template<typename Type, typename Allocator>
	inline constexpr basic_string<Type, Allocator>::basic_string()
	{
		_set_empty();
	}

	template <typename Type, typename Allocator>
	inline constexpr basic_string<Type, Allocator>::~basic_string()
	{
		if (!empty())
		{
			_alloc.deallocate(_data, _size + 1);
		}
	}

	template <typename Type, typename Allocator>
	inline constexpr basic_string<Type, Allocator>::basic_string(const sz size) noexcept : _size(size)
	{
		if (_size == 0)
		{
			_set_empty();
		}
		else
		{
			_data = _alloc.allocate(_size + 1);
		}
	}

	template <typename Type, typename Allocator>
	inline constexpr basic_string<Type, Allocator>::basic_string(const basic_string& s) noexcept
	{
		if (!s.empty())
		{
			_size = s._size;
			_data = _alloc.allocate(_size + 1);
			memcpy(_data, s._data, _size * sizeof(Type));
			_data[_size] = as<Type>(0);
		}
		else
		{
			_set_empty();
		}
	}

	template <typename Type, typename Allocator>
	constexpr basic_string<Type, Allocator>::basic_string(basic_string&& s) noexcept
	{
		_size = s._size;
		_data = move(s._data);
		_alloc = move(s._alloc);

		s._size = 0;
		s._set_empty();
	}

	template<typename Type, typename Allocator>
	inline constexpr basic_string<Type, Allocator>::basic_string(const Type* data) noexcept
	{
		_size = ryujin::strlen(data);

		if (_size != 0)
		{
			_data = _alloc.allocate(_size + 1);
			memcpy(_data, data, _size);
			_data[_size] = as<Type>(0);
		}
		else
		{
			_set_empty();
		}
	}

	template <typename Type, typename Allocator>
	inline constexpr basic_string<Type, Allocator>::basic_string(const Type* data, const sz size) noexcept
	{
		if (size != 0)
		{
			_size = size;

			_data = _alloc.allocate(size + 1);
			memcpy(_data, data, size);
			_data[_size] = as<Type>(0);
		}
		else
		{
			_set_empty();
		}
	}

	template<typename Type, typename Allocator>
	template<sz N>
	inline constexpr basic_string<Type, Allocator>::basic_string(const Type(&data)[N]) noexcept
	{
		if (N != 0)
		{
			_size = N;

			_data = _alloc.allocate(_size + 1);
			memcpy(_data, data, _size);
			_data[_size] = as<Type>(0);
		}
		else
		{
			_set_empty();
		}
	}

	template<typename Type, typename Allocator>
	inline constexpr bool basic_string<Type, Allocator>::operator==(const basic_string& rhs) const noexcept
	{
		return ryujin::strcmp(_data, rhs._data) == 0;
	}

	template<typename Type, typename Allocator>
	inline constexpr bool basic_string<Type, Allocator>::operator!=(const basic_string& rhs) const noexcept
	{
		return ryujin::strcmp(_data, rhs._data) != 0;
	}

	template<typename Type, typename Allocator>
	inline constexpr basic_string<Type, Allocator>& basic_string<Type, Allocator>::operator=(const basic_string<Type, Allocator>& rhs) noexcept
	{
		if (&rhs == this)
		{
			return *this;
		}

		if (!empty())
		{
			_alloc.deallocate(_data, _size + 1);
		}

		if (!rhs.empty())
		{
			_size = rhs._size;

			_data = _alloc.allocate(_size + 1);
			memcpy(_data, rhs._data, _size);
			_data[_size] = as<Type>(0);
		}
		else
		{
			_set_empty();
		}

		return *this;
	}

	template<typename Type, typename Allocator>
	inline constexpr basic_string<Type, Allocator>& basic_string<Type, Allocator>::operator=(basic_string<Type, Allocator>&& rhs) noexcept
	{
		if (&rhs == this)
		{
			return *this;
		}

		if (!empty())
		{
			_alloc.deallocate(_data, _size + 1);
		}

		_size = rhs._size;
		_data = move(rhs._data);
		_alloc = move(rhs._alloc);

		rhs._size = 0;
		rhs._set_empty();

		return *this;
	}

	template<typename Type, typename Allocator>
	inline constexpr basic_string<Type, Allocator>& basic_string<Type, Allocator>::operator=(const Type* data) noexcept
	{
		if (!empty())
		{
			_alloc.deallocate(_data, _size + 1);
		}

		_size = ryujin::strlen(data);
		
		if (_size != 0)
		{
			_data = _alloc.allocate(_size + 1);
			memcpy(_data, data, _size);
			_data[_size] = as<Type>(0);
		}
		else
		{
			_set_empty();
		}

		return *this;
	}

	template<typename Type, typename Allocator>
	template<sz N>
	inline constexpr basic_string<Type, Allocator>& basic_string<Type, Allocator>::operator=(const Type(&data)[N]) noexcept
	{
		if (!empty())
		{
			_alloc.deallocate(_data, _size + 1);
		}

		_size = N;

		if (_size != 0)
		{
			_data = _alloc.allocate(_size + 1);
			memcpy(_data, data, _size);
			_data[_size] = as<Type>(0);
		}
		else
		{
			_set_empty();
		}

		return *this;
	}

	template<typename Type, typename Allocator>
	inline constexpr basic_string<Type, Allocator>& basic_string<Type, Allocator>::operator+=(const basic_string& rhs) noexcept
	{
		if (empty())
		{
			return operator=(rhs);
		}

		if (rhs.empty())
		{
			return *this;
		}

		const sz newSize = _size + rhs._size;
		const Type* buffer = _alloc.allocate(newSize + 1);
		memcpy(buffer, _data, _size);
		memcpy(buffer + _size, rhs._data, rhs._size);
		buffer[newSize] = as<Type>(0);

		_alloc.deallocate(_data, _size + 1);
		_size = newSize;

		_data = buffer;

		return *this;
	}

	template<typename Type, typename Allocator>
	inline constexpr basic_string<Type, Allocator>& basic_string<Type, Allocator>::operator+=(const Type* rhs) noexcept
	{
		if (empty())
		{
			return operator=(rhs);
		}

		const sz rhsSize = ryujin::strlen(rhs);
		if (rhsSize == 0)
		{
			return *this;
		}

		const sz newSize = _size + rhsSize;
		const Type* buffer = _alloc.allocate(newSize + 1);
		memcpy(buffer, _data, _size);
		memcpy(buffer + _size, rhs, rhsSize);
		buffer[newSize] = as<Type>(0);

		_alloc.deallocate(_data, _size + 1);
		_size = newSize;

		_data = buffer;

		return *this;
	}

	template<typename Type, typename Allocator>
	template<sz N>
	inline constexpr basic_string<Type, Allocator>& basic_string<Type, Allocator>::operator+=(const Type(&data)[N]) noexcept
	{
		if (empty())
		{
			return operator=(data);
		}

		if (N == 0)
		{
			return *this;
		}

		const sz newSize = _size + N;
		const Type* buffer = _alloc.allocate(newSize + 1);
		memcpy(buffer, _data, _size);
		memcpy(buffer + _size, data, N);
		buffer[newSize] = as<Type>(0);

		_alloc.deallocate(_data, _size + 1);
		_size = newSize;

		_data = buffer;

		return *this;
	}

	template<typename Type, typename Allocator>
	inline constexpr Type& basic_string<Type, Allocator>::operator[](sz index) const noexcept
	{
		return _data[index];
	}

	template<typename Type, typename Allocator>
	inline constexpr typename basic_string<Type, Allocator>::iterator basic_string<Type, Allocator>::begin() noexcept
	{
		return _data;
	}

	template<typename Type, typename Allocator>
	inline constexpr typename basic_string<Type, Allocator>::const_iterator basic_string<Type, Allocator>::begin() const noexcept
	{
		return _data;
	}

	template<typename Type, typename Allocator>
	inline constexpr typename basic_string<Type, Allocator>::const_iterator basic_string<Type, Allocator>::cbegin() const noexcept
	{
		return _data;
	}

	template<typename Type, typename Allocator>
	inline constexpr typename basic_string<Type, Allocator>::iterator basic_string<Type, Allocator>::end() noexcept
	{
		return _data + _size;
	}

	template<typename Type, typename Allocator>
	inline constexpr typename basic_string<Type, Allocator>::const_iterator basic_string<Type, Allocator>::end() const noexcept
	{
		return _data + _size;
	}

	template<typename Type, typename Allocator>
	inline constexpr typename basic_string<Type, Allocator>::const_iterator basic_string<Type, Allocator>::cend() const noexcept
	{
		return _data + _size;
	}

	template<typename Type, typename Allocator>
	inline constexpr void basic_string<Type, Allocator>::clear()
	{
		erase(begin(), end());
	}

	template<typename Type, typename Allocator>
	inline constexpr typename basic_string<Type, Allocator>::iterator basic_string<Type, Allocator>::erase(iterator it)
	{
		return erase(it, it+1);
	}

	template<typename Type, typename Allocator>
	inline constexpr typename basic_string<Type, Allocator>::iterator basic_string<Type, Allocator>::erase(iterator start, iterator stop)
	{
		assert((start >= begin() && stop <= end() && start <= stop) && "Range defined by start and/or stop is not contained wholly within this string");

		const sz s = start - begin();
		const sz e = stop - begin();
		const sz count = e - s;

		if (!empty())
		{
			sz size = _size - count;
			
			Type* buffer = _alloc.allocate(size);
			memcpy(buffer, _data, s);
			memcpy(buffer, _data + e, end() - stop);

			_alloc.deallocate(_data, _size + 1);
			_data = buffer;

			_size = size;
		}

		return begin() + s;
	}

	template<typename Type, typename Allocator>
	inline constexpr sz basic_string<Type, Allocator>::length() const noexcept
	{
		return _size;
	}

	template<typename Type, typename Allocator>
	inline constexpr bool basic_string<Type, Allocator>::empty() const noexcept
	{
		return _data == _empty;
	}

	template<typename Type, typename Allocator>
	inline constexpr void basic_string<Type, Allocator>::_set_empty() noexcept
	{
		_data = const_cast<Type*>(_empty);
	}

	template <typename Type, typename Allocator>
	struct hash<basic_string<Type, Allocator>>
	{
		inline constexpr sz operator()(const basic_string<Type, Allocator>& v) const noexcept
		{
			const i32 p = 31, m = 1e9 + 7;
			sz hash_value = 0;

			i64 p_pow = 1;
			const sz n = v.size();
			for (sz i = 0; i < n; ++i)
			{
				hash_value = (hash_value + (v[i] - as<Type>('a') + 1) * p_pow) % m;
				p_pow = (p_pow * p) % m;
			}

			return hash<sz>()(hash_value);
		}
	};

	using string = basic_string<char>;
	using wstring = basic_string<wchar_t>;
}

#endif // string_hpp__
