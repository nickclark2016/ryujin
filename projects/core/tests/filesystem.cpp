#include <gtest/gtest.h>

#include <ryujin/filesystem.hpp>
#include <ryujin/string.hpp>

using namespace ryujin::filesystem;
using ryujin::string;

TEST(Path, WCharConstructor)
{
	path p = path(L"Test Path");
}
