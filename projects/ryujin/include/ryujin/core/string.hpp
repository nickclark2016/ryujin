#ifndef string_hpp__
#define string_hpp__

#include "algorithm.hpp"
#include "allocator.hpp"
#include "as.hpp"
#include "memory.hpp"
#include "primitives.hpp"
#include "utility.hpp"
#include "vector.hpp"

#include <cassert>
#include <memory>

/// TODO: Capacity opt
/// TODO: replace, reserve, rfind, split

namespace ryujin
{
	void convert_string(const char* source, wchar_t* dest, sz length);
	void convert_string(const wchar_t* source, char* dest, sz length);
	void convert_string(const char* source, char* dest, sz length);
	void convert_string(const wchar_t* source, wchar_t* dest, sz length);

	template<typename Type, typename Allocator = allocator<Type>>
	class basic_string
	{
	public:
		using iterator = Type*;
		using const_iterator = const Type*;

		constexpr basic_string();
		constexpr ~basic_string();
		explicit constexpr basic_string(sz size) noexcept;

		constexpr basic_string(const basic_string& s) noexcept;
		constexpr basic_string(basic_string&& s) noexcept;

		constexpr basic_string(const Type* data) noexcept;
		constexpr basic_string(const Type* data, sz size) noexcept;

		template<sz N>
		constexpr basic_string(const Type(&data)[N]) noexcept;

		constexpr bool operator==(const basic_string& rhs) const noexcept;
		constexpr bool operator!=(const basic_string& rhs) const noexcept;

		constexpr bool operator==(const Type* rhs) const noexcept;
		constexpr bool operator!=(const Type* rhs) const noexcept;

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

		constexpr basic_string& replace(sz pos, sz len, const Type c);
		constexpr basic_string& replace(sz pos, sz len, const Type* str);
		constexpr basic_string& replace(sz pos, sz len, const basic_string& str);
		constexpr basic_string& replace(iterator start, iterator end, const Type c);
		constexpr basic_string& replace(iterator start, iterator end, const Type* str);
		constexpr basic_string& replace(iterator start, iterator end, const basic_string& str);

		constexpr basic_string& insert(sz pos, const basic_string& str);
		constexpr basic_string& insert(sz pos, const Type* s);
		constexpr basic_string& insert(sz pos, const Type* s, sz n);
		constexpr basic_string& insert(sz pos, sz n, Type c);
		constexpr void insert(iterator p, sz n, Type c);
		constexpr iterator insert(iterator p, Type c);

		constexpr void clear();
		constexpr iterator erase(iterator it);
		constexpr iterator erase(iterator start, iterator stop);

		constexpr sz length() const noexcept;
		constexpr bool empty() const noexcept;

		constexpr const Type* c_str() const noexcept;

		constexpr Type* data() noexcept;
		constexpr const Type* data() const noexcept;

		constexpr static sz npos = (~(sz)0);

		constexpr basic_string substr(sz pos = 0, sz len = npos);

		constexpr sz first_index_of(Type token) const noexcept;
		constexpr sz first_index_of_any(vector<Type> tokens) const noexcept;
		constexpr sz first_index_of_any(const Type* tokens) const noexcept;
		constexpr sz last_index_of(Type token) const noexcept;
		constexpr sz last_index_of_any(vector<Type> tokens) const noexcept;
		constexpr sz last_index_of_any(const Type* tokens) const noexcept;

		constexpr sz find(const basic_string& str, sz pos = 0) const noexcept;
		constexpr sz find(const Type* str, sz pos = 0) const noexcept;
		constexpr sz find(const Type* str, sz pos, sz n) const noexcept;

		constexpr sz rfind(const basic_string& str, sz pos = npos) const noexcept;
		constexpr sz rfind(const Type* str, sz pos = npos) const;
		constexpr sz rfind(const Type* str, sz pos, sz n) const;

		constexpr bool contains(const Type token) const noexcept;
		constexpr bool contains(const Type* str) const noexcept;
		constexpr bool contains(const basic_string& str) const noexcept;

		constexpr bool starts_with(const Type token) const noexcept;
		constexpr bool starts_with(const Type* str) const noexcept;
		constexpr bool starts_with(const basic_string& str) const noexcept;

		constexpr bool ends_with(const Type token) const noexcept;
		constexpr bool ends_with(const Type* str) const noexcept;
		constexpr bool ends_with(const basic_string& str) const noexcept;

	private:
		Type* _data = nullptr;
		sz _size = 0;

		Allocator _alloc;

		static constexpr Type _empty[1] = { 0 };

		constexpr void _set_empty() noexcept;
	};

