#pragma once

#include "type_safe/integer.hpp"
#include "type_safe/types.hpp"

#include <yaml-cpp/yaml.h>

#include <string>
#include <type_traits>

namespace core
{
template<typename BaseUnit, int Exponent>
struct UnitExp
{
    static std::string suffix()
    {
        return std::string( BaseUnit::suffix() ) + "^" + std::to_string( Exponent );
    }

    static std::string typeId()
    {
        return std::string( BaseUnit::typeId() ) + "^" + std::to_string( Exponent );
    }
};


template<typename Unit, typename IntType = int32_t>
struct IntQuantity
{
    static_assert( std::is_integral<IntType>::value, "IntType must be int" );

    using unit = Unit;
    using int_type = IntType;
    int_type value;

    constexpr explicit IntQuantity(int_type value) noexcept
            : value{value}
    {}

    template<typename T>
    explicit IntQuantity(T) = delete;

    std::string toString() const
    {
        return std::to_string( value ) + Unit::suffix();
    }
};


template<typename Unit, typename IntType>
constexpr IntQuantity<Unit, IntType> operator-(IntQuantity<Unit, IntType> l, IntQuantity<Unit, IntType> r) noexcept
{
    return IntQuantity<Unit, IntType>{static_cast<IntType>(l.value - r.value)};
}

template<typename Unit, typename IntType>
constexpr IntQuantity<Unit, IntType>& operator-=(IntQuantity<Unit, IntType>& l, IntQuantity<Unit, IntType> r) noexcept
{
    l.value -= r.value;
    return l;
}

template<typename Unit, typename IntType>
constexpr IntQuantity<Unit, IntType> operator+(IntQuantity<Unit, IntType> l, IntQuantity<Unit, IntType> r) noexcept
{
    return IntQuantity<Unit, IntType>{static_cast<IntType>(l.value + r.value)};
}

template<typename Unit, typename IntType>
constexpr IntQuantity<Unit, IntType>& operator+=(IntQuantity<Unit, IntType>& l, IntQuantity<Unit, IntType> r) noexcept
{
    l.value += r.value;
    return l;
}

template<typename Unit, typename IntType>
constexpr typename IntQuantity<Unit, IntType>::int_type
operator/(IntQuantity<Unit, IntType> l, IntQuantity<Unit, IntType> r) noexcept
{
    return l.value / r.value;
}

template<typename Unit, typename IntType>
constexpr IntQuantity<Unit, IntType>
operator/(IntQuantity<Unit, IntType> l, const typename IntQuantity<Unit, IntType>::int_type& r) noexcept
{
    return IntQuantity<Unit, IntType>{l.value / r};
}

template<typename Unit, typename IntType>
constexpr IntQuantity<Unit, IntType>&
operator/=(IntQuantity<Unit, IntType>& l, typename IntQuantity<Unit, IntType>::int_type r) noexcept
{
    l.value /= r;
    return l;
}

template<typename Unit, typename IntType>
constexpr IntQuantity<Unit, IntType>
operator*(IntQuantity<Unit, IntType> l, typename IntQuantity<Unit, IntType>::int_type r) noexcept
{
    return IntQuantity<Unit, IntType>{l.value * r};
}

template<typename Unit, typename IntType>
constexpr IntQuantity<Unit, IntType> operator*(IntQuantity<Unit, IntType> l, float r) noexcept
{
    return IntQuantity<Unit, IntType>{static_cast<typename IntQuantity<Unit, IntType>::int_type>(l.value * r)};
}

template<typename Unit, typename IntType>
constexpr IntQuantity<Unit, IntType>&
operator*=(IntQuantity<Unit, IntType>& l, typename IntQuantity<Unit, IntType>::int_type r) noexcept
{
    l.value *= r;
    return l;
}

template<typename Unit, typename IntType>
constexpr std::enable_if_t<std::is_signed<IntType>::value, IntQuantity<Unit, IntType>>
operator-(IntQuantity<Unit, IntType> l) noexcept
{
    return IntQuantity<Unit, IntType>{static_cast<IntType>(-l.value)};
}

template<typename Unit, typename IntType>
constexpr IntQuantity<Unit, IntType> operator+(IntQuantity<Unit, IntType> l) noexcept
{
    return l;
}

template<typename Unit, typename IntType>
constexpr IntQuantity<Unit, IntType> operator%(IntQuantity<Unit, IntType> l, IntQuantity<Unit, IntType> r) noexcept
{
    return IntQuantity<Unit, IntType>{l.value % r.value};
}

template<typename Unit, typename IntType>
constexpr bool operator<(IntQuantity<Unit, IntType> l, IntQuantity<Unit, IntType> r) noexcept
{
    return l.value < r.value;
}

template<typename Unit, typename IntType>
constexpr bool operator<=(IntQuantity<Unit, IntType> l, IntQuantity<Unit, IntType> r) noexcept
{
    return l.value <= r.value;
}

template<typename Unit, typename IntType>
constexpr bool operator==(IntQuantity<Unit, IntType> l, IntQuantity<Unit, IntType> r) noexcept
{
    return l.value == r.value;
}

template<typename Unit, typename IntType>
constexpr bool operator>(IntQuantity<Unit, IntType> l, IntQuantity<Unit, IntType> r) noexcept
{
    return l.value > r.value;
}

template<typename Unit, typename IntType>
constexpr bool operator>=(IntQuantity<Unit, IntType> l, IntQuantity<Unit, IntType> r) noexcept
{
    return l.value >= r.value;
}

template<typename Unit, typename IntType>
constexpr bool operator!=(IntQuantity<Unit, IntType> l, IntQuantity<Unit, IntType> r) noexcept
{
    return l.value != r.value;
}

// exponents
template<typename Unit, typename IntType>
constexpr IntQuantity<UnitExp<Unit, 2>> operator*(IntQuantity<Unit, IntType> a, IntQuantity<Unit, IntType> b)
{
    return IntQuantity<UnitExp<Unit, 2>>{a.value * b.value};
}

template<typename Unit, typename IntType, int Exp>
constexpr IntQuantity<UnitExp<Unit, Exp + 1>, IntType>
operator*(IntQuantity<UnitExp<Unit, Exp>, IntType> a, IntQuantity<Unit, IntType> b)
{
    return IntQuantity<UnitExp<Unit, Exp + 1>, IntType>{a.value * b.value};
}

template<typename Unit, typename IntType, int Exp>
constexpr IntQuantity<UnitExp<Unit, 2>, IntType>
operator*(IntQuantity<Unit, IntType> a, IntQuantity<UnitExp<Unit, Exp>, IntType> b)
{
    return IntQuantity<UnitExp<Unit, IntType, Exp + 1>>{a.value * b.value};
}

template<typename Unit, typename IntType, int Exp>
constexpr IntQuantity<UnitExp<Unit, Exp - 1>, IntType>
operator/(IntQuantity<UnitExp<Unit, Exp>, IntType> a, IntQuantity<Unit, IntType> b)
{
    return IntQuantity<UnitExp<Unit, IntType, Exp - 1>>{a.value / b.value};
}

template<typename Unit, typename IntType>
constexpr IntQuantity<Unit, IntType> operator/(IntQuantity<UnitExp<Unit, 2>, IntType> a, IntQuantity<Unit, IntType> b)
{
    return IntQuantity<Unit, IntType>{a.value / b.value};
}

template<typename Unit, typename IntType>
constexpr core::IntQuantity<Unit, IntType>
operator*(typename core::IntQuantity<Unit, IntType>::int_type l, core::IntQuantity<Unit, IntType> r)
{
    return core::IntQuantity<Unit, IntType>{l * r.value};
}

template<typename Unit, typename IntType>
constexpr core::IntQuantity<Unit, IntType> operator*(float l, core::IntQuantity<Unit, IntType> r)
{
    return core::IntQuantity<Unit, IntType>{static_cast<core::IntQuantity<Unit, IntType>::int_type>(l * r.value)};
}

template<typename Unit, typename IntType>
constexpr std::enable_if_t<std::is_signed<IntType>::value, IntQuantity<Unit, IntType>>
abs(const IntQuantity<Unit, IntType>& v) noexcept
{
    return v.value >= 0 ? v : -v;
}

template<typename Unit, typename IntType>
constexpr std::enable_if_t<!std::is_signed<IntType>::value, IntQuantity<Unit, IntType>>
abs(const IntQuantity<Unit, IntType>& v) noexcept
{
    return v;
}
}

