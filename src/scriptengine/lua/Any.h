#pragma once

#include "LuaPrimitives.h"
#include "Traits.h"

#include <memory>


namespace lua
{
namespace detail
{
class Pushable
{
public:
    virtual int push(lua_State* state) const = 0;

    virtual ~Pushable() = default;
};


template<typename... Args>
class TupleHolder : public Pushable
{
    using Tuple = std::tuple<Args...>;
public:
    TupleHolder(Tuple&& tuple)
        : m_tuple(std::forward<Tuple>(tuple))
    {
    }


    TupleHolder(const Tuple& tuple)
        : m_tuple(tuple)
    {
    }


    TupleHolder(Args&&... args)
        : m_tuple(std::forward<Args>(args)...)
    {
    }


    int push(lua_State* state) const override
    {
        return traits::ValueTraits<Tuple>::push(state, m_tuple);
    }


private:
    Tuple m_tuple;
};
}


enum class Type
{
    Nil,
    Number,
    Integer,
    Unsigned,
    Boolean,
    String,
    Tuple
};


class Any
{
public:
    Any(Nil = nullptr)
    {
    }


    Any(const long double nbr)
        : number{static_cast<Number>(nbr)}
        , m_type{Type::Number}
    {
    }


    Any(const double nbr)
        : number{nbr}
        , m_type{Type::Number}
    {
    }


    Any(const float nbr)
        : number{nbr}
        , m_type{Type::Number}
    {
    }


    Any(const long long nbr)
        : integer{nbr}
        , m_type{Type::Integer}
    {
    }


    Any(const long nbr)
        : integer{nbr}
        , m_type{Type::Integer}
    {
    }


    Any(const int nbr)
        : integer{nbr}
        , m_type{Type::Integer}
    {
    }


    Any(const short nbr)
        : integer{nbr}
        , m_type{Type::Integer}
    {
    }


    Any(const signed char nbr)
        : integer{nbr}
        , m_type{Type::Integer}
    {
    }


    Any(const unsigned long long nbr)
        : uinteger{nbr}
        , m_type{Type::Unsigned}
    {
    }


    Any(const unsigned long nbr)
        : uinteger{nbr}
        , m_type{Type::Unsigned}
    {
    }


    Any(const unsigned int nbr)
        : uinteger{nbr}
        , m_type{Type::Unsigned}
    {
    }


    Any(const unsigned short nbr)
        : uinteger{nbr}
        , m_type{Type::Unsigned}
    {
    }


    Any(const unsigned char nbr)
        : uinteger{nbr}
        , m_type{Type::Unsigned}
    {
    }


    Any(const bool b)
        : boolean{b}
        , m_type{Type::Boolean}
    {
    }


    Any(const char* str)
        : string{str}
        , m_type{str == nullptr ? Type::Nil : Type::String}
    {
    }


    Any(std::string&& str)
        : string{std::forward<std::string>(str)}
        , m_type{Type::String}
    {
    }


    Any(const std::string& str)
        : string{str}
        , m_type{Type::String}
    {
    }


    template<typename T0, typename T1, typename... Ts>
    Any(T0&& arg0, T1&& arg1, Ts&&... args)
        : tuple{
            std::make_unique<detail::TupleHolder<T0, T1, Ts...>>(
                std::forward<T0>(arg0),
                std::forward<T1>(arg1),
                std::forward<Ts>(args)...)
        }
        , m_type{Type::Tuple}
    {
    }


    template<typename... Args>
    Any(std::tuple<Args...>&& args)
        : tuple{std::make_unique<detail::TupleHolder<Args...>>(std::forward<std::tuple<Args...>>(args))}
        , m_type{Type::Tuple}
    {
    }


    int push(lua_State* state) const
    {
        switch( m_type )
        {
            case Type::Number:
                return traits::ValueTraits<Number>::push(state, number);
            case Type::Integer:
                return traits::ValueTraits<Integer>::push(state, integer);
            case Type::Unsigned:
                return traits::ValueTraits<Unsigned>::push(state, uinteger);
            case Type::Boolean:
                return traits::ValueTraits<Boolean>::push(state, boolean);
            case Type::String:
                return traits::ValueTraits<String>::push(state, string.c_str());
            case Type::Tuple:
                return tuple->push(state);

            case Type::Nil:
            default:
                return traits::ValueTraits<Nil>::push(state, nullptr);
        }
    }


private:
    union
    {
        Number number;

        Integer integer;

        Unsigned uinteger;

        Boolean boolean;
    };


    std::string string{};

    std::unique_ptr<detail::Pushable> tuple;

    Type m_type = Type::Nil;
};


namespace traits
{
template<>
struct ValueTraits<Any>
{
    static int push(lua_State* luaState, const Any& any) noexcept
    {
        return any.push(luaState);
    }
};
}
}
