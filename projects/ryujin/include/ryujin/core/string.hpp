#ifndef string_hpp__
#define string_hpp__

#include <memory>

#include "primitives.hpp"
#include "utility.hpp"

namespace ryujin
{
	template <typename Type, typename Allocator = std::allocator<Type>>
	class basic_string
	{
		public:
			constexpr basic_string() = default;
			constexpr ~basic_string();

			explicit constexpr basic_string(sz size) noexcept;

			explicit constexpr basic_string(const basic_string& s) noexcept;
			constexpr basic_string(basic_string&& s) noexcept;

			constexpr basic_string(const Type* data) noexcept;
			constexpr basic_string(const Type* data, sz size) noexcept;
			
			template<sz N>
			constexpr basic_string(const Type(&data)[N]) noexcept;

			constexpr static sz strlen(const Type* data) noexcept;
			constexpr static bool strcmp(const basic_string& lhs, const basic_string& rhs) noexcept;

			constexpr bool operator==(const basic_string& rhs) const noexcept;
			constexpr bool operator!=(const basic_string& rhs) const noexcept;

			constexpr basic_string& operator=(const basic_string& rhs) noexcept;
			constexpr basic_string& operator=(basic_string&& rhs) noexcept;

		private:
			Type* _data = nullptr;
			sz _size = 0;

			Allocator _alloc;
	};

	template <typename Type, typename Allocator>
	inline constexpr basic_string<Type, Allocator>::~basic_string()
	{
		delete _data;
	}

	template <typename Type, typename Allocator>
	inline constexpr basic_string<Type, Allocator>::basic_string(const sz size) noexcept : _size(size)
	{
		_data = _alloc.allocate(_size);
	}

	template <typename Type, typename Allocator>
	inline constexpr basic_string<Type, Allocator>::basic_string(const basic_string& s) noexcept
	{
		_size = s._size;
		memcpy(_data, s._data, _size);
	}

	template <typename Type, typename Allocator>
	constexpr basic_string<Type, Allocator>::basic_string(basic_string&& s) noexcept
	{
		_size = s._size;
		_data = move(s._data);
		_alloc = move(s._alloc);
	}

	template<typename Type, typename Allocator>
	inline constexpr basic_string<Type, Allocator>::basic_string(const Type* data) noexcept
	{
		_size = strlen(data);

		_data = _alloc.allocate(_size);
		memcpy(_data, data, _size);
	}

	template <typename Type, typename Allocator>
	inline constexpr basic_string<Type, Allocator>::basic_string(const Type* data, const sz size) noexcept : _size(size)
	{
		_data = _alloc.allocate(size);
		memcpy(_data, data, size);
	}

	template<typename Type, typename Allocator>
	inline constexpr sz basic_string<Type, Allocator>::strlen(const Type* data) noexcept
	{
		sz i = 0;
		while (data[i] != 0) ++i;
		return i;
	}

	template<typename Type, typename Allocator>
	inline constexpr bool basic_string<Type, Allocator>::strcmp(const basic_string& lhs, const basic_string& rhs) noexcept
	{
		i32 index = 0;
		i32 size = strlen(lhs);
		if (size != strlen(rhs))
			return false;

		while (index < size)
		{
			if (lhs._data[index] != rhs._data[index++])
			{
				return false;
			}
		}

		return true;
	}

	template<typename Type, typename Allocator>
	inline constexpr bool basic_string<Type, Allocator>::operator==(const basic_string& rhs) const noexcept
	{
		return strcmp(*this, rhs);
	}

	template<typename Type, typename Allocator>
	inline constexpr bool basic_string<Type, Allocator>::operator!=(const basic_string& rhs) const noexcept
	{
		return !strcmp(*this, rhs);
	}

	template<typename Type, typename Allocator>
	inline constexpr basic_string<Type, Allocator>& basic_string<Type, Allocator>::operator=(const basic_string<Type, Allocator>& rhs) noexcept
	{
		delete _data;

		_size = rhs._size;

		_data = _alloc.allocate(_size);
		memcpy(_data, rhs._data, _size);
	}

	template<typename Type, typename Allocator>
	inline constexpr basic_string<Type, Allocator>& basic_string<Type, Allocator>::operator=(basic_string<Type, Allocator>&& rhs) noexcept
	{
		delete _data;

		_size = rhs._size;
		_data = move(rhs._data);
		_alloc = move(rhs._alloc);
	}
	
	template<typename Type, typename Allocator>
	template<sz N>
	inline constexpr basic_string<Type, Allocator>::basic_string(const Type(&data)[N]) noexcept
	{
		_size = sizeof(data);

		_data = _alloc.allocate(_size);
		memcpy(_data, data, _size);
	}

	using string = basic_string<char>;
	using wstring = basic_string<wchar_t>;
}

#endif // string_hpp__
