#include <gtest/gtest.h>

#include <ryujin/scheduler/task.hpp>

using ryujin::task;

TEST(Task, ExecuteTaskWithReturnType)
{
    task<int(int, int)> t([](int a, int b) { return a + b; }, 1, 2);
    t.execute();
    
    ASSERT_EQ(t.get_result(), 3);
    
    constexpr bool isExpectedReturnType = ryujin::is_same_v<int, typename task<int(int, int)>::result>;

    ASSERT_TRUE(isExpectedReturnType);
}