	namespace detail
	{
		template<typename T>
		inline T* CharTypeAssignN(T* pDestination, size_t n, T c)
		{
			T* pDest = pDestination;
			const T* const pEnd = pDestination + n;
			while (pDest < pEnd)
				*pDest++ = c;
			return pDestination;
		}

		template <typename Type, typename Allocator>
		const typename basic_string<Type, Allocator>::value_type* CharTypeStringFindEnd(const typename basic_string<Type, Allocator>::value_type* pBegin, const typename basic_string<Type, Allocator>::value_type* pEnd, typename basic_string<Type, Allocator>::value_type c)
		{
			const typename basic_string<Type, Allocator>::value_type* pTemp = pEnd;
			while (--pTemp >= pBegin)
			{
				if (*pTemp == c)
					return pTemp;
			}

			return pEnd;
		}

		template <typename Type, typename Allocator>
		const typename basic_string<Type, Allocator>::value_type*
			CharTypeStringRSearch(const typename basic_string<Type, Allocator>::value_type* p1Begin, const typename basic_string<Type, Allocator>::value_type* p1End,
				const typename basic_string<Type, Allocator>::value_type* p2Begin, const typename basic_string<Type, Allocator>::value_type* p2End)
		{
			// Test for zero length strings, in which case we have a match or a failure,
			// but the return value is the same either way.
			if ((p1Begin == p1End) || (p2Begin == p2End))
				return p1Begin;

			// Test for a pattern of length 1.
			if ((p2Begin + 1) == p2End)
				return CharTypeStringFindEnd(p1Begin, p1End, *p2Begin);

			// Test for search string length being longer than string length.
			if ((p2End - p2Begin) > (p1End - p1Begin))
				return p1End;

			// General case.
			const typename basic_string<Type, Allocator>::value_type* pSearchEnd = (p1End - (p2End - p2Begin) + 1);
			const typename basic_string<Type, Allocator>::value_type* pCurrent1;
			const typename basic_string<Type, Allocator>::value_type* pCurrent2;

			while (pSearchEnd != p1Begin)
			{
				// Search for the last occurrence of *p2Begin.
				pCurrent1 = CharTypeStringFindEnd(p1Begin, pSearchEnd, *p2Begin);
				if (pCurrent1 == pSearchEnd) // If the first char of p2 wasn't found,
					return p1End;           // then we immediately have failure.

				// In this case, *pTemp == *p2Begin. So compare the rest.
				pCurrent2 = p2Begin;
				while (*pCurrent1++ == *pCurrent2++)
				{
					if (pCurrent2 == p2End)
						return (pCurrent1 - (p2End - p2Begin));
				}

				// A smarter algorithm might know to subtract more than just one,
				// but in most cases it won't make much difference anyway.
				--pSearchEnd;
			}

			return p1End;
		}
	}

	template<typename Type>
	inline constexpr sz strlen(const Type* data) noexcept
	{
		if (data == nullptr)
		{
			return 0;
		}

		sz i = 0;
		while (data[i] != as<Type>(0)) ++i;
		return i;
	}

	template<typename Type>
	inline constexpr i32 strcmp(const Type* lhs, const Type* rhs) noexcept
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

	template<typename Type>
	inline constexpr const Type* strstr(const Type* lhs, sz lhsSize, const Type* rhs, sz rhsSize) noexcept
	{
		// RHS is empty or null
		if (rhsSize == 0 || *rhs == as<Type>(0))
		{
			return lhs;
		}

		// LHS is null or LHS length is less than RHS
		if (rhsSize > lhsSize || *lhs == as<Type>(0))
		{
			return nullptr;
		}

		// next stores the index of the next best partial match
		auto next = ryujin::make_unique<sz[]>(rhsSize + 1);
		ryujin::memset(next.get(), 0, (rhsSize + 1) * sizeof(sz));

		for (sz i = 1; i < rhsSize; i++)
		{
			sz j = next[i + 1];

			while (j > 0 && rhs[j] != rhs[i])
			{
				j = next[j];
			}

			if (j > 0 || rhs[j] == rhs[i])
			{
				next[i + 1] = j + 1;
			}
		}

		for (sz i = 0, j = 0; i < lhsSize; i++)
		{
			if (*(lhs + i) == *(rhs + j))
			{
				if (++j == rhsSize)
				{
					return (lhs + i - j + 1);
				}
			}
			else if (j > 0)
			{
				j = next[j];
				i--;
			}
		}

		return nullptr;
	}

