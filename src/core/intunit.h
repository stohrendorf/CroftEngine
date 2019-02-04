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


template<typename Tag, typename IntType = int32_t>
struct IntUnit
{
    static_assert( std::is_integral<IntType>::value, "IntType must be int" );

    using tag = Tag;
    using int_type = IntType;
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


template<typename Tag, typename IntType>
constexpr IntUnit<Tag, IntType> operator-(IntUnit<Tag, IntType> l, IntUnit<Tag, IntType> r) noexcept
{
    return IntUnit<Tag, IntType>{static_cast<IntType>(l.value - r.value)};
}

template<typename Tag, typename IntType>
inline IntUnit<Tag, IntType>& operator-=(IntUnit<Tag, IntType>& l, IntUnit<Tag, IntType> r) noexcept
{
    l.value -= r.value;
    return l;
}

template<typename Tag, typename IntType>
constexpr IntUnit<Tag, IntType> operator+(IntUnit<Tag, IntType> l, IntUnit<Tag, IntType> r) noexcept
{
    return IntUnit<Tag, IntType>{static_cast<IntType>(l.value + r.value)};
}

template<typename Tag, typename IntType>
inline IntUnit<Tag, IntType>& operator+=(IntUnit<Tag, IntType>& l, IntUnit<Tag, IntType> r) noexcept
{
    l.value += r.value;
    return l;
}

template<typename Tag, typename IntType>
constexpr typename IntUnit<Tag, IntType>::int_type operator/(IntUnit<Tag, IntType> l, IntUnit<Tag, IntType> r) noexcept
{
    return l.value / r.value;
}

template<typename Tag, typename IntType>
constexpr IntUnit<Tag, IntType>
operator/(IntUnit<Tag, IntType> l, const typename IntUnit<Tag, IntType>::int_type& r) noexcept
{
    return IntUnit<Tag, IntType>{l.value / r};
}

template<typename Tag, typename IntType>
inline IntUnit<Tag, IntType>& operator/=(IntUnit<Tag, IntType>& l, typename IntUnit<Tag, IntType>::int_type r) noexcept
{
    l.value /= r;
    return l;
}

template<typename Tag, typename IntType>
constexpr IntUnit<Tag, IntType> operator*(IntUnit<Tag, IntType> l, typename IntUnit<Tag, IntType>::int_type r) noexcept
{
    return IntUnit<Tag, IntType>{l.value * r};
}

template<typename Tag, typename IntType>
constexpr IntUnit<Tag, IntType> operator*(IntUnit<Tag, IntType> l, float r) noexcept
{
    return IntUnit<Tag, IntType>{static_cast<typename IntUnit<Tag, IntType>::int_type>(l.value * r)};
}

template<typename Tag, typename IntType>
inline IntUnit<Tag, IntType>& operator*=(IntUnit<Tag, IntType>& l, typename IntUnit<Tag, IntType>::int_type r) noexcept
{
    l.value *= r;
    return l;
}

template<typename Tag, typename IntType>
constexpr IntUnit<Tag, IntType> operator-(IntUnit<Tag, IntType> l) noexcept
{
    return IntUnit<Tag, IntType>{static_cast<IntType>(-l.value)};
}

template<typename Tag, typename IntType>
constexpr IntUnit<Tag, IntType> operator+(IntUnit<Tag, IntType> l) noexcept
{
    return l;
}

template<typename Tag, typename IntType>
constexpr IntUnit<Tag, IntType> operator%(IntUnit<Tag, IntType> l, IntUnit<Tag, IntType> r) noexcept
{
    return IntUnit<Tag, IntType>{l.value % r.value};
}

template<typename Tag, typename IntType>
constexpr bool operator<(IntUnit<Tag, IntType> l, IntUnit<Tag, IntType> r) noexcept
{
    return l.value < r.value;
}

template<typename Tag, typename IntType>
constexpr bool operator<=(IntUnit<Tag, IntType> l, IntUnit<Tag, IntType> r) noexcept
{
    return l.value <= r.value;
}

template<typename Tag, typename IntType>
constexpr bool operator==(IntUnit<Tag, IntType> l, IntUnit<Tag, IntType> r) noexcept
{
    return l.value == r.value;
}

template<typename Tag, typename IntType>
constexpr bool operator>(IntUnit<Tag, IntType> l, IntUnit<Tag, IntType> r) noexcept
{
    return l.value > r.value;
}

template<typename Tag, typename IntType>
constexpr bool operator>=(IntUnit<Tag, IntType> l, IntUnit<Tag, IntType> r) noexcept
{
    return l.value >= r.value;
}

template<typename Tag, typename IntType>
constexpr bool operator!=(IntUnit<Tag, IntType> l, IntUnit<Tag, IntType> r) noexcept
{
    return l.value != r.value;
}

// multiplicity
template<typename Tag, typename IntType>
constexpr IntUnit<MultiTag<Tag, 2>> operator*(IntUnit<Tag, IntType> a, IntUnit<Tag, IntType> b)
{
    return IntUnit<MultiTag<Tag, 2>>{a.value * b.value};
}

template<typename Tag, typename IntType, int N1>
constexpr IntUnit<MultiTag<Tag, N1 + 1>, IntType>
operator*(IntUnit<MultiTag<Tag, N1>, IntType> a, IntUnit<Tag, IntType> b)
{
    return IntUnit<MultiTag<Tag, N1 + 1>, IntType>{a.value * b.value};
}

template<typename Tag, typename IntType, int N2>
constexpr IntUnit<MultiTag<Tag, 2>, IntType> operator*(IntUnit<Tag, IntType> a, IntUnit<MultiTag<Tag, N2>, IntType> b)
{
    return IntUnit<MultiTag<Tag, IntType, N2 + 1>>{a.value * b.value};
}

template<typename Tag, typename IntType, int N1>
constexpr IntUnit<MultiTag<Tag, N1 - 1>, IntType>
operator/(IntUnit<MultiTag<Tag, N1>, IntType> a, IntUnit<Tag, IntType> b)
{
    return IntUnit<MultiTag<Tag, IntType, N1 - 1>>{a.value / b.value};
}

template<typename Tag, typename IntType>
constexpr IntUnit<Tag, IntType> operator/(IntUnit<MultiTag<Tag, 2>, IntType> a, IntUnit<Tag, IntType> b)
{
    return IntUnit<Tag, IntType>{a.value / b.value};
}

template<typename Tag, typename IntType>
constexpr core::IntUnit<Tag, IntType>
operator*(typename core::IntUnit<Tag, IntType>::int_type l, core::IntUnit<Tag, IntType> r)
{
    return core::IntUnit<Tag, IntType>{l * r.value};
}

template<typename Tag, typename IntType>
constexpr core::IntUnit<Tag, IntType> operator*(float l, core::IntUnit<Tag, IntType> r)
{
    return core::IntUnit<Tag, IntType>{static_cast<core::IntUnit<Tag, IntType>::int_type>(l * r.value)};
}
}

