#include <gtest/gtest.h>

#include <ryujin/core/functional.hpp>

using ryujin::move_only_function;

namespace invocable
{
    struct Invocable
    {
        int sum(int a, int b)
        {
            return a + b;
        }

        move_only_function<int(int, int) const> sum_fn()
        {
            return move_only_function([](int a, int b) { return a + b; });
        }

        int operator()(int a, int b)
        {
            return sum(a, b);
        }
    };

    int sum(int a, int b)
    {
        return a + b;
    }
}

namespace invocable_noexcept
{
    struct Invocable
    {
        int sum(int a, int b) noexcept
        {
            return a + b;
        }

        move_only_function<int(int, int) const noexcept> sum_fn()
        {
            return move_only_function([](int a, int b) noexcept { return a + b; });
        }

        int operator()(int a, int b) noexcept
        {
            return sum(a, b);
        }
    };

    int sum(int a, int b) noexcept
    {
        return a + b;
    }
}

TEST(MoveOnlyFunction, SimpleInvokeFromLambdaByRValueReference)
{
    move_only_function<int(int, int) const> sum([](int a, int b) { return a + b; });
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
    move_only_function<int(int, int)> sum = invocable::Invocable();
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
    static_assert(ryujin::is_member_function_pointer<decltype(& invocable::Invocable::sum)>::value, "Not a member function pointer");
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

TEST(MoveOnlyFunctionNoexcept, SimpleInvokeFromLambdaByRValueReference)
{
    move_only_function<int(int, int)> sum = [](int a, int b) { return a + b; };
    int expected = 3;
    int actual = sum(1, 2);
    ASSERT_EQ(actual, expected);
}

TEST(MoveOnlyFunctionNoexcept, SimpleInvokeFromLambdaByLValueReference)
{
    move_only_function<int(int, int)> sum = [](int a, int b) { return a + b; };
    int a = 1;
    int b = 2;
    int expected = 3;
    int actual = sum(a, b);
    ASSERT_EQ(actual, expected);
}

TEST(MoveOnlyFunctionNoexcept, SimpleInvokeFromMovedLambdaByLValueReference)
{
    move_only_function<int(int, int)> src = [](int a, int b) noexcept { return a + b; };
    move_only_function<int(int, int)> sum = ryujin::move(src);
    int a = 1;
    int b = 2;
    int expected = 3;
    int actual = sum(a, b);
    ASSERT_EQ(actual, expected);
}

TEST(MoveOnlyFunctionNoexcept, InvokeFromObject)
{
    move_only_function<int(int, int)> sum = invocable_noexcept::Invocable();
    int expected = 3;
    ASSERT_EQ(sum(1, 2), expected);
}

TEST(MoveOnlyFunctionNoexcept, InvokeFromReturnedFunction)
{
    invocable_noexcept::Invocable instance;
    auto sum = instance.sum_fn();
    int expected = 3;
    ASSERT_EQ(sum(1, 2), 3);
}

TEST(MoveOnlyFunctionNoexcept, InvokeFromMoveConstructedFunction)
{
    move_only_function<int(int, int) noexcept> src = [](int a, int b) noexcept { return a + b; };
    move_only_function<int(int, int) noexcept> dst(ryujin::move(src));
    int expected = 3;
    ASSERT_EQ(dst(1, 2), 3);
    ASSERT_TRUE(dst);
    ASSERT_FALSE(src);
}

TEST(MoveOnlyFunctionNoexcept, InvokeFromMoveAssignedFunction)
{
    move_only_function<int(int, int) noexcept> src = [](int a, int b) noexcept { return a + b; };
    move_only_function<int(int, int) noexcept> dst = [](int a, int b) noexcept -> int { return 0; };
    dst = ryujin::move(src);
    int expected = 3;
    ASSERT_EQ(dst(1, 2), 3);
    ASSERT_TRUE(dst);
    ASSERT_FALSE(src);
}

TEST(MoveOnlyFunctionNoexcept, TemplateDeductionFromFreeFunction)
{
    move_only_function sum = &invocable_noexcept::sum;
    int expected = 3;
    ASSERT_EQ(sum(1, 2), expected);
}

TEST(MoveOnlyFunctionNoexcept, TemplateDeductionFromMemberFunction)
{
    move_only_function sum = &invocable_noexcept::Invocable::sum;
    int expected = 3;
    invocable_noexcept::Invocable instance;
    ASSERT_EQ(sum(instance, 1, 2), expected);
}

TEST(MoveOnlyFunctionNoexcept, TemplateDeductionFromLambda)
{
    move_only_function sum = [](int a, int b) noexcept { return a + b; };
    int expected = 3;
    ASSERT_EQ(sum(1, 2), expected);
}
