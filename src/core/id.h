#pragma once

#include "tpl_helper.h"

#include <yaml-cpp/yaml.h>

#include <cstdint>
#include <functional>

namespace engine
{
enum class TR1ItemId;
enum class TR1SoundId;
}

namespace core
{
/**
 * @brief A simple wrapper for specifying something without arithmetic support.
 * @tparam StorageType ID type
 * @tparam Tag Tag for avoiding mixing different IDs with the same storage type
 */
template<typename StorageType, typename Tag, typename... Enums>
struct Id
{
    static_assert( tpl::is_all_enum_v<Enums...>, "Compatible types must be enums" );

    using type = StorageType;
    using tag = Tag;

    constexpr explicit Id(type value)
        : m_value{ value }
    {}

    template<typename T>
    constexpr Id(T value)
        : m_value{ static_cast<type>(value) }
    {
        static_assert( tpl::contains_v<T, Enums...>, "Incompatible type" );
    }

    constexpr auto& operator=(type value)
    {
        m_value = value;
        return *this;
    }

    template<typename T>
    constexpr auto& operator=(T value)
    {
        static_assert( tpl::contains_v<T, Enums...>, "Incompatible type" );
        m_value = static_cast<type>(value);
        return *this;
    }

    constexpr explicit operator type() const
    {
        return m_value;
    }

    constexpr type get() const
    {
        return m_value;
    }

    template<typename T>
    constexpr T get_as() const
    {
        static_assert( tpl::contains_v<T, Enums...>, "Incompatible target type" );
        return static_cast<T>(m_value);
    }

    constexpr bool operator<(const Id<type, tag, Enums...> r) const
    {
        return get() < r.get();
    }

    constexpr bool operator==(const Id<type, tag, Enums...> r) const
    {
        return get() == r.get();
    }

    constexpr bool operator!=(const Id<type, tag, Enums...> r) const
    {
        return get() != r.get();
    }

    template<typename T>
    constexpr bool operator==(const T r) const
    {
        return get_as<T>() == r;
    }

    template<typename T>
    constexpr bool operator!=(const T r) const
    {
        return get_as<T>() != r;
    }

    template<typename T>
    constexpr bool operator<(const T r) const
    {
        return get_as<T>() < r;
    }

    template<typename T>
    constexpr bool operator<=(const T r) const
    {
        return get_as<T>() <= r;
    }

    template<typename T>
    constexpr bool operator>(const T r) const
    {
        return get_as<T>() > r;
    }

    template<typename T>
    constexpr bool operator>=(const T r) const
    {
        return get_as<T>() >= r;
    }

private:
    StorageType m_value;
};

#define DECLARE_ID(name, type) \
    struct name ## _generated_tag {}; \
    using name = ::core::Id<type, name ## _generated_tag>

#define DECLARE_ID_E(name, type, ...) \
    struct name ## _generated_tag {}; \
    using name = ::core::Id<type, name ## _generated_tag, __VA_ARGS__>

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

DECLARE_ID_E( TypeId, uint16_t, engine::TR1ItemId );
DECLARE_ID_E( SoundId, uint16_t, engine::TR1SoundId );

inline constexpr AnimStateId operator "" _as(unsigned long long value)
{
    return AnimStateId{ static_cast<AnimStateId::type>(value) };
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

// YAML converters
namespace YAML
{
template<typename Type, typename Tag, typename... Enums>
struct convert<core::Id<Type, Tag, Enums...>>
{
    static Node encode(const core::Id<Type, Tag, Enums...>& rhs)
    {
        Node node{ NodeType::Scalar };
        node = rhs.get();
        return node;
    }

    static bool decode(const Node& node, core::Id<Type, Tag, Enums...>& rhs)
    {
        if( !node.IsScalar() )
            return false;

        rhs = core::Id<Type, Tag, Enums...>{ node.as<Type>() };
        return true;
    }
};

template<typename Type, typename Tag, typename... Enums>
struct as_if<core::Id<Type, Tag, Enums...>, void>
{
    explicit as_if(const Node& node_)
        : node{ node_ }
    {}

    const Node& node;

    core::Id<Type, Tag, Enums...> operator()() const
    {
        if( !node.m_pNode )
            throw TypedBadConversion<core::Id<Type, Tag, Enums...>>{ node.Mark() };

        core::Id<Type, Tag, Enums...> t{ Type{ 0 } };
        if( convert<core::Id<Type, Tag, Enums...>>::decode( node, t ) )
            return t;
        throw TypedBadConversion<core::Id<Type, Tag, Enums...>>{ node.Mark() };
    }
};
}
