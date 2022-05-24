#include <gtest/gtest.h>

#include <ryujin/core/filesystem.hpp>
#include <ryujin/core/string.hpp>

using namespace ryujin::filesystem;
using ryujin::string;

TEST(Path, WCharConstructor)
{
	path p = path(L"Test Path");
}
