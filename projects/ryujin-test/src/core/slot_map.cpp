#include <ryujin/core/slot_map.hpp>

#include <gtest/gtest.h>

using ryujin::slot_map;

TEST(SlotMap, DefaultConstructor)
{
    slot_map<int> slots;

    ASSERT_EQ(slots.size(), 0);
    ASSERT_EQ(slots.capacity(), 0);
}

TEST(SlotMap, Insert)
{
    slot_map<int> slots;
    const auto key = slots.insert(1);
    
    ASSERT_EQ(key.index, 0);
    ASSERT_EQ(key.generation, 0);
    ASSERT_NE(key, decltype(slots)::invalid);
    ASSERT_EQ(slots.size(), 1);
    ASSERT_GE(slots.capacity(), 1);

    const auto retrieved = slots.try_get(key);
    ASSERT_NE(retrieved, nullptr);
    ASSERT_EQ(*retrieved, 1);
}

TEST(SlotMap, InsertMultiple)
{
    slot_map<int> slots;
    const auto key1 = slots.insert(1);
    const auto key2 = slots.insert(2);

    ASSERT_EQ(key1.index, 0);
    ASSERT_EQ(key1.generation, 0);
    ASSERT_NE(key1, decltype(slots)::invalid);
    ASSERT_EQ(key2.index, 1);
    ASSERT_EQ(key2.generation, 0);
    ASSERT_NE(key2, decltype(slots)::invalid);
    ASSERT_EQ(slots.size(), 2);
    ASSERT_GE(slots.capacity(), 2);
    ASSERT_NE(key1, key2);

    const auto retrieved1 = slots.try_get(key1);
    ASSERT_NE(retrieved1, nullptr);
    ASSERT_EQ(*retrieved1, 1);

    const auto retrieved2 = slots.try_get(key2);
    ASSERT_NE(retrieved2, nullptr);
    ASSERT_EQ(*retrieved2, 2);
}

TEST(SlotMap, InsertRemoveInsert)
{
    slot_map<int> slots;
    const auto key1 = slots.insert(1);
    slots.erase(key1);
    const auto key2 = slots.insert(2);

    ASSERT_NE(key1, key2);
    ASSERT_EQ(key1.index, key2.index);
    ASSERT_EQ(key2.index, 0);
    ASSERT_EQ(key2.generation, 1);
    ASSERT_EQ(slots.try_get(key1), nullptr);
    ASSERT_EQ(slots.size(), 1);
    ASSERT_GE(slots.capacity(), 1);
    ASSERT_EQ(*slots.try_get(key2), 2);
}