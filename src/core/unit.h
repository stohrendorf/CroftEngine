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
};


template<typename Unit, typename Type = int32_t>
struct Quantity
{
    using unit = Unit;
    using type = Type;
    type value;

    constexpr explicit Quantity(type value) noexcept
            : value{value}
    {}

    template<typename T>
    explicit Quantity(T) = delete;

    std::string toString() const
    {
        return std::to_string( value ) + Unit::suffix();
    }

    template<typename T>
    constexpr Quantity<Unit, T> cast() const
    {
        return Quantity<Unit, T>{static_cast<T>(value)};
    }

    template<typename Q>
    constexpr Quantity<Unit, typename Q::type> as() const
    {
        static_assert( std::is_same_v<typename Q::unit, Unit>, "Unit mismatch" );
        return Quantity<Unit, typename Q::type>{static_cast<typename Q::type>(value)};
    }
};


template<typename Unit, typename Type>
constexpr Quantity<Unit, Type> operator-(Quantity<Unit, Type> l, Quantity<Unit, Type> r) noexcept
{
    return Quantity<Unit, Type>{static_cast<Type>(l.value - r.value)};
}

template<typename Unit, typename Type>
constexpr Quantity<Unit, Type>& operator-=(Quantity<Unit, Type>& l, Quantity<Unit, Type> r) noexcept
{
    l.value -= r.value;
    return l;
}

template<typename Unit, typename Type>
constexpr Quantity<Unit, Type> operator+(Quantity<Unit, Type> l, Quantity<Unit, Type> r) noexcept
{
    return Quantity<Unit, Type>{static_cast<Type>(l.value + r.value)};
}

template<typename Unit, typename Type>
constexpr Quantity<Unit, Type>& operator+=(Quantity<Unit, Type>& l, Quantity<Unit, Type> r) noexcept
{
    l.value += r.value;
    return l;
}

template<typename Unit, typename Type>
constexpr typename Quantity<Unit, Type>::type
operator/(Quantity<Unit, Type> l, Quantity<Unit, Type> r) noexcept
{
    return l.value / r.value;
}

template<typename Unit, typename Type>
constexpr Quantity<Unit, Type>
operator/(Quantity<Unit, Type> l, const typename Quantity<Unit, Type>::type& r) noexcept
{
    return Quantity<Unit, Type>{l.value / r};
}

template<typename Unit, typename Type>
constexpr Quantity<Unit, Type>&
operator/=(Quantity<Unit, Type>& l, typename Quantity<Unit, Type>::type r) noexcept
{
    l.value /= r;
    return l;
}

template<typename Unit, typename Type>
constexpr Quantity<Unit, Type>
operator*(Quantity<Unit, Type> l, typename Quantity<Unit, Type>::type r) noexcept
{
    return Quantity<Unit, Type>{l.value * r};
}

template<typename Unit, typename Type>
constexpr Quantity<Unit, Type> operator*(Quantity<Unit, Type> l, float r) noexcept
{
    return Quantity<Unit, Type>{static_cast<typename Quantity<Unit, Type>::type>(l.value * r)};
}

template<typename Unit, typename Type>
constexpr Quantity<Unit, Type>&
operator*=(Quantity<Unit, Type>& l, typename Quantity<Unit, Type>::type r) noexcept
{
    l.value *= r;
    return l;
}

template<typename Unit, typename Type>
constexpr std::enable_if_t<std::is_signed<Type>::value, Quantity<Unit, Type>>
operator-(Quantity<Unit, Type> l) noexcept
{
    return Quantity<Unit, Type>{static_cast<Type>(-l.value)};
}

template<typename Unit, typename Type>
constexpr Quantity<Unit, Type> operator+(Quantity<Unit, Type> l) noexcept
{
    return l;
}

template<typename Unit, typename Type>
constexpr Quantity<Unit, Type> operator%(Quantity<Unit, Type> l, Quantity<Unit, Type> r) noexcept
{
    return Quantity<Unit, Type>{l.value % r.value};
}

template<typename Unit, typename Type>
constexpr bool operator<(Quantity<Unit, Type> l, Quantity<Unit, Type> r) noexcept
{
    return l.value < r.value;
}

template<typename Unit, typename Type>
constexpr bool operator<=(Quantity<Unit, Type> l, Quantity<Unit, Type> r) noexcept
{
    return l.value <= r.value;
}

template<typename Unit, typename Type>
constexpr bool operator==(Quantity<Unit, Type> l, Quantity<Unit, Type> r) noexcept
{
    return l.value == r.value;
}

