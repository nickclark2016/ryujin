#include <gtest/gtest.h>

#include <ryujin/core/functional.hpp>

using ryujin::move_only_function;

struct InvocableType
{
    inline int operator()(int a, int b)
    {
        return a + b;
    }
};

namespace invocable
{
    struct Invocable
    {
        int sum(int a, int b)
        {
            return a + b;
        }

        move_only_function<int(int, int)> sum_fn()
        {
            return move_only_function([](int a, int b) { return a + b; });
        }
    };

    int sum(int a, int b)
    {
        return a + b;
    }
}

TEST(MoveOnlyFunction, SimpleInvokeFromLambdaByRValueReference)
{
    move_only_function<int(int, int)> sum = [](int a, int b) { return a + b; };
    int expected = 3;
    int actual = sum(1, 2);
    ASSERT_EQ(actual, expected);
}

TEST(MoveOnlyFunction, SimpleInvokeFromLambdaByLValueReference)
{
    move_only_function<int(int, int)> sum = [](int a, int b) { return a + b; };
    int a = 1;
    int b = 2;
    int expected = 3;
    int actual = sum(a, b);
    ASSERT_EQ(actual, expected);
}

TEST(MoveOnlyFunction, SimpleInvokeFromMovedLambdaByLValueReference)
{
    move_only_function<int(int, int)> src = [](int a, int b) { return a + b; };
    move_only_function<int(int, int)> sum = ryujin::move(src);
    int a = 1;
    int b = 2;
    int expected = 3;
    int actual = sum(a, b);
    ASSERT_EQ(actual, expected);
}

TEST(MoveOnlyFunction, InvokeFromObject)
{
    move_only_function<int(int, int)> sum = InvocableType();
    int expected = 3;
    ASSERT_EQ(sum(1, 2), expected);
}

TEST(MoveOnlyFunction, InvokeFromReturnedFunction)
{
    invocable::Invocable instance;
    auto sum = instance.sum_fn();
    int expected = 3;
    ASSERT_EQ(sum(1, 2), 3);
}

TEST(MoveOnlyFunction, InvokeFromMoveConstructedFunction)
{
    move_only_function<int(int, int)> src = [](int a, int b) { return a + b; };
    move_only_function<int(int, int)> dst(ryujin::move(src));
    int expected = 3;
    ASSERT_EQ(dst(1, 2), 3);
    ASSERT_TRUE(dst);
    ASSERT_FALSE(src);
}

TEST(MoveOnlyFunction, InvokeFromMoveAssignedFunction)
{
    move_only_function<int(int, int)> src = [](int a, int b) { return a + b; };
    move_only_function<int(int, int)> dst = [](int a, int b) -> int { return 0; };
    dst = ryujin::move(src);
    int expected = 3;
    ASSERT_EQ(dst(1, 2), 3);
    ASSERT_TRUE(dst);
    ASSERT_FALSE(src);
}

TEST(MoveOnlyFunction, TemplateDeductionFromFreeFunction)
{
    move_only_function sum = &invocable::sum;
    int expected = 3;
    ASSERT_EQ(sum(1, 2), expected);
}

TEST(MoveOnlyFunction, TemplateDeductionFromMemberFunction)
{
    move_only_function sum = &invocable::Invocable::sum;
    int expected = 3;
    invocable::Invocable instance;
    ASSERT_EQ(sum(instance, 1, 2), expected);
}

TEST(MoveOnlyFunction, TemplateDeductionFromLambda)
{
    move_only_function sum = [](int a, int b) { return a + b; };
    int expected = 3;
    ASSERT_EQ(sum(1, 2), expected);
}