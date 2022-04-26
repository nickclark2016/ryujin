#include <gtest/gtest.h>

#include <ryujin/core/functional.hpp>

using ryujin::no_move_function;

TEST(NoMoveFunction, SimpleInvokeFromLambdaByRValueReference)
{
    no_move_function<int(int, int)> sum = [](int a, int b) { return a + b; };
    int expected = 3;
    int actual = sum(1, 2);
    ASSERT_EQ(actual, expected);
}

TEST(NoMoveFunction, SimpleInvokeFromLambdaByLValueReference)
{
    no_move_function<int(int, int)> sum = [](int a, int b) { return a + b; };
    int a = 1;
    int b = 2;
    int expected = 3;
    int actual = sum(a, b);
    ASSERT_EQ(actual, expected);
}

TEST(NoMoveFunction, SimpleInvokeFromMovedLambdaByLValueReference)
{
    no_move_function<int(int, int)> src = [](int a, int b) { return a + b; };
    no_move_function<int(int, int)> sum = ryujin::move(src);
    int a = 1;
    int b = 2;
    int expected = 3;
    int actual = sum(a, b);
    ASSERT_EQ(actual, expected);
}