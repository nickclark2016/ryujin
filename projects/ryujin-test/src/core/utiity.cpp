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

TEST(Tuple, SwapTrivialTuple)
{
    tuple<int, char> t0 = ryujin::make_tuple(1, 'a');
    tuple<int, char> t1 = ryujin::make_tuple(2, 'b');

    t0.swap(t1);

    const int& i0 = ryujin::get<0>(t0);
    const char& c0 = ryujin::get<1>(t0);

    const int& i1 = ryujin::get<0>(t1);
    const char& c1 = ryujin::get<1>(t1);

    ASSERT_EQ(i0, 2);
    ASSERT_EQ(c0, 'b');
    ASSERT_EQ(i1, 1);
    ASSERT_EQ(c1, 'a');
}

TEST(Tuple, StructuredBindingTrivialTuple)
{
    tuple<int, char> t = ryujin::make_tuple(1, 'a');
    auto& [i, c] = t;

    ASSERT_EQ(i, 1);
    ASSERT_EQ(c, 'a');
}

TEST(Tuple, StructuredBindingTrivialConstTuple)
{
    const tuple<int, char> t = ryujin::make_tuple(1, 'a');
    const auto& [i, c] = t;

    ASSERT_EQ(i, 1);
    ASSERT_EQ(c, 'a');
}