#include <gtest/gtest.h>

#include <ryujin/core/result.hpp>

using ryujin::result;

TEST(Result, FromSuccessReference)
{
	const int success = 3;
	auto res = result<int, char>::from_success(success);

	const bool successful = res;
	ASSERT_TRUE(successful);
	ASSERT_EQ(*res, 3);
}

TEST(Result, FromSuccessMove)
{
	int success = 3;
	auto res = result<int, char>::from_success(std::move(success));

	const bool successful = res;
	ASSERT_TRUE(successful);
	ASSERT_EQ(*res, 3);
}

TEST(Result, FromErrorReference)
{
	const char error = 'a';
	auto res = result<int, char>::from_error(error);

	const bool successful = res;
	ASSERT_FALSE(successful);
	ASSERT_EQ(res.error_code(), 'a');
}

TEST(Result, FromErrorMove)
{
	char error = 'a';
	auto res = result<int, char>::from_error(std::move(error));

	const bool successful = res;
	ASSERT_FALSE(successful);
	ASSERT_EQ(res.error_code(), 'a');
}

TEST(Result, FromSuccessMoveConstructor)
{
	const int success = 3;
	auto res = result<int, char>::from_success(success);

	const bool successful = res;
	ASSERT_TRUE(successful);
	ASSERT_EQ(*res, 3);

	auto res2 = std::move(res);

	const bool successful2 = res2;
	ASSERT_TRUE(successful2);
	ASSERT_EQ(*res2, 3);

	ASSERT_FALSE(res);
}

TEST(Result, FromSuccessMoveAssignment)
{
	const int success = 3;
	auto res = result<int, char>::from_success(success);
	auto res2 = result<int, char>::from_success(4);

	const bool successful = res;
	ASSERT_TRUE(successful);
	ASSERT_EQ(*res, 3);

	res2 = std::move(res);

	const bool successful2 = res2;
	ASSERT_TRUE(successful2);
	ASSERT_EQ(*res2, 3);

	ASSERT_FALSE(res);
}