	template<typename Type>
	inline constexpr const Type* strstr(const Type* lhs, const Type* rhs) noexcept
	{
		sz lhsSize = lhs != nullptr ? ryujin::strlen(lhs) : 0;
		sz rhsSize = rhs != nullptr ? ryujin::strlen(rhs) : 0;

		return ryujin::strstr(lhs, lhsSize, rhs, rhsSize);
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
			ryujin::copy(s._data, s._data + s._size, _data);
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
			ryujin::memcpy(_data, data, _size);
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
			ryujin::memcpy(_data, data, size);
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
			ryujin::memcpy(_data, data, _size);
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
	inline constexpr bool basic_string<Type, Allocator>::operator==(const Type* rhs) const noexcept
	{
		return ryujin::strcmp(_data, rhs) == 0;
	}

	template<typename Type, typename Allocator>
	inline constexpr bool basic_string<Type, Allocator>::operator!=(const Type* rhs) const noexcept
	{
		return ryujin::strcmp(_data, rhs) != 0;
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
			ryujin::memcpy(_data, rhs._data, _size);
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
			ryujin::memcpy(_data, data, _size);
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
			ryujin::memcpy(_data, data, _size);
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
		ryujin::memcpy(buffer, _data, _size);
		ryujin::memcpy(buffer + _size, rhs._data, rhs._size);
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
		ryujin::memcpy(buffer, _data, _size);
		ryujin::memcpy(buffer + _size, rhs, rhsSize);
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
		ryujin::memcpy(buffer, _data, _size);
		ryujin::memcpy(buffer + _size, data, N);
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
	inline constexpr basic_string<Type, Allocator>& basic_string<Type, Allocator>::replace(sz pos, sz len, const Type c)
	{
		if (len == 1)
		{
			_data[pos] = c;
			return *this;
		}

		const sz newLen = _size - len + 1;
		const Type* buffer = _alloc.allocate(newLen + 1);
		ryujin::copy(_data, _data + pos, buffer);
		buffer[pos] = c;
		ryujin::copy(_data + pos + len, _data + _size, buffer + pos + 1);
		buffer[newLen] = as<Type>(0);

		_alloc.deallocate(_data, _size + 1);
		
		_data = buffer;
		_size = newLen;

		return *this;
	}

	template<typename Type, typename Allocator>
	inline constexpr basic_string<Type, Allocator>& basic_string<Type, Allocator>::replace(sz pos, sz len, const Type* str)
	{
		const sz strLen = ryujin::strlen(str);

		const sz newLen = _size - len + strLen;
		const Type* buffer = _alloc.allocate(newLen + 1);
		ryujin::copy(_data, _data + pos, buffer);
		ryujin::copy(str, str + strLen, buffer + pos);
		ryujin::copy(_data + pos + len, _data + _size, buffer + pos + strLen);
		_data[newLen] = as<Type>(0);

		_alloc.deallocate(_data, _size + 1);
		
		_data = buffer;
		_size = newLen;

		return *this;
	}

	template<typename Type, typename Allocator>
	inline constexpr basic_string<Type, Allocator>& basic_string<Type, Allocator>::replace(sz pos, sz len, const basic_string& str)
	{
		const sz strLen = str.length();

		const sz newLen = _size - len + strLen;
		const Type* buffer = _alloc.allocate(newLen + 1);
		ryujin::copy(_data, _data + pos, buffer);
		ryujin::copy(str.c_str(), str.c_str() + strLen, buffer + pos);
		ryujin::copy(_data + pos + len, _data + _size, buffer + pos + strLen);
		_data[newLen] = as<Type>(0);

		_alloc.deallocate(_data, _size + 1);

		_data = buffer;
		_size = newLen;

		return *this;
	}

	template<typename Type, typename Allocator>
	inline constexpr basic_string<Type, Allocator>& basic_string<Type, Allocator>::replace(iterator start, iterator end, const Type c)
	{
		const sz size = as<sz>(end - start);
		return replace(begin() - start, size, c);
	}

	template<typename Type, typename Allocator>
	inline constexpr basic_string<Type, Allocator>& basic_string<Type, Allocator>::replace(iterator start, iterator end, const Type* str)
	{
		const sz size = as<sz>(end - start);
		return replace(begin() - start, size, str);
	}

	template<typename Type, typename Allocator>
	inline constexpr basic_string<Type, Allocator>& basic_string<Type, Allocator>::replace(iterator start, iterator end, const basic_string& str)
	{
		return replace(start, end, str._data);
	}

	template<typename Type, typename Allocator>
	inline constexpr basic_string<Type, Allocator>& basic_string<Type, Allocator>::insert(sz pos, const basic_string& str)
	{
		return insert(pos, str._data, str._size);
	}

	template<typename Type, typename Allocator>
	inline constexpr basic_string<Type, Allocator>& basic_string<Type, Allocator>::insert(sz pos, const Type* s)
	{
		return insert(pos, s, strlen(s));
	}

	template<typename Type, typename Allocator>
	inline constexpr basic_string<Type, Allocator>& basic_string<Type, Allocator>::insert(sz pos, const Type* s, sz n)
	{
		if (s == nullptr || n == 0)
		{
			return *this;
		}

		if (empty())
		{
			return operator=(s);
		}

		const sz newSize = n + _size;
		const Type* buffer = _alloc.allocate(newSize + 1);
		ryujin::memcpy(buffer, _data, pos);
		ryujin::memcpy(buffer, s, n);
		ryujin::memcpy(buffer, _data + (pos - n), (_size - pos));
		buffer[newSize] = as<Type>(0);

		_alloc.deallocate(_data, _size + 1);
		_size = newSize;

		_data = buffer;

		return *this;
	}

	template<typename Type, typename Allocator>
	inline constexpr basic_string<Type, Allocator>& basic_string<Type, Allocator>::insert(sz pos, sz n, Type c)
	{
		assert(false && "Needs implementation");
		return *this;
	}

	template<typename Type, typename Allocator>
	inline constexpr void basic_string<Type, Allocator>::insert(iterator p, sz n, Type c)
	{
		assert(false && "Needs implementation");
	}

	template<typename Type, typename Allocator>
	inline constexpr basic_string<Type, Allocator>::iterator basic_string<Type, Allocator>::insert(iterator p, Type c)
	{
		assert(false && "Needs implementation");
		return _data;
	}

	template<typename Type, typename Allocator>
	inline constexpr void basic_string<Type, Allocator>::clear()
	{
		erase(begin(), end());
	}

	template<typename Type, typename Allocator>
	inline constexpr typename basic_string<Type, Allocator>::iterator basic_string<Type, Allocator>::erase(iterator it)
	{
		return erase(it, it + 1);
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
			ryujin::memcpy(buffer, _data, s);
			ryujin::memcpy(buffer, _data + e, end() - stop);

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
	constexpr const Type* basic_string<Type, Allocator>::c_str() const noexcept
	{
		return _data;
	}

	template<typename Type, typename Allocator>
	constexpr Type* basic_string<Type, Allocator>::data() noexcept
	{
		return _data;
	}

	template<typename Type, typename Allocator>
	constexpr const Type* basic_string<Type, Allocator>::data() const noexcept
	{
		return _data;
	}

	template<typename Type, typename Allocator>
	inline constexpr basic_string<Type, Allocator> basic_string<Type, Allocator>::substr(sz pos, sz len)
	{
		if (len == npos)
		{
			len = _size - pos + 1;
		}

		basic_string str(len);

		ryujin::memcpy(str._data, _data + pos, len * sizeof(Type));
		str._data[len] = as<Type>(0);

		return str;
	}

	template<typename Type, typename Allocator>
	inline constexpr sz basic_string<Type, Allocator>::first_index_of(Type token) const noexcept
	{
		sz i = 0;
		while (_data[i] != token) ++i;
		return i < _size ? i : npos;
	}

	template<typename Type, typename Allocator>
	inline constexpr sz basic_string<Type, Allocator>::first_index_of_any(vector<Type> tokens) const noexcept
	{
		sz lowestIndex = npos;

		for (Type token : tokens)
		{
			sz tokenIndex = first_index_of(token);
			lowestIndex = ryujin::min(tokenIndex, lowestIndex);
		}

		return lowestIndex;
	}

	template<typename Type, typename Allocator>
	inline constexpr sz basic_string<Type, Allocator>::first_index_of_any(const Type* tokens) const noexcept
	{
		sz lowestIndex = npos;
		sz tokensLen = ryujin::strlen(tokens);

		for (sz i = 0; i < tokensLen; i++)
		{
			sz tokenIndex = first_index_of(tokens[i]);
			lowestIndex = ryujin::min(tokenIndex, lowestIndex);
		}

		return lowestIndex;
	}

	template<typename Type, typename Allocator>
	inline constexpr sz basic_string<Type, Allocator>::last_index_of(Type token) const noexcept
	{
		sz i = _size - 1;
		while (_data[i] != token) --i;
		return i < _size ? i : npos;
	}

	template<typename Type, typename Allocator>
	inline constexpr sz basic_string<Type, Allocator>::last_index_of_any(vector<Type> tokens) const noexcept
	{
		sz highestIndex = 0;

		for (Type token : tokens)
		{
			sz tokenIndex = last_index_of(token);
			highestIndex = ryujin::max(tokenIndex, highestIndex);
		}

		return highestIndex;
	}

	template<typename Type, typename Allocator>
	inline constexpr sz basic_string<Type, Allocator>::last_index_of_any(const Type* tokens) const noexcept
	{
		sz highestIndex = 0;
		sz tokensLen = ryujin::strlen(tokens);

		for (sz i = 0; i < tokensLen; i++)
		{
			sz tokenIndex = last_index_of(tokens[i]);
			highestIndex = ryujin::max(tokenIndex, highestIndex);
		}

		return highestIndex;
	}

	template<typename Type, typename Allocator>
	inline constexpr sz basic_string<Type, Allocator>::find(const basic_string& str, sz pos) const noexcept
	{
		return find(str.c_str(), pos);
	}

	template<typename Type, typename Allocator>
	inline constexpr sz basic_string<Type, Allocator>::find(const Type* str, sz pos) const noexcept
	{
		if (pos >= _size)
		{
			return npos;
		}

		const Type* subresult = ryujin::strstr(_data + pos, str);
		if (subresult != nullptr)
		{
			return (_size - strlen(subresult));
		}
		else
		{
			return npos;
		}
	}

	template<typename Type, typename Allocator>
	inline constexpr sz basic_string<Type, Allocator>::find(const Type* str, sz pos, sz n) const noexcept
	{
		if (pos >= _size)
		{
			return npos;
		}

		const Type* subresult = ryujin::strstr(_data + pos, _size, str, n);
		if (subresult != nullptr)
		{
			return (_size - strlen(subresult));
		}
		else
		{
			return npos;
		}
	}

	template<typename Type, typename Allocator>
	inline constexpr sz basic_string<Type, Allocator>::rfind(const basic_string& str, sz pos) const noexcept
	{
		return rfind(str._data, pos, str._size);
	}

	template<typename Type, typename Allocator>
	inline constexpr sz basic_string<Type, Allocator>::rfind(const Type* str, sz pos) const
	{
		return rfind(str, pos, strlen(str));
	}

	template<typename Type, typename Allocator>
	inline constexpr sz basic_string<Type, Allocator>::rfind(const Type* str, sz pos, sz n) const
	{
		if (n <= _size)
		{
			if (n)
			{
				const const_iterator pEnd = _data + ryujin::min(_size - n, pos) + n;
				const const_iterator pResult = detail::CharTypeStringRSearch(begin(), pEnd, str, str + n);

				if (pResult != pEnd)
				{
					return as<sz>(pResult - begin());
				}
			}
			else
			{
				return ryujin::min(_size, pos);
			}
		}
	}

	template<typename Type, typename Allocator>
	inline constexpr bool basic_string<Type, Allocator>::contains(const Type token) const noexcept
	{
		return first_index_of(token) != npos;
	}

	template<typename Type, typename Allocator>
	inline constexpr bool basic_string<Type, Allocator>::contains(const Type* str) const noexcept
	{
		return find(str) != npos;
	}

	template<typename Type, typename Allocator>
	inline constexpr bool basic_string<Type, Allocator>::contains(const basic_string& str) const noexcept
	{
		return contains(str._data);
	}

	template<typename Type, typename Allocator>
	inline constexpr bool basic_string<Type, Allocator>::starts_with(const Type token) const noexcept
	{
		return first_index_of(token) == 0;
	}

	template<typename Type, typename Allocator>
	inline constexpr bool basic_string<Type, Allocator>::starts_with(const Type* str) const noexcept
	{
		return find(str) == 0;
	}

	template<typename Type, typename Allocator>
	inline constexpr bool basic_string<Type, Allocator>::starts_with(const basic_string& str) const noexcept
	{
		return find(str) == 0;
	}

	template<typename Type, typename Allocator>
	inline constexpr bool basic_string<Type, Allocator>::ends_with(const Type token) const noexcept
	{
		return last_index_of(token) == _size - 1;
	}

	template<typename Type, typename Allocator>
	inline constexpr bool basic_string<Type, Allocator>::ends_with(const Type* str) const noexcept
	{
		// TODO: Requires rfind
		return false;
	}

	template<typename Type, typename Allocator>
	inline constexpr bool basic_string<Type, Allocator>::ends_with(const basic_string& str) const noexcept
	{
		// TODO: Requires rfind
		return false;
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
			const i32 p = 31, m = static_cast<i32>(1e9) + 7;
			sz hash_value = 0;

			i64 p_pow = 1;
			const sz n = v.length();
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
