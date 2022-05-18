#include <gtest/gtest.h>

#include <ryujin/core/string.hpp>

using ryujin::string;
using ryujin::wstring;

TEST(String, DefaultConstructor)
{
	string a;
	wstring b;

	ASSERT_EQ(a.length(), 0);
	ASSERT_EQ(b.length(), 0);
}

TEST(String, PointerConstructor)
{
	string a("BigBoiTest");
	wstring b(L"BigBoiTest");

	ASSERT_EQ(a.length(), 10);
	ASSERT_EQ(b.length(), 10);
}

TEST(String, EqualsComparison)
{
	string a("test");
	string b("test");
	string c("test2");

	ASSERT_EQ(a, b);
	ASSERT_NE(a, c);

	ASSERT_TRUE(a == "test");
	ASSERT_TRUE(c == "test2");

	ASSERT_FALSE(a == "test2");
}

TEST(String, substr)
{
	string a("Hello World");

	string hello = a.substr(0, 5);
	string world = a.substr(6);

	ASSERT_EQ(hello, "Hello");
	ASSERT_EQ(world, "World");
}