#include <gtest/gtest.h>

#include <ryujin/core/tree.hpp>

using namespace ryujin;

TEST(RedBlackTree, DefaultConstructor)
{
    rb_tree<int> tree;
    
    ASSERT_EQ(tree.size(), 0);
    ASSERT_TRUE(tree.empty());
}

TEST(RedBlackTree, Insertion)
{
    rb_tree<int> tree;
    tree.insert(1);
    auto it = tree.find(1);

    ASSERT_EQ(tree.size(), 1);
    ASSERT_FALSE(tree.empty());
    ASSERT_NE(it, tree.end());
}

TEST(RedBlackTree, InsertMultiple)
{
    rb_tree<int> tree;
    tree.insert(1);
    tree.insert(2);
    auto it1 = tree.find(1);
    auto it2 = tree.find(2);

    ASSERT_EQ(tree.size(), 2);
    ASSERT_FALSE(tree.empty());
    ASSERT_NE(it1, tree.end());
    ASSERT_NE(it2, tree.end());
}

TEST(RedBlackTree, InsertAndRebalance)
{
    rb_tree<int> tree;
    for (int i = 0; i < 32; ++i)
    {
        tree.insert(i);
    }

    ASSERT_EQ(tree.size(), 32);
    ASSERT_FALSE(tree.empty());

    for (int i = 0; i < 32; ++i)
    {
        auto it = tree.find(i);
        ASSERT_NE(it, tree.end());
    }
}

TEST(RedBlackTree, FindOnEmptyTree)
{
    rb_tree<int> tree;

    ASSERT_EQ(tree.find(1), tree.end());
}

TEST(RedBlackTree, FindOnNotEmptyTreeNoSuchElement)
{
    rb_tree<int> tree;
    tree.insert(0);

    ASSERT_EQ(tree.find(1), tree.end());
}

TEST(RedBlackTree, FindOnNotEmptyAfterRebalanceTreeNoSuchElement)
{
    rb_tree<int> tree;
    for(int i = 0; i < 32; ++i)
    {
        tree.insert(i);
    }

    ASSERT_EQ(tree.find(33), tree.end());
}

TEST(RedBlackTree, IteratorOnEmpty)
{
    rb_tree<int> tree;
    auto begin = tree.begin();
    auto end = tree.end();

    ASSERT_EQ(begin, end);
}

TEST(RedBlackTree, IteratorOnSingleElementTree)
{
    rb_tree<int> tree;
    tree.insert(1);

    auto begin = tree.begin();
    auto end = tree.end();

    ASSERT_EQ(*begin, 1);
    ASSERT_NE(begin, end);

    auto next = ++begin;
    ASSERT_EQ(next, end);
}

TEST(RedBlackTree, IteratorAfterRebalance)
{
    rb_tree<int> tree;
    for (int i = 0; i < 32; ++i)
    {
        tree.insert(i);
    }

    auto it = tree.begin();
    for (int i = 0; i < 32; ++i)
    {
        ASSERT_EQ(*it, i);
        ++it;
    }
}