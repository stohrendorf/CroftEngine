#pragma once

#include <cstdint>
#include <functional>

namespace core
{
/**
 * @brief A simple wrapper for specifying something without arithmetic support.
 * @tparam StorageType ID type
 * @tparam Tag Tag for avoiding mixing different IDs with the same storage type
 */
template<typename StorageType, typename Tag>
struct Id
{
    using type = StorageType;
    using tag = Tag;

    constexpr explicit Id(type value)
            : m_value{value}
    {}

    template<typename T>
    constexpr Id(T) = delete;

    constexpr auto& operator=(type value)
    {
        m_value = value;
        return *this;
    }

    template<typename T>
    constexpr auto& operator=(T value) = delete;

    constexpr explicit operator type() const
    {
        return m_value;
    }

    constexpr type get() const
    {
        return m_value;
    }

    template<typename EType>
    constexpr typename std::enable_if<std::is_enum<EType>::value, EType>::type as() const
    {
        return static_cast<EType>(m_value);
    }

private:
    StorageType m_value;
};


template<typename StorageType, typename Tag>
constexpr bool operator==(Id<StorageType, Tag> l, Id<StorageType, Tag> r)
{
    return l.get() == r.get();
}

template<typename StorageType, typename Tag, typename EType>
constexpr typename std::enable_if<std::is_enum<EType>::value, bool>::type operator==(Id<StorageType, Tag> l, EType r)
{
    return l.get() == static_cast<StorageType>(r);
}

template<typename StorageType, typename Tag>
constexpr bool operator!=(Id<StorageType, Tag> l, Id<StorageType, Tag> r)
{
    return l.get() != r.get();
}

template<typename StorageType, typename Tag>
constexpr bool operator<(Id<StorageType, Tag> l, Id<StorageType, Tag> r)
{
    return l.get() < r.get();
}

#define DECLARE_ID(name, type) \
    struct name ## _generated_tag {}; \
    using name = ::core::Id<type, name ## _generated_tag>

DECLARE_ID( RoomId8, uint8_t );
DECLARE_ID( RoomId16, uint16_t );
DECLARE_ID( RoomIdI16, int16_t );
DECLARE_ID( RoomId32, uint32_t );
DECLARE_ID( RoomGroupId, uint8_t );
DECLARE_ID( AnimStateId, uint16_t );
DECLARE_ID( TextureProxyId, uint16_t );
DECLARE_ID( TextureId, uint16_t );
DECLARE_ID( MeshId, uint32_t );
DECLARE_ID( StaticMeshId, uint32_t );
DECLARE_ID( SampleId, uint32_t );
DECLARE_ID( BoxId, int16_t );
DECLARE_ID( SpriteInstanceId, uint16_t );
DECLARE_ID( ItemId, uint16_t );

DECLARE_ID( TypeId, uint16_t );
DECLARE_ID( SoundId, uint16_t );

inline constexpr AnimStateId operator "" _as(unsigned long long value)
{
    return AnimStateId{static_cast<AnimStateId::type>(value)};
}
}

using core::operator ""_as;

namespace std
{
template<typename StorageType, typename Tag>
struct hash<core::Id<StorageType, Tag>>
{
    constexpr size_t operator()(const core::Id<StorageType, Tag>& v) const
    {
        return hash<StorageType>{}( v.get() );
    }
};
}