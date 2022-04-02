#include <gtest/gtest.h>

#include <ryujin/core/vector.hpp>

#include <utility>

struct NonTrivialType
{
    int copies = 0;
    int moves = 0;

    NonTrivialType() = default;
    
    NonTrivialType(const NonTrivialType& other)
        : copies(1)
    {
    }

    NonTrivialType(NonTrivialType&& other) noexcept
        : moves(1)
    {
    }

    ~NonTrivialType()
    {
        copies = moves = 0;
    }

    NonTrivialType& operator=(const NonTrivialType& rhs)
    {
        ++copies;
        return *this;
    }

    NonTrivialType& operator=(NonTrivialType&& rhs) noexcept
    {
        ++moves;
        return *this;
    }
};

TEST(StaticVectorInt, DefaultConstructor)
{
    ryujin::static_vector<int, 32> vec;

    ASSERT_EQ(vec.size(), 0);
    ASSERT_EQ(vec.capacity(), 32);
    ASSERT_TRUE(vec.empty());
}

TEST(StaticVectorInt, CopyConstructorEmpty)
{
    ryujin::static_vector<int, 32> source;
    ryujin::static_vector<int, 32> destination(source);

    ASSERT_EQ(destination.size(), source.size());
    ASSERT_EQ(destination.capacity(), source.capacity());
}

TEST(StaticVectorInt, MoveConstructorEmpty)
{
    ryujin::static_vector<int, 32> source;
    ryujin::static_vector<int, 32> destination(std::move(source));

    ASSERT_EQ(destination.size(), source.size());
    ASSERT_EQ(destination.capacity(), source.capacity());
}

TEST(StaticVectorInt, PushBackEmpty)
{
    ryujin::static_vector<int, 32> vec;
    const auto res = vec.push_back(1);

    ASSERT_EQ(vec.size(), 1);
    ASSERT_EQ(vec.capacity(), 32);
    ASSERT_TRUE(res);
    ASSERT_EQ(*res, vec.begin());
    ASSERT_EQ(**res, 1);

    ASSERT_EQ(std::find(vec.begin(), vec.end(), 1), vec.begin());
}

TEST(StaticVectorInt, PushBackAtCapacity)
{
    ryujin::static_vector<int, 1> vec;
    const auto res = vec.push_back(1);
    const auto res2 = vec.push_back(2);

    ASSERT_EQ(vec.size(), 1);
    ASSERT_EQ(vec.capacity(), 1);
    ASSERT_TRUE(res);
    ASSERT_FALSE(res2);

    ASSERT_EQ(std::find(vec.begin(), vec.end(), 1), vec.begin());
    ASSERT_EQ(std::find(vec.begin(), vec.end(), 2), vec.end());
}

TEST(StaticVectorInt, InsertAtFrontEmpty)
{
    ryujin::static_vector<int, 32> vec;

    const auto res = vec.insert(vec.begin(), 1);
    ASSERT_EQ(vec.size(), 1);
    ASSERT_EQ(vec.capacity(), 32);
    ASSERT_TRUE(res);
    ASSERT_EQ(*res, vec.begin());
    ASSERT_EQ(**res, 1);

    ASSERT_EQ(std::find(vec.begin(), vec.end(), 1), vec.begin());
}

TEST(StaticVectorInt, InsertAtFrontNotFull)
{
    ryujin::static_vector<int, 32> vec;

    const auto res = vec.insert(vec.begin(), 1);
    const auto res2 = vec.insert(vec.begin(), 2);

    ASSERT_EQ(vec.size(), 2);
    ASSERT_EQ(vec.capacity(), 32);
    ASSERT_TRUE(res);

    ASSERT_TRUE(res);
    ASSERT_EQ(*res2, vec.begin());
    ASSERT_EQ(**res2, 2);

    ASSERT_EQ(std::find(vec.begin(), vec.end(), 1), vec.begin() + 1);
    ASSERT_EQ(std::find(vec.begin(), vec.end(), 2), vec.begin());
}

TEST(StaticVectorInt, InsertAtFrontFull)
{
    ryujin::static_vector<int, 1> vec;

    const auto res = vec.insert(vec.begin(), 1);
    const auto res2 = vec.insert(vec.begin(), 2);

    ASSERT_EQ(vec.size(), 1);
    ASSERT_EQ(vec.capacity(), 1);
    ASSERT_TRUE(res);
    ASSERT_EQ(*res, vec.begin());
    ASSERT_EQ(**res, 1);

    ASSERT_FALSE(res2);

    ASSERT_EQ(std::find(vec.begin(), vec.end(), 1), vec.begin());
    ASSERT_EQ(std::find(vec.begin(), vec.end(), 2), vec.end());
}

