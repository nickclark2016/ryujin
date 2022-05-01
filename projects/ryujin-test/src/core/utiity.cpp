#include <gtest/gtest.h>

#include <ryujin/core/utility.hpp>

using ryujin::pair;

TEST(Pair, MakeTrivialPair)
{
    pair<int, char> p = ryujin::make_pair(1, 'a');
    const auto& [n, c] = p;
    
    ASSERT_EQ(p.first, n);
    ASSERT_EQ(p.second, c);
    ASSERT_EQ(p.first, 1);
    ASSERT_EQ(p.second, 'a');
}