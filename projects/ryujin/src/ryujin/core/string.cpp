#include <ryujin/core/string.hpp>

#include <clocale>
#include <cstdlib>

void ryujin::convert_string(const char* source, wchar_t* dest, sz length)
{
	std::mbstowcs(dest, source, length);
}

void ryujin::convert_string(const wchar_t* source, wchar_t* dest, sz length)
{
	ryujin::copy(source, source + length, dest);
}

void ryujin::convert_string(const wchar_t* source, char* dest, sz length)
{
	std::wcstombs(dest, source, length);
}

void ryujin::convert_string(const char* source, char* dest, sz length)
{
	ryujin::copy(source, source + length, dest);
}