TEST(StaticVectorInt, InsertInMiddle)
{
    ryujin::static_vector<int, 32> vec;

    vec.insert(vec.begin(), 3);
    vec.insert(vec.begin(), 1);

    const auto res = vec.insert(vec.begin() + 1, 2);

    ASSERT_EQ(vec.size(), 3);
    ASSERT_EQ(vec.capacity(), 32);
    ASSERT_TRUE(res);
    ASSERT_EQ(*res, vec.begin() + 1);
    ASSERT_EQ(**res, 2);

    ASSERT_EQ(std::find(vec.begin(), vec.end(), 1), vec.begin());
    ASSERT_EQ(std::find(vec.begin(), vec.end(), 2), vec.begin() + 1);
    ASSERT_EQ(std::find(vec.begin(), vec.end(), 3), vec.begin() + 2);
}

TEST(StaticVectorInt, InsertInMiddleFull)
{
    ryujin::static_vector<int, 2> vec;

    vec.insert(vec.begin(), 3);
    vec.insert(vec.begin(), 1);

    const auto res = vec.insert(vec.begin() + 1, 2);

    ASSERT_EQ(vec.size(), 2);
    ASSERT_EQ(vec.capacity(), 2);
    ASSERT_FALSE(res);

    ASSERT_EQ(std::find(vec.begin(), vec.end(), 1), vec.begin());
    ASSERT_EQ(std::find(vec.begin(), vec.end(), 2), vec.end());
    ASSERT_EQ(std::find(vec.begin(), vec.end(), 3), vec.begin() + 1);
}

TEST(StaticVectorInt, InsertAtEnd)
{
    ryujin::static_vector<int, 32> vec;

    vec.insert(vec.begin(), 1);
    vec.insert(vec.begin() + 1, 2);

    const auto res = vec.insert(vec.end(), 3);

    ASSERT_EQ(vec.size(), 3);
    ASSERT_EQ(vec.capacity(), 32);
    ASSERT_TRUE(res);

    ASSERT_EQ(std::find(vec.begin(), vec.end(), 1), vec.begin());
    ASSERT_EQ(std::find(vec.begin(), vec.end(), 2), vec.begin() + 1);
    ASSERT_EQ(std::find(vec.begin(), vec.end(), 3), vec.begin() + 2);
}

TEST(StaticVectorInt, InsertAtIllegalPosition)
{
    ryujin::static_vector<int, 32> vec;

    vec.insert(vec.begin(), 1);
    vec.insert(vec.begin() + 1, 2);

    const auto res = vec.insert(vec.end() + 1, 3);

    ASSERT_EQ(vec.size(), 2);
    ASSERT_EQ(vec.capacity(), 32);
    ASSERT_FALSE(res);
    ASSERT_EQ(res.error_code(), decltype(vec)::error_code::INVALID_OPERATION);
}

TEST(StaticVectorInt, InsertRangeAtStartEmpty)
{
    ryujin::static_vector<int, 32> insertion;
    insertion.push_back(1);
    insertion.push_back(2);
    insertion.push_back(3);
    insertion.push_back(4);

    ryujin::static_vector<int, 4> vec;
    const auto res = vec.insert(vec.begin(), insertion.begin(), insertion.end());

    ASSERT_EQ(vec.size(), 4);
    ASSERT_TRUE(res);
    ASSERT_EQ(std::find(vec.begin(), vec.end(), 1), vec.begin());
    ASSERT_EQ(std::find(vec.begin(), vec.end(), 2), vec.begin() + 1);
    ASSERT_EQ(std::find(vec.begin(), vec.end(), 3), vec.begin() + 2);
    ASSERT_EQ(std::find(vec.begin(), vec.end(), 4), vec.begin() + 3);
}

TEST(StaticVectorInt, InsertRangeAtStart)
{
    ryujin::static_vector<int, 32> insertion;
    insertion.push_back(1);
    insertion.push_back(2);
    insertion.push_back(3);
    insertion.push_back(4);

    ryujin::static_vector<int, 5> vec;
    vec.push_back(5);
    const auto res = vec.insert(vec.begin(), insertion.begin(), insertion.end());

    ASSERT_EQ(vec.size(), 5);
    ASSERT_TRUE(res);
    ASSERT_EQ(std::find(vec.begin(), vec.end(), 1), vec.begin());
    ASSERT_EQ(std::find(vec.begin(), vec.end(), 2), vec.begin() + 1);
    ASSERT_EQ(std::find(vec.begin(), vec.end(), 3), vec.begin() + 2);
    ASSERT_EQ(std::find(vec.begin(), vec.end(), 4), vec.begin() + 3);
    ASSERT_EQ(std::find(vec.begin(), vec.end(), 5), vec.begin() + 4);
}

