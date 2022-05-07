#include <gtest/gtest.h>

#include <ryujin/core/unordered_map.hpp>

using ryujin::unordered_map;

TEST(UnorderedMap, DefaultConstructor)
{
    unordered_map<int, float> um;

    ASSERT_EQ(um.size(), 0);
    ASSERT_GE(um.capacity(), 0);
    ASSERT_TRUE(um.empty());
    ASSERT_FALSE(um.contains(1));
}

TEST(UnorderedMap, SingleInsertByCopy)
{
    unordered_map<int, float> um;
    
    const auto pair = ryujin::make_pair(1, 2.0f);

    um.insert(pair);

    ASSERT_EQ(um.size(), 1);
    ASSERT_GE(um.capacity(), 32);
    ASSERT_FALSE(um.empty());
    ASSERT_TRUE(um.contains(1));
}

TEST(UnorderedMap, SingleInsertByCopyTwice)
{
    unordered_map<int, float> um;

    const auto pair = ryujin::make_pair(1, 2.0f);

    const auto first = um.insert(pair);
    const auto second = um.insert(pair);

    ASSERT_NE(first, um.end());
    ASSERT_EQ(second, um.end());

    ASSERT_EQ(um.size(), 1);
    ASSERT_GE(um.capacity(), 32);
    ASSERT_FALSE(um.empty());
    ASSERT_TRUE(um.contains(1));

    ASSERT_EQ(first->first, 1);
    ASSERT_EQ(first->second, 2.0f);
}

TEST(UnorderedMap, InsertUntilResize)
{
    using ryujin::sz;

    unordered_map<sz, float> um;

    for (sz i = 0; i < 32; ++i)
    {
        const auto pair = ryujin::make_pair(i, 2.0f * i);
        um.insert(pair);
    }

    ASSERT_EQ(um.size(), 32);
    ASSERT_GE(um.capacity(), 32);
    ASSERT_FALSE(um.empty());

    for (sz i = 0; i < 32; ++i)
    {
        ASSERT_TRUE(um.contains(i));
    }
}

TEST(UnorderedMap, InsertSkipUntilResize)
{
    using ryujin::sz;

    unordered_map<sz, float> um;

    for (sz i = 0; i < 32; i += 2)
    {
        const auto pair = ryujin::make_pair(i, 2.0f * i);
        um.insert(pair);
    }

    ASSERT_EQ(um.size(), 16);
    ASSERT_GE(um.capacity(), 32);
    ASSERT_FALSE(um.empty());

    for (sz i = 0; i < 32; ++i)
    {
        if (i % 2 == 0)
        {
            ASSERT_TRUE(um.contains(i));
        }
        else
        {
            ASSERT_FALSE(um.contains(i));
        }
    }
}

TEST(UnorderedMap, InsertThenErase)
{
    using ryujin::sz;

    unordered_map<int, float> um;
    um.insert({ 1, 2.0f });

    ASSERT_EQ(um.size(), 1);
    ASSERT_TRUE(um.contains(1));

    um.erase(1);

    ASSERT_EQ(um.size(), 0);
    ASSERT_TRUE(um.empty());
    ASSERT_FALSE(um.contains(1));
}