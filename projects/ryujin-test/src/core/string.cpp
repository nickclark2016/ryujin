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

TEST(String, strcmp)
{
	ASSERT_EQ(ryujin::strcmp("test", "test"), 0);
	ASSERT_EQ(ryujin::strcmp("test", "test2"), -1);
	ASSERT_EQ(ryujin::strcmp("test3", "test"), 1);
}

TEST(String, strstr)
{
	string a("This is a test string");
	
	const char* test = ryujin::strstr(a.c_str(), "test");

	ASSERT_EQ(ryujin::strcmp(test, "test string"), 0);
	ASSERT_NE(ryujin::strcmp(test, "somethingelse"), 0);
}

TEST(String, Contains)
{
	string a("This is a test string");
	ASSERT_TRUE(a.contains("string"));

	ASSERT_FALSE(a.contains("Hello"));
	ASSERT_FALSE(a.contains("String"));

	ASSERT_TRUE(a.contains("is a test"));
}

TEST(String, Find)
{
	string a("This is a test string");
	ASSERT_TRUE(a.find("a test") == 8);

	ASSERT_TRUE(a.find("a test", 0, 4) == 8);

	ASSERT_EQ(a.find('a'), 8);
}

TEST(String, indexof)
{
	string a("This is another test string");
	ASSERT_TRUE(a.first_index_of('i') == 2);
	ASSERT_TRUE(a.last_index_of('r') == 23);

	ASSERT_FALSE(a.first_index_of('T') == 1);
	ASSERT_FALSE(a.last_index_of(' ') == 4);
}