TEST(StaticVectorInt, InsertRangeInMiddle)
{
    ryujin::static_vector<int, 32> insertion;
    insertion.push_back(2);
    insertion.push_back(3);

    ryujin::static_vector<int, 4> vec;
    vec.push_back(1);
    vec.push_back(4);
    const auto res = vec.insert(vec.begin() + 1, insertion.begin(), insertion.end());

    ASSERT_EQ(vec.size(), 4);
    ASSERT_TRUE(res);
    ASSERT_EQ(std::find(vec.begin(), vec.end(), 1), vec.begin());
    ASSERT_EQ(std::find(vec.begin(), vec.end(), 2), vec.begin() + 1);
    ASSERT_EQ(std::find(vec.begin(), vec.end(), 3), vec.begin() + 2);
    ASSERT_EQ(std::find(vec.begin(), vec.end(), 4), vec.begin() + 3);
}

TEST(StaticVectorInt, InsertRangeTooLarge)
{
    ryujin::static_vector<int, 32> insertion;
    insertion.push_back(2);
    insertion.push_back(3);
    insertion.push_back(4);

    ryujin::static_vector<int, 4> vec;
    vec.push_back(1);
    vec.push_back(5);
    const auto res = vec.insert(vec.begin() + 1, insertion.begin(), insertion.end());

    ASSERT_EQ(vec.size(), 2);
    ASSERT_FALSE(res);
    ASSERT_EQ(std::find(vec.begin(), vec.end(), 1), vec.begin());
    ASSERT_EQ(std::find(vec.begin(), vec.end(), 5), vec.begin() + 1);
    ASSERT_EQ(std::find(vec.begin(), vec.end(), 2), vec.end());
    ASSERT_EQ(std::find(vec.begin(), vec.end(), 3), vec.end());
    ASSERT_EQ(std::find(vec.begin(), vec.end(), 4), vec.end());
}

TEST(StaticVectorInt, PopBackEmpty)
{
    ryujin::static_vector<int, 32> vec;

    const auto res = vec.pop_back();

    ASSERT_EQ(res, decltype(vec)::error_code::INVALID_OPERATION);
}

TEST(StaticVectorInt, PopBackSingleElement)
{
    ryujin::static_vector<int, 32> vec;
    vec.push_back(1);

    const auto res = vec.pop_back();
    ASSERT_EQ(res, decltype(vec)::error_code::SUCCESS);
    ASSERT_EQ(vec.size(), 0);
    ASSERT_TRUE(vec.empty());
}

TEST(StaticVectorInt, EraseBackElementSingleElement)
{
    ryujin::static_vector<int, 32> vec;
    vec.push_back(1);

    const auto res = vec.erase(vec.begin());
    ASSERT_EQ(vec.size(), 0);
    ASSERT_TRUE(vec.empty());
    ASSERT_EQ(*res, vec.begin());
}

TEST(StaticVectorInt, EraseBackElementMultiElement)
{
    ryujin::static_vector<int, 32> vec;
    vec.push_back(1);
    vec.push_back(2);

    const auto res = vec.erase(vec.begin() + 1);
    ASSERT_EQ(vec.size(), 1);
    ASSERT_EQ(*res, vec.begin() + 1);
}

TEST(StaticVectorInt, EraseAllElements)
{
    ryujin::static_vector<int, 32> vec;
    vec.push_back(1);
    vec.push_back(2);

    const auto res = vec.erase(vec.begin(), vec.end());
    ASSERT_EQ(vec.size(), 0);
    ASSERT_EQ(*res, vec.begin());
}

TEST(StaticVectorInt, EraseFrontElement)
{
    ryujin::static_vector<int, 32> vec;
    vec.push_back(1);
    vec.push_back(2);

    const auto res = vec.erase(vec.begin(), vec.begin() + 1);
    ASSERT_EQ(vec.size(), 1);
    ASSERT_EQ(*res, vec.begin());
}

TEST(StaticVectorInt, EraseEmptyVector)
{
    ryujin::static_vector<int, 32> vec;

    const auto res = vec.erase(vec.begin(), vec.end());
    ASSERT_EQ(vec.size(), 0);
    ASSERT_EQ(*res, vec.begin());
}

TEST(StaticVectorInt, EraseEmptyVectorInvalidIterator)
{
    ryujin::static_vector<int, 32> vec;

    const auto res = vec.erase(vec.begin(), vec.end() + 1);
    ASSERT_EQ(vec.size(), 0);
    ASSERT_FALSE(res);
    ASSERT_EQ(res.error_code(), decltype(vec)::error_code::INVALID_OPERATION);
}

