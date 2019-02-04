#pragma once

#include "type_safe/integer.hpp"
#include "type_safe/types.hpp"

#include <yaml-cpp/yaml.h>

#include <string>

namespace core
{
template<typename Tag, int Multiplicity>
struct MultiTag
{
    static std::string suffix()
    {
        return std::string( Tag::suffix() ) + "^" + std::to_string( Multiplicity );
    }

    static std::string typeId()
    {
        return std::string( Tag::typeId() ) + "^" + std::to_string( Multiplicity );
    }
};


template<typename Tag>
struct IntUnit
{
    using tag = Tag;
    using int_type = int32_t;
    int_type value;

    constexpr explicit IntUnit(int_type value) noexcept
            : value{value}
    {}

    template<typename T>
    explicit IntUnit(T) = delete;

    std::string toString() const
    {
        return std::to_string( value ) + Tag::suffix();
    }
};


template<typename Tag>
constexpr IntUnit<Tag> operator-(IntUnit<Tag> l, IntUnit<Tag> r) noexcept
{
    return IntUnit<Tag>{l.value - r.value};
}

template<typename Tag>
inline IntUnit<Tag>& operator-=(IntUnit<Tag>& l, IntUnit<Tag> r) noexcept
{
    l.value -= r.value;
    return l;
}

template<typename Tag>
constexpr IntUnit<Tag> operator+(IntUnit<Tag> l, IntUnit<Tag> r) noexcept
{
    return IntUnit<Tag>{l.value + r.value};
}

template<typename Tag>
inline IntUnit<Tag>& operator+=(IntUnit<Tag>& l, IntUnit<Tag> r) noexcept
{
    l.value += r.value;
    return l;
}

template<typename Tag>
constexpr typename IntUnit<Tag>::int_type operator/(IntUnit<Tag> l, IntUnit<Tag> r) noexcept
{
    return l.value / r.value;
}

template<typename Tag>
constexpr IntUnit<Tag> operator/(IntUnit<Tag> l, const typename IntUnit<Tag>::int_type& r) noexcept
{
    return IntUnit<Tag>{l.value / r};
}

template<typename Tag>
inline IntUnit<Tag>& operator/=(IntUnit<Tag>& l, typename IntUnit<Tag>::int_type r) noexcept
{
    l.value /= r;
    return l;
}

template<typename Tag>
constexpr IntUnit<Tag> operator*(IntUnit<Tag> l, typename IntUnit<Tag>::int_type r) noexcept
{
    return IntUnit<Tag>{l.value * r};
}

template<typename Tag>
constexpr IntUnit<Tag> operator*(IntUnit<Tag> l, float r) noexcept
{
    return IntUnit<Tag>{static_cast<typename IntUnit<Tag>::int_type>(l.value * r)};
}

template<typename Tag>
inline IntUnit<Tag>& operator*=(IntUnit<Tag>& l, typename IntUnit<Tag>::int_type r) noexcept
{
    l.value *= r;
    return l;
}

template<typename Tag>
constexpr IntUnit<Tag> operator-(IntUnit<Tag> l) noexcept
{
    return IntUnit<Tag>{-l.value};
}

template<typename Tag>
constexpr IntUnit<Tag> operator+(IntUnit<Tag> l) noexcept
{
    return l;
}

template<typename Tag>
constexpr IntUnit<Tag> operator%(IntUnit<Tag> l, IntUnit<Tag> r) noexcept
{
    return IntUnit<Tag>{l.value % r.value};
}

template<typename Tag>
constexpr bool operator<(IntUnit<Tag> l, IntUnit<Tag> r) noexcept
{
    return l.value < r.value;
}

template<typename Tag>
constexpr bool operator<=(IntUnit<Tag> l, IntUnit<Tag> r) noexcept
{
    return l.value <= r.value;
}

template<typename Tag>
constexpr bool operator==(IntUnit<Tag> l, IntUnit<Tag> r) noexcept
{
    return l.value == r.value;
}

template<typename Tag>
constexpr bool operator>(IntUnit<Tag> l, IntUnit<Tag> r) noexcept
{
    return l.value > r.value;
}

template<typename Tag>
constexpr bool operator>=(IntUnit<Tag> l, IntUnit<Tag> r) noexcept
{
    return l.value >= r.value;
}

template<typename Tag>
constexpr bool operator!=(IntUnit<Tag> l, IntUnit<Tag> r) noexcept
{
    return l.value != r.value;
}

// multiplicity
template<typename Tag>
constexpr IntUnit<MultiTag<Tag, 2>> operator*(IntUnit<Tag> a, IntUnit<Tag> b)
{
    return IntUnit<MultiTag<Tag, 2>>{a.value * b.value};
}

template<typename Tag, int N1>
constexpr IntUnit<MultiTag<Tag, N1 + 1>> operator*(IntUnit<MultiTag<Tag, N1>> a, IntUnit<Tag> b)
{
    return IntUnit<MultiTag<Tag, N1 + 1>>{a.value * b.value};
}

template<typename Tag, int N2>
constexpr IntUnit<MultiTag<Tag, 2>> operator*(IntUnit<Tag> a, IntUnit<MultiTag<Tag, N2>> b)
{
    return IntUnit<MultiTag<Tag, N2 + 1>>{a.value * b.value};
}

template<typename Tag, int N1>
constexpr IntUnit<MultiTag<Tag, N1 - 1>> operator/(IntUnit<MultiTag<Tag, N1>> a, IntUnit<Tag> b)
{
    return IntUnit<MultiTag<Tag, N1 - 1>>{a.value / b.value};
}

template<typename Tag>
constexpr IntUnit<Tag> operator/(IntUnit<MultiTag<Tag, 2>> a, IntUnit<Tag> b)
{
    return IntUnit<Tag>{a.value / b.value};
}

template<typename Tag>
constexpr core::IntUnit<Tag> operator*(typename core::IntUnit<Tag>::int_type l, core::IntUnit<Tag> r)
{
    return core::IntUnit<Tag>{l * r.value};
}

template<typename Tag>
constexpr core::IntUnit<Tag> operator*(float l, core::IntUnit<Tag> r)
{
    return core::IntUnit<Tag>{static_cast<core::IntUnit<Tag>::int_type>(l * r.value)};
}
}

namespace YAML
{
template<typename Tag>
struct convert<core::IntUnit<Tag>>
{
    static Node encode(const core::IntUnit<Tag>& rhs)
    {
        Node node( NodeType::Sequence );
        node.push_back( Tag::typeId() );
        node.push_back( rhs.value );
        return node;
    }

    static bool decode(const Node& node, core::IntUnit<Tag>& rhs)
    {
        if( !node.IsSequence() )
            return false;
        if( node.size() != 2 )
            return false;
        if( node[0].as<std::string>() != Tag::typeId() )
            return false;

        rhs.value = node[1].as<core::IntUnit<Tag>::int_type>();
        return true;
    }
};


template<typename Tag>
struct as_if<core::IntUnit<Tag>, void>
{
    explicit as_if(const Node& node_) : node( node_ )
    {}

    const Node& node;

    core::IntUnit<Tag> operator()() const
    {
        if( !node.m_pNode )
            throw TypedBadConversion<core::IntUnit<Tag>>( node.Mark() );

        core::IntUnit<Tag> t{0};
        if( convert<core::IntUnit<Tag>>::decode( node, t ) )
            return t;
        throw TypedBadConversion<core::IntUnit<Tag>>( node.Mark() );
    }
};
}
