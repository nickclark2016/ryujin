#ifndef entity_hpp__
#define entity_hpp__

#include "../core/primitives.hpp"

namespace ryujin
{
    template <typename T>
    struct entity
    {
    };

    template <>
    struct entity<u32>
    {
        using type = u32;

        u32 identifier : 22;
        u16 version : 10;
    };

    template <>
    struct entity<u64>
    {
        using type = u64;

        u32 identifier;
        u32 version;
    };

    template <typename T>
    constexpr bool operator==(const entity<T>& lhs, const entity<T>& rhs) noexcept
    {
        return lhs.identifier == rhs.identifier && lhs.version == rhs.version;
    }

    template <typename T>
    constexpr bool operator!=(const entity<T>& lhs, const entity<T>& rhs) noexcept
    {
        return lhs.identifier != rhs.identifier || lhs.version != rhs.version;
    }

    template <typename T>
    struct entity_traits
    {
    };
    
    template <>
    struct entity_traits<u32>
    {
        using type = u32;
        using version_type = u16;
        using identifier_type = u32;

        static constexpr u32 identifier_mask = 0xFFFFF8;
        static constexpr u32 version_mask = 0xFF8;
        static constexpr u32 version_shift = 22;

        static constexpr type to_type(const entity<type> entity)
        {
            const type ver = entity.version & ~version_mask;
            const type id = entity.identifier & ~identifier_mask;
            return (ver << version_shift) | id;
        }

        static constexpr entity<type> from_type(const type value)
        {
	        const entity<type> entity = {
                .identifier = static_cast<identifier_type>(value) & identifier_mask,
                .version = static_cast<version_type>(value >> version_shift & version_mask)
            };

            return entity;
        }
    };

    template <>
    struct entity_traits<u64>
    {
        using type = u64;
        using version_type = u32;
        using identifier_type = u32;

        static constexpr u32 identifier_mask = 0xFFFFFFFF;
        static constexpr u32 version_mask = 0xFFFFFFFF;
        static constexpr u32 version_shift = 32;

        static constexpr type to_type(const entity<type> entity)
        {
            const type ver = entity.version & ~version_mask;
            const type id = entity.identifier & ~identifier_mask;
            return (ver << version_shift) | id;
        }

        static constexpr entity<type> from_type(const type value)
        {
            const entity<type> entity = {
                .identifier = static_cast<identifier_type>(value) & identifier_mask,
                .version = static_cast<version_type>(value >> version_shift & version_mask)
            };

            return entity;
        }
    };
} // namespace entities

#endif // entity_hpp__
