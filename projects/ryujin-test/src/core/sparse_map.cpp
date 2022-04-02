#include <gtest/gtest.h>

#include <ryujin/entities/entity.hpp>
#include <ryujin/entities/sparse_map.hpp>

using ryujin::entity;
using ryujin::entity_traits;
using ryujin::sparse_map;

TEST(SparseMapEntityUint32, DefaultConstructor)
{
	sparse_map<entity<std::uint32_t>, int, 1024> map;

	ASSERT_EQ(map.size(), 0);
	ASSERT_GE(map.capacity(), 0u);
}

TEST(SparseMapEntityUint32, InsertSingleValue)
{
	sparse_map<entity<std::uint32_t>, int, 1024> map;

	ASSERT_EQ(map.size(), 0);
	ASSERT_GE(map.capacity(), 0u);

	const entity<std::uint32_t> entity = { 0, 0 };
	map.insert(entity, 1);

	ASSERT_EQ(map.size(), 1);
	ASSERT_GE(map.capacity(), 1u);
	ASSERT_TRUE(map.contains(entity));
	ASSERT_TRUE(map.contains(entity));
	ASSERT_TRUE(map.contains(entity, 1));
	ASSERT_EQ(map.get(entity), 1);
	ASSERT_FALSE(map.contains(entity, 2));
}

TEST(SparseMapEntityUint32, InsertThenRemoveValue)
{
	sparse_map<entity<std::uint32_t>, int, 1024> map;

	ASSERT_EQ(map.size(), 0);
	ASSERT_GE(map.capacity(), 0u);

	const entity<std::uint32_t> entity = { 0, 0 };
	map.insert(entity, 1);

	ASSERT_EQ(map.size(), 1);
	ASSERT_GE(map.capacity(), 1u);
	ASSERT_TRUE(map.contains(entity));
	ASSERT_TRUE(map.contains(entity));
	ASSERT_TRUE(map.contains(entity, 1));
	ASSERT_EQ(map.get(entity), 1);
	ASSERT_FALSE(map.contains(entity, 2));

	map.remove(entity);

	ASSERT_EQ(map.size(), 0);
	ASSERT_GE(map.capacity(), 0u);
	ASSERT_FALSE(map.contains(entity));
	ASSERT_FALSE(map.contains(entity, 1));
}

TEST(SparseMapEntityUint32, InsertThenRemoveWrongValue)
{
	sparse_map<entity<std::uint32_t>, int, 1024> map;

	ASSERT_EQ(map.size(), 0);
	ASSERT_GE(map.capacity(), 0u);

	const entity<std::uint32_t> entity = { 0, 0 };
	map.insert(entity, 1);

	ASSERT_EQ(map.size(), 1);
	ASSERT_GE(map.capacity(), 1u);
	ASSERT_TRUE(map.contains(entity));
	ASSERT_TRUE(map.contains(entity));
	ASSERT_TRUE(map.contains(entity, 1));
	ASSERT_EQ(map.get(entity), 1);
	ASSERT_FALSE(map.contains(entity, 2));

	map.remove(entity, 2);

	ASSERT_EQ(map.size(), 1);
	ASSERT_GE(map.capacity(), 1u);
	ASSERT_TRUE(map.contains(entity));
	ASSERT_TRUE(map.contains(entity, 1));
}

TEST(SparseMapEntityUint32, InsertThenRemoveCorrectValue)
{
	sparse_map<entity<std::uint32_t>, int, 1024> map;

	ASSERT_EQ(map.size(), 0);
	ASSERT_GE(map.capacity(), 0u);

	const entity<std::uint32_t> entity = { 0, 0 };
	map.insert(entity, 1);

	ASSERT_EQ(map.size(), 1);
	ASSERT_GE(map.capacity(), 1u);
	ASSERT_TRUE(map.contains(entity));
	ASSERT_TRUE(map.contains(entity));
	ASSERT_TRUE(map.contains(entity, 1));
	ASSERT_EQ(map.get(entity), 1);
	ASSERT_FALSE(map.contains(entity, 2));

	map.remove(entity, 1);

	ASSERT_EQ(map.size(), 0);
	ASSERT_GE(map.capacity(), 0u);
	ASSERT_FALSE(map.contains(entity));
	ASSERT_FALSE(map.contains(entity, 1));
}

