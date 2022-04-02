#include <gtest/gtest.h>

#include <ryujin/entities/entity.hpp>
#include <ryujin/entities/sparse_set.hpp>

using ryujin::entity;
using ryujin::entity_traits;
using ryujin::sparse_set;

TEST(SparseSetEntityUint32, DefaultConstructor)
{
	sparse_set<entity<uint32_t>, 1024> set;

	ASSERT_TRUE(set.empty());
	ASSERT_EQ(set.size(), 0);
	ASSERT_EQ(set.capacity(), 0);
}

TEST(SparseSetEntityUint32, InsertSingleValue)
{
	sparse_set<entity<uint32_t>, 1024> set;

	ASSERT_TRUE(set.empty());
	ASSERT_EQ(set.size(), 0);
	ASSERT_EQ(set.capacity(), 0);

	const entity<std::uint32_t> entity = { 0, 0 };
	
	set.insert(entity);

	ASSERT_TRUE(set.contains(entity));
	ASSERT_FALSE(set.empty());
	ASSERT_EQ(set.size(), 1);
	ASSERT_GE(set.capacity(), 1u);
}

TEST(SparseSetEntityUint32, InsertThenRemoveValue)
{
	sparse_set<entity<uint32_t>, 1024> set;

	ASSERT_TRUE(set.empty());
	ASSERT_EQ(set.size(), 0);
	ASSERT_EQ(set.capacity(), 0);

	const entity<std::uint32_t> entity = { 0, 0 };

	set.insert(entity);

	ASSERT_TRUE(set.contains(entity));
	ASSERT_FALSE(set.empty());
	ASSERT_EQ(set.size(), 1);
	ASSERT_GE(set.capacity(), 1u);

	set.remove(entity);

	ASSERT_FALSE(set.contains(entity));
	ASSERT_TRUE(set.empty());
	ASSERT_EQ(set.size(), 0);
	ASSERT_GE(set.capacity(), 0u);
}

TEST(SparseSetEntityUint32, ClearEmpty)
{
	sparse_set<entity<uint32_t>, 1024> set;

	ASSERT_TRUE(set.empty());
	ASSERT_EQ(set.size(), 0);
	ASSERT_EQ(set.capacity(), 0);

	const entity<std::uint32_t> entity = { 0, 0 };

	set.insert(entity);

	ASSERT_TRUE(set.contains(entity));
	ASSERT_FALSE(set.empty());
	ASSERT_EQ(set.size(), 1);
	ASSERT_GE(set.capacity(), 1u);

	set.clear();

	ASSERT_TRUE(set.empty());
	ASSERT_EQ(set.size(), 0);
	ASSERT_GE(set.capacity(), 0u);
}

TEST(SparseSetEntityUint32, ClearAfterInsert)
{
	sparse_set<entity<uint32_t>, 1024> set;

	ASSERT_TRUE(set.empty());
	ASSERT_EQ(set.size(), 0);
	ASSERT_EQ(set.capacity(), 0);

	set.clear();

	ASSERT_TRUE(set.empty());
	ASSERT_EQ(set.size(), 0);
	ASSERT_GE(set.capacity(), 0u);
}

TEST(SparseSetEntityUint32, EmptyEquality)
{
	sparse_set<entity<uint32_t>, 1024> lhs;
	sparse_set<entity<uint32_t>, 1024> rhs;

	ASSERT_TRUE(lhs == rhs);
	ASSERT_FALSE(lhs != rhs);
}

TEST(SparseSetEntityUint32, LhsEmptyEquality)
{
	sparse_set<entity<uint32_t>, 1024> lhs;
	sparse_set<entity<uint32_t>, 1024> rhs;

	rhs.insert(entity<std::uint32_t>{ 0, 0 });

	ASSERT_FALSE(lhs == rhs);
	ASSERT_TRUE(lhs != rhs);
}

TEST(SparseSetEntityUint32, RhsEmptyEquality)
{
	sparse_set<entity<uint32_t>, 1024> lhs;
	sparse_set<entity<uint32_t>, 1024> rhs;

	lhs.insert(entity<std::uint32_t>{ 0, 0 });

	ASSERT_FALSE(lhs == rhs);
	ASSERT_TRUE(lhs != rhs);
}

TEST(SparseSetEntityUint32, SameValueEquality)
{
	sparse_set<entity<uint32_t>, 1024> lhs;
	sparse_set<entity<uint32_t>, 1024> rhs;

	lhs.insert(entity<std::uint32_t>{ 0, 0 });
	rhs.insert(entity<std::uint32_t>{ 0, 0 });

	ASSERT_TRUE(lhs == rhs);
	ASSERT_FALSE(lhs != rhs);
}

TEST(SparseSetEntityUint32, DifferentValueEquality)
{
	sparse_set<entity<uint32_t>, 1024> lhs;
	sparse_set<entity<uint32_t>, 1024> rhs;

	lhs.insert(entity<std::uint32_t>{ 0, 0 });
	rhs.insert(entity<std::uint32_t>{ 0, 1 });

	ASSERT_FALSE(lhs == rhs);
	ASSERT_TRUE(lhs != rhs);
}