#include <gtest/gtest.h>

#include <ryujin/entities/registry.hpp>

using ryujin::entity;
using ryujin::entity_handle;
using ryujin::base_registry;

TEST(Registry, DefaultConstructor)
{
	base_registry<entity<std::uint32_t>> reg;

	ASSERT_EQ(reg.active(), 0);
	ASSERT_GE(reg.capacity(), 0u);
}

TEST(Registry, AllocateSingle)
{
	base_registry<entity<std::uint32_t>> reg;

	auto entity = reg.allocate();
	ASSERT_EQ(reg.active(), 1);
	ASSERT_GE(reg.capacity(), 1u);
}

TEST(Registry, AllocateSingleThenDeallocate)
{
	base_registry<entity<std::uint32_t>> reg;

	auto entity = reg.allocate();
	reg.deallocate(entity);

	ASSERT_EQ(reg.active(), 0);
	ASSERT_GE(reg.capacity(), 0u);
}

TEST(Registry, AllocateSingleThenAssign)
{
	base_registry<entity<std::uint32_t>> reg;

	auto entity = reg.allocate();
	reg.assign(entity, 1);

	ASSERT_EQ(reg.active(), 1);
	ASSERT_GE(reg.capacity(), 1u);
	ASSERT_TRUE(reg.contains<int>(entity));
}

TEST(Registry, AllocateMultiple)
{
	base_registry<entity<std::uint32_t>> reg;

	for (int i = 0; i < 10; ++i)
	{
		reg.allocate();
	}

	ASSERT_EQ(reg.active(), 10);
	ASSERT_GE(reg.capacity(), 10u);
}

TEST(Registry, AllocateMultipleAndReleaseAll)
{
	base_registry<entity<std::uint32_t>> reg;

	std::vector<entity_handle<entity<std::uint32_t>>> entities;
	for (int i = 0; i < 10; ++i)
	{
		auto ent = reg.allocate();
		entities.push_back(ent);
	}

	ASSERT_EQ(reg.active(), 10);
	ASSERT_GE(reg.capacity(), 10u);

	for (auto entity : entities)
	{
		reg.deallocate(entity);
	}

	ASSERT_EQ(reg.active(), 0);
	ASSERT_GE(reg.capacity(), 10u);
}

TEST(Registry, AllocateMultipleAndReleaseEveryOther)
{
	base_registry<entity<std::uint32_t>> reg;

	std::vector<entity_handle<entity<std::uint32_t>>> entities;
	for (int i = 0; i < 10; ++i)
	{
		auto ent = reg.allocate();
		if (i % 2 == 0)
		{
			entities.push_back(ent);
		}
	}

	ASSERT_EQ(reg.active(), 10);
	ASSERT_GE(reg.capacity(), 10u);

	for (auto entity : entities)
	{
		reg.deallocate(entity);
	}

	ASSERT_EQ(reg.active(), 5);
	ASSERT_GE(reg.capacity(), 10u);
}

TEST(Registry, IterateOverEmptyComponents)
{
	base_registry<entity<std::uint32_t>> reg;

	int count = 0; // expect 0
	for (auto& component : reg.component_view<int>())
	{
		++count;
	}

	ASSERT_EQ(count, 0);
}

TEST(Registry, IterateOverSingleComponent)
{
	base_registry<entity<std::uint32_t>> reg;

	auto entity = reg.allocate();
	reg.assign(entity, 1);

	int count = 0; // expect 1
	for (auto& component : reg.component_view<int>())
	{
		++count;
	}

	ASSERT_EQ(count, 1);
}

TEST(Registry, IterateOverNoEntities)
{
	base_registry<entity<std::uint32_t>> reg;

	int count = 0; // expect 0
	for (auto component : reg.entity_view<int>())
	{
		++count;
	}

	ASSERT_EQ(count, 0);
}

TEST(Registry, IterateOverSingleEntityWithComponent)
{
	base_registry<entity<std::uint32_t>> reg;
	reg.allocate().assign(1);

	int count = 0; // expect 1
	for (auto component : reg.entity_view<int>())
	{
		++count;
	}

	ASSERT_EQ(count, 1);
}