TEST(SparseMapEntityUint32, Clear)
{
	sparse_map<entity<std::uint32_t>, int, 1024> map;

	ASSERT_EQ(map.size(), 0);
	ASSERT_GE(map.capacity(), 0u);

	map.clear();

	ASSERT_EQ(map.size(), 0);
	ASSERT_GE(map.capacity(), 0u);
}

TEST(SparseMapEntityUint32, ClearAfterInsert)
{
	sparse_map<entity<std::uint32_t>, int, 1024> map;

	ASSERT_EQ(map.size(), 0);
	ASSERT_GE(map.capacity(), 0u);

	const entity<std::uint32_t> entity = { 0, 0 };
	map.insert(entity, 1);

	ASSERT_EQ(map.size(), 1);
	ASSERT_GE(map.capacity(), 1u);
	ASSERT_TRUE(map.contains(entity));
	ASSERT_TRUE(map.contains(entity));
	ASSERT_TRUE(map.contains(entity, 1));
	ASSERT_EQ(map.get(entity), 1);
	ASSERT_FALSE(map.contains(entity, 2));

	map.clear();

	ASSERT_EQ(map.size(), 0);
	ASSERT_GE(map.capacity(), 0u);
}

TEST(SparseMapEntityUint32, EqualityEmpty)
{
	sparse_map<entity<std::uint32_t>, int, 1024> lhs;
	sparse_map<entity<std::uint32_t>, int, 1024> rhs;

	ASSERT_TRUE(lhs == rhs);
	ASSERT_FALSE(lhs != rhs);
}

TEST(SparseMapEntityUint32, LhsEmptyEquality)
{
	sparse_map<entity<std::uint32_t>, int, 1024> lhs;
	sparse_map<entity<std::uint32_t>, int, 1024> rhs;

	rhs.insert(entity<std::uint32_t>{ 0, 0 }, 1);

	ASSERT_FALSE(lhs == rhs);
	ASSERT_TRUE(lhs != rhs);
}

TEST(SparseMapEntityUint32, RhsEmptyEquality)
{
	sparse_map<entity<std::uint32_t>, int, 1024> lhs;
	sparse_map<entity<std::uint32_t>, int, 1024> rhs;

	lhs.insert(entity<std::uint32_t>{ 0, 0 }, 1);

	ASSERT_FALSE(lhs == rhs);
	ASSERT_TRUE(lhs != rhs);
}

TEST(SparseMapEntityUint32, SameValueEquality)
{
	sparse_map<entity<std::uint32_t>, int, 1024> lhs;
	sparse_map<entity<std::uint32_t>, int, 1024> rhs;

	lhs.insert(entity<std::uint32_t>{ 0, 0 }, 1);
	rhs.insert(entity<std::uint32_t>{ 0, 0 }, 1);

	ASSERT_TRUE(lhs == rhs);
	ASSERT_FALSE(lhs != rhs);
}

TEST(SparseMapEntityUint32, DifferentValueEquality)
{
	sparse_map<entity<std::uint32_t>, int, 1024> lhs;
	sparse_map<entity<std::uint32_t>, int, 1024> rhs;

	lhs.insert(entity<std::uint32_t>{ 0, 0 }, 1);
	rhs.insert(entity<std::uint32_t>{ 0, 0 }, 2);

	ASSERT_FALSE(lhs == rhs);
	ASSERT_TRUE(lhs != rhs);
}

TEST(SparseMapEntityUint32, DifferentKeyEquality)
{
	sparse_map<entity<std::uint32_t>, int, 1024> lhs;
	sparse_map<entity<std::uint32_t>, int, 1024> rhs;

	lhs.insert(entity<std::uint32_t>{ 0, 0 }, 1);
	rhs.insert(entity<std::uint32_t>{ 0, 1 }, 1);

	ASSERT_FALSE(lhs == rhs);
	ASSERT_TRUE(lhs != rhs);
}