#include <gtest/gtest.h>

#include <ryujin/core/map.hpp>

using ryujin::map;

TEST(Map, DefaultConstructor)
{
    map<int, double> m;

    ASSERT_EQ(m.size(), 0);
    ASSERT_TRUE(m.empty());
}

TEST(Map, Insert)
{
    map<int, double> m;
    auto it = m.insert({ 1, 2.0 });

    ASSERT_EQ(m.size(), 1);
    ASSERT_FALSE(m.empty());
    ASSERT_NE(it, m.end());
    ASSERT_EQ(it->first, 1);
    ASSERT_EQ(it->second, 2.0);
}

TEST(Map, IterateEmpty)
{
    map<int, double> m;

    ASSERT_EQ(m.begin(), m.end());

    ryujin::sz iterations = 0;

    for (const auto& [key, value] : m)
    {
        ++iterations;
    }

    ASSERT_EQ(iterations, 0);
}

TEST(Map, IterateSingle)
{
    map<int, double> m;

    m.insert({ 1, 2.0 });

    ryujin::sz iterations = 0;

    for (const auto& [key, value] : m)
    {
        ASSERT_EQ(key, 1);
        ASSERT_EQ(value, 2.0);
        ++iterations;
    }

    ASSERT_EQ(iterations, 1);
}

TEST(Map, EraseByValue)
{
    map<int, double> m;

    m.insert({ 1, 2.0 });
    m.insert({ 3, 4.0 });

    ASSERT_EQ(m.size(), 2);

    auto it = m.erase(1);

    ASSERT_EQ(it->first, 3);

    ASSERT_FALSE(m.contains(1));
}