namespace YAML
{
template<typename Unit, typename IntType>
struct convert<core::IntQuantity<Unit, IntType>>
{
    static Node encode(const core::IntQuantity<Unit, IntType>& rhs)
    {
        Node node{NodeType::Sequence};
        node.SetStyle( YAML::EmitterStyle::Flow );
        node.push_back( Unit::typeId() );
        node.push_back( rhs.value );
        return node;
    }

    static bool decode(const Node& node, core::IntQuantity<Unit, IntType>& rhs)
    {
        if( !node.IsSequence() )
            return false;
        if( node.size() != 2 )
            return false;
        if( node[0].as<std::string>() != Unit::typeId() )
            return false;

        rhs.value = node[1].as<core::IntQuantity<Unit, IntType>::int_type>();
        return true;
    }
};


template<typename Unit, typename IntType>
struct as_if<core::IntQuantity<Unit, IntType>, void>
{
    explicit as_if(const Node& node_) : node{node_}
    {}

    const Node& node;

    core::IntQuantity<Unit, IntType> operator()() const
    {
        if( !node.m_pNode )
            throw TypedBadConversion<core::IntQuantity<Unit, IntType>>{node.Mark()};

        core::IntQuantity<Unit, IntType> t{IntType{0}};
        if( convert<core::IntQuantity<Unit, IntType>>::decode( node, t ) )
            return t;
        throw TypedBadConversion<core::IntQuantity<Unit, IntType>>{node.Mark()};
    }
};
}
