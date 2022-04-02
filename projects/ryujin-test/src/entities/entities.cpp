#include <gtest/gtest.h>

#include <ryujin/entities/entity.hpp>

using ryujin::entity;
using ryujin::entity_traits;

TEST(EntityUint32, ToTypeZero)
{
	const entity<std::uint32_t> e{ 0, 0 };
	const auto uuid = entity_traits<std::uint32_t>::to_type(e);

	ASSERT_EQ(uuid, 0);
}

TEST(EntityUint32, ToTypeZeroVersion)
{
	const entity<std::uint32_t> e{ 1, 0 };
	const auto uuid = entity_traits<std::uint32_t>::to_type(e);

	ASSERT_EQ(uuid, 1);
}

TEST(EntityUint32, ToTypeZeroIdentifier)
{
	const entity<std::uint32_t> e{ 0, 1 };
	const auto uuid = entity_traits<std::uint32_t>::to_type(e);

	ASSERT_EQ(uuid, 1 << entity_traits<std::uint32_t>::version_shift);
}

TEST(EntityUint32, ToTypeNonZeroIdAndVersion)
{
	const entity<std::uint32_t> e{ 1, 1 };
	const auto uuid = entity_traits<std::uint32_t>::to_type(e);

	ASSERT_EQ(uuid, (1 << entity_traits<std::uint32_t>::version_shift) | 1);
}