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

private:
    StorageType m_value;
};


template<typename StorageType, typename Tag>
constexpr bool operator==(Id<StorageType, Tag> l, Id<StorageType, Tag> r)
{
    return l.get() == r.get();
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

struct RoomIdTag
{
};

using RoomId = Id<uint16_t, RoomIdTag>;

struct RoomGroupIdTag
{
};

struct AnimStateIdTag
{
};

struct TextureProxyIdTag
{
};

struct TextureIdTag
{
};

struct MeshIdTag
{
};

using MeshId = Id<uint32_t, MeshIdTag>;

struct StaticMeshIdTag
{
};

using StaticMeshId = Id<uint32_t, StaticMeshIdTag>;

struct SampleIdTag
{
};

using SampleId = Id<uint32_t, SampleIdTag>;

struct BoxIdTag
{
};

struct SpriteInstanceIdTag
{
};

struct ItemIdTag
{
};
}

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