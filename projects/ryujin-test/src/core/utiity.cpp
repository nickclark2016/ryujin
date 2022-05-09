#include <gtest/gtest.h>

#include <ryujin/core/utility.hpp>

using ryujin::pair;
using ryujin::tuple;

TEST(Pair, MakeTrivialPair)
{
    pair<int, char> p = ryujin::make_pair(1, 'a');
    const auto& [n, c] = p;
    
    ASSERT_EQ(p.first, n);
    ASSERT_EQ(p.second, c);
    ASSERT_EQ(p.first, 1);
    ASSERT_EQ(p.second, 'a');
}

TEST(Tuple, MakeTrivialTuple)
{
    tuple<int, char> t = ryujin::make_tuple(1, 'a');
    int& i = ryujin::get<0>(t);
    char& c = ryujin::get<1>(t);

    ASSERT_EQ(i, 1);
    ASSERT_EQ(c, 'a');
}

TEST(Tuple, MakeTrivialConstTuple)
{
    const tuple<int, char> t = ryujin::make_tuple(1, 'a');
    const int& i = ryujin::get<0>(t);
    const char& c = ryujin::get<1>(t);

    ASSERT_EQ(i, 1);
    ASSERT_EQ(c, 'a');
}