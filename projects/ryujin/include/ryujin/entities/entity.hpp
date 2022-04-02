#ifndef entity_hpp__
#define entity_hpp__

#include <cstddef>
#include <cstdint>

namespace ryujin
{
    template <typename T>
    struct entity
    {
    };

    template <>
    struct entity<std::uint32_t>
    {
        using type = std::uint32_t;

        std::uint32_t identifier : 22;
        std::uint16_t version : 10;
    };

    template <>
    struct entity<std::uint64_t>
    {
        using type = std::uint64_t;

        std::uint32_t identifier;
        std::uint32_t version;
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
    struct entity_traits<std::uint32_t>
    {
        using type = std::uint32_t;
        using version_type = std::uint16_t;
        using identifier_type = std::uint32_t;

        static constexpr std::uint32_t identifier_mask = 0xFFFFF8;
        static constexpr std::uint32_t version_mask = 0xFF8;
        static constexpr std::uint32_t version_shift = 22;

        static constexpr type to_type(const entity<type> entity)
        {
            const type ver = entity.version & ~version_mask;
            const type id = entity.identifier & ~identifier_mask;
            return (ver << version_shift) | id;
        }

        static constexpr entity<type> from_type(const type value)
        {
            entity<type> entity = {};
            entity.identifier = value & identifier_mask;
            entity.version = (value >> version_shift) & version_mask;

            return entity;
        }
    };

    template <>
    struct entity_traits<std::uint64_t>
    {
        using type = std::uint64_t;
        using version_type = std::uint32_t;
        using identifier_type = std::uint32_t;

        static constexpr std::uint32_t identifier_mask = 0xFFFFFFFF;
        static constexpr std::uint32_t version_mask = 0xFFFFFFFF;
        static constexpr std::uint32_t version_shift = 32;

        static constexpr type to_type(const entity<type> entity)
        {
            const type ver = entity.version & ~version_mask;
            const type id = entity.identifier & ~identifier_mask;
            return (ver << version_shift) | id;
        }

        static constexpr entity<type> from_type(const type value)
        {
            entity<type> entity = {};
            entity.identifier = value & identifier_mask;
            entity.version = (value >> version_shift) & version_mask;

            return entity;
        }
    };
} // namespace entities

#endif // entity_hpp__