TEST(Registry, IterateOverSingleEntityWithComponentNoDefinedComponents)
{
	base_registry<entity<std::uint32_t>> reg;
	reg.allocate().assign(1);

	int count = 0; // expect 1
	for (auto component : reg.entity_view<>())
	{
		++count;
	}

	ASSERT_EQ(count, 1);
}

TEST(Registry, IterateOverSingleEntityWithDifferentComponent)
{
	base_registry<entity<std::uint32_t>> reg;
	reg.allocate().assign(1);

	int count = 0; // expect 0
	for (auto component : reg.entity_view<unsigned int>())
	{
		++count;
	}

	ASSERT_EQ(count, 0);
}

TEST(Registry, IterateOverNoEntitiesAfterRelease)
{
	base_registry<entity<std::uint32_t>> reg;
	auto entity = reg.allocate().assign(1);
	reg.deallocate(entity);

	int count = 0; // expect 0
	for (auto component : reg.entity_view<int>())
	{
		++count;
	}

	ASSERT_EQ(count, 0);
}

TEST(Registry, IterateOverEveryOtherEntity)
{
	base_registry<entity<std::uint32_t>> reg;
	
	for (int i = 0; i < 16; ++i)
	{
		auto entity = reg.allocate();
		if (i % 2 == 0)
		{
			entity.assign(i);
		}
	}

	int count = 0; // expect 8
	for (auto component : reg.entity_view<int>())
	{
		++count;
	}

	ASSERT_EQ(count, 8);
}

TEST(Registry, IterateOverEveryOtherEntityNoComponentsDefined)
{
	base_registry<entity<std::uint32_t>> reg;

	for (int i = 0; i < 16; ++i)
	{
		auto entity = reg.allocate();
		if (i % 2 == 0)
		{
			entity.assign(i);
		}
	}

	int count = 0; // expect 16
	for (auto component : reg.entity_view<>())
	{
		++count;
	}

	ASSERT_EQ(count, 16);
}

TEST(Registry, IterateOverEveryOtherEntityAfterDeallocation)
{
	base_registry<entity<std::uint32_t>> reg;
	std::vector<entity_handle<entity<std::uint32_t>>> ents;

	for (int i = 0; i < 16; ++i)
	{
		auto entity = reg.allocate();
		if (i % 2 == 0)
		{
			ents.push_back(entity);
		}
	}

	for (auto ent : ents) reg.deallocate(ent);

	int count = 0; // expect 8
	for (auto component : reg.entity_view<>())
	{
		++count;
	}

	ASSERT_EQ(count, 8);
}

TEST(Registry, IterateOverEveryOtherEntityWithComponentAfterDeallocation)
{
	base_registry<entity<std::uint32_t>> reg;
	std::vector<entity_handle<entity<std::uint32_t>>> ents;

	for (int i = 0; i < 16; ++i)
	{
		auto entity = reg.allocate();
		if (i % 2 == 0)
		{
			entity.assign(1);
			ents.push_back(entity);
		}
	}

	for (auto ent : ents) reg.deallocate(ent);

	int count = 0; // expect 0
	for (auto component : reg.entity_view<int>())
	{
		++count;
	}

	ASSERT_EQ(count, 0);
}

TEST(Registry, IterateOverEveryOtherEntityWithComponentAfterDeallocationOfComponentlessEntities)
{
	base_registry<entity<std::uint32_t>> reg;
	std::vector<entity_handle<entity<std::uint32_t>>> ents;

	for (int i = 0; i < 16; ++i)
	{
		auto entity = reg.allocate();
		if (i % 2 == 0)
		{
			entity.assign(1);
		}
		else
		{
			ents.push_back(entity);
		}
	}

	for (auto ent : ents) reg.deallocate(ent);

	int count = 0; // expect 8
	for (auto component : reg.entity_view<int>())
	{
		++count;
	}

	ASSERT_EQ(count, 8);
}