TEST(StaticVectorInt, EraseRangeEmptyVector)
{
    ryujin::static_vector<int, 32> vec;

    const auto res = vec.erase(vec.begin(), vec.end());
    ASSERT_EQ(vec.size(), 0);
    ASSERT_TRUE(res);
    ASSERT_EQ(*res, vec.begin());
}

TEST(StaticVectorInt, EraseRangeSingleValueVector)
{
    ryujin::static_vector<int, 32> vec;
    vec.push_back(1);

    const auto res = vec.erase(vec.begin(), vec.end());
    ASSERT_EQ(vec.size(), 0);
    ASSERT_TRUE(res);
    ASSERT_EQ(*res, vec.begin());
    ASSERT_EQ(std::find(vec.begin(), vec.end(), 1), vec.end());
}

TEST(StaticVectorInt, EraseRangeFromFront)
{
    ryujin::static_vector<int, 32> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);

    const auto res = vec.erase(vec.begin(), vec.begin() + 2);
    ASSERT_EQ(vec.size(), 1);
    ASSERT_TRUE(res);
    ASSERT_EQ(*res, vec.begin());
    ASSERT_EQ(std::find(vec.begin(), vec.end(), 1), vec.end());
    ASSERT_EQ(std::find(vec.begin(), vec.end(), 2), vec.end());
    ASSERT_EQ(std::find(vec.begin(), vec.end(), 3), vec.begin());
}

TEST(StaticVectorInt, EraseRangeFromEnd)
{
    ryujin::static_vector<int, 32> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);

    const auto res = vec.erase(vec.begin() + 1, vec.end());
    ASSERT_EQ(vec.size(), 1);
    ASSERT_TRUE(res);
    ASSERT_EQ(*res, vec.begin() + 1);
    ASSERT_EQ(std::find(vec.begin(), vec.end(), 1), vec.begin());
    ASSERT_EQ(std::find(vec.begin(), vec.end(), 2), vec.end());
    ASSERT_EQ(std::find(vec.begin(), vec.end(), 3), vec.end());
}

TEST(StaticVectorInt, EraseRangeFromMiddle)
{
    ryujin::static_vector<int, 32> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    vec.push_back(4);

    const auto res = vec.erase(vec.begin() + 1, vec.begin() + 3);
    ASSERT_EQ(vec.size(), 2);
    ASSERT_TRUE(res);
    ASSERT_EQ(*res, vec.begin() + 1);
    ASSERT_EQ(std::find(vec.begin(), vec.end(), 1), vec.begin());
    ASSERT_EQ(std::find(vec.begin(), vec.end(), 2), vec.end());
    ASSERT_EQ(std::find(vec.begin(), vec.end(), 3), vec.end());
    ASSERT_EQ(std::find(vec.begin(), vec.end(), 4), vec.begin() + 1);
}

TEST(StaticVectorNonTrivialType, DefaultConstructor)
{
    ryujin::static_vector<NonTrivialType, 32> vec;

    ASSERT_EQ(vec.size(), 0);
    ASSERT_TRUE(vec.empty());
}

TEST(StaticVectorNonTrivialType, CopyConstructorEmpty)
{
    ryujin::static_vector<NonTrivialType, 32> source;
    ryujin::static_vector<NonTrivialType, 32> destination(source);

    ASSERT_EQ(destination.size(), source.size());
    ASSERT_EQ(destination.capacity(), source.capacity());
}

TEST(StaticVectorNonTrivialType, MoveConstructorEmpty)
{
    ryujin::static_vector<NonTrivialType, 32> source;
    ryujin::static_vector<NonTrivialType, 32> destination(std::move(source));

    ASSERT_EQ(destination.size(), source.size());
    ASSERT_EQ(destination.capacity(), source.capacity());
}

TEST(StaticVectorNonTrivialType, PushBackEmpty)
{
    ryujin::static_vector<NonTrivialType, 32> vec;

    const auto res = vec.push_back({});

    ASSERT_EQ(vec.size(), 1);
    ASSERT_EQ(vec.capacity(), 32);
    ASSERT_TRUE(res);
    ASSERT_EQ(*res, vec.begin());
}

TEST(StaticVectorNonTrivialType, PushBackAtCapacity)
{
    ryujin::static_vector<NonTrivialType, 1> vec;

    vec.push_back({});
    const auto res = vec.push_back({});

    ASSERT_EQ(vec.size(), 1);
    ASSERT_EQ(vec.capacity(), 1);
    ASSERT_FALSE(res);
    ASSERT_EQ(res.error_code(), decltype(vec)::error_code::OUT_OF_MEMORY);
}