template<typename Unit, typename Type>
constexpr bool operator>(Quantity<Unit, Type> l, Quantity<Unit, Type> r) noexcept
{
    return l.value > r.value;
}

template<typename Unit, typename Type>
constexpr bool operator>=(Quantity<Unit, Type> l, Quantity<Unit, Type> r) noexcept
{
    return l.value >= r.value;
}

template<typename Unit, typename Type>
constexpr bool operator!=(Quantity<Unit, Type> l, Quantity<Unit, Type> r) noexcept
{
    return l.value != r.value;
}

// exponents
template<typename Unit, typename Type>
constexpr Quantity<UnitExp<Unit, 2>> operator*(Quantity<Unit, Type> a, Quantity<Unit, Type> b)
{
    return Quantity<UnitExp<Unit, 2>>{a.value * b.value};
}

template<typename Unit, typename Type, int Exp>
constexpr Quantity<UnitExp<Unit, Exp + 1>, Type>
operator*(Quantity<UnitExp<Unit, Exp>, Type> a, Quantity<Unit, Type> b)
{
    return Quantity<UnitExp<Unit, Exp + 1>, Type>{a.value * b.value};
}

template<typename Unit, typename Type, int Exp>
constexpr Quantity<UnitExp<Unit, 2>, Type>
operator*(Quantity<Unit, Type> a, Quantity<UnitExp<Unit, Exp>, Type> b)
{
    return Quantity<UnitExp<Unit, Type, Exp + 1>>{a.value * b.value};
}

template<typename Unit, typename Type, int Exp>
constexpr Quantity<UnitExp<Unit, Exp - 1>, Type>
operator/(Quantity<UnitExp<Unit, Exp>, Type> a, Quantity<Unit, Type> b)
{
    return Quantity<UnitExp<Unit, Type, Exp - 1>>{a.value / b.value};
}

template<typename Unit, typename Type>
constexpr Quantity<Unit, Type> operator/(Quantity<UnitExp<Unit, 2>, Type> a, Quantity<Unit, Type> b)
{
    return Quantity<Unit, Type>{a.value / b.value};
}

template<typename Unit, typename Type>
constexpr core::Quantity<Unit, Type>
operator*(typename core::Quantity<Unit, Type>::type l, core::Quantity<Unit, Type> r)
{
    return core::Quantity<Unit, Type>{l * r.value};
}

template<typename Unit, typename Type>
constexpr core::Quantity<Unit, Type> operator*(float l, core::Quantity<Unit, Type> r)
{
    return core::Quantity<Unit, Type>{static_cast<core::Quantity<Unit, Type>::type>(l * r.value)};
}

template<typename Unit, typename Type>
constexpr std::enable_if_t<std::is_signed<Type>::value, Quantity<Unit, Type>>
abs(const Quantity<Unit, Type>& v) noexcept
{
    return v.value >= 0 ? v : -v;
}

template<typename Unit, typename Type>
constexpr std::enable_if_t<!std::is_signed<Type>::value, Quantity<Unit, Type>>
abs(const Quantity<Unit, Type>& v) noexcept
{
    return v;
}
}

namespace YAML
{
template<typename Unit, typename Type>
struct convert<core::Quantity<Unit, Type>>
{
    static Node encode(const core::Quantity<Unit, Type>& rhs)
    {
        Node node{NodeType::Sequence};
        node.SetStyle( YAML::EmitterStyle::Flow );
        node.push_back( Unit::suffix() );
        node.push_back( rhs.value );
        return node;
    }

    static bool decode(const Node& node, core::Quantity<Unit, Type>& rhs)
    {
        if( !node.IsSequence() )
            return false;
        if( node.size() != 2 )
            return false;
        if( node[0].as<std::string>() != Unit::suffix() )
            return false;

        rhs.value = node[1].as<core::Quantity<Unit, Type>::type>();
        return true;
    }
};


template<typename Unit, typename Type>
struct as_if<core::Quantity<Unit, Type>, void>
{
    explicit as_if(const Node& node_) : node{node_}
    {}

    const Node& node;

    core::Quantity<Unit, Type> operator()() const
    {
        if( !node.m_pNode )
            throw TypedBadConversion<core::Quantity<Unit, Type>>{node.Mark()};

        core::Quantity<Unit, Type> t{Type{0}};
        if( convert<core::Quantity<Unit, Type>>::decode( node, t ) )
            return t;
        throw TypedBadConversion<core::Quantity<Unit, Type>>{node.Mark()};
    }
};
}