namespace YAML
{
template<typename Tag, typename IntType>
struct convert<core::IntUnit<Tag, IntType>>
{
    static Node encode(const core::IntUnit<Tag, IntType>& rhs)
    {
        Node node( NodeType::Sequence );
        node.SetStyle( YAML::EmitterStyle::Flow );
        node.push_back( Tag::typeId() );
        node.push_back( rhs.value );
        return node;
    }

    static bool decode(const Node& node, core::IntUnit<Tag, IntType>& rhs)
    {
        if( !node.IsSequence() )
            return false;
        if( node.size() != 2 )
            return false;
        if( node[0].as<std::string>() != Tag::typeId() )
            return false;

        rhs.value = node[1].as<core::IntUnit<Tag, IntType>::int_type>();
        return true;
    }
};


template<typename Tag, typename IntType>
struct as_if<core::IntUnit<Tag, IntType>, void>
{
    explicit as_if(const Node& node_) : node( node_ )
    {}

    const Node& node;

    core::IntUnit<Tag, IntType> operator()() const
    {
        if( !node.m_pNode )
            throw TypedBadConversion<core::IntUnit<Tag, IntType>>( node.Mark() );

        core::IntUnit<Tag, IntType> t{IntType{0}};
        if( convert<core::IntUnit<Tag, IntType>>::decode( node, t ) )
            return t;
        throw TypedBadConversion<core::IntUnit<Tag, IntType>>( node.Mark() );
    }
};
}
