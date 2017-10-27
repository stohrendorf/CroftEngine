#pragma once

#include "LuaPrimitives.h"

#include <memory>
#include <utility>

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
class TupleHolder final
    : public Pushable
{
    using Tuple = std::tuple<Args...>;
public:
    explicit TupleHolder(Tuple&& tuple)
        : m_tuple(std::forward<Tuple>(tuple))
    {
    }

    explicit TupleHolder(const Tuple& tuple)
        : m_tuple(tuple)
    {
    }

    explicit TupleHolder(Args&& ... args)
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

class Any final
{
public:
    explicit Any(Nil = nullptr)
    {
    }

    explicit Any(const long double nbr)
        : number{static_cast<Number>(nbr)}
          , m_type{Type::Number}
    {
    }

    explicit Any(const double nbr)
        : number{nbr}
          , m_type{Type::Number}
    {
    }

    explicit Any(const float nbr)
        : number{nbr}
          , m_type{Type::Number}
    {
    }

    explicit Any(const long long nbr)
        : integer{nbr}
          , m_type{Type::Integer}
    {
    }

    explicit Any(const long nbr)
        : integer{nbr}
          , m_type{Type::Integer}
    {
    }

    explicit Any(const int nbr)
        : integer{nbr}
          , m_type{Type::Integer}
    {
    }

    explicit Any(const short nbr)
        : integer{nbr}
          , m_type{Type::Integer}
    {
    }

    explicit Any(const signed char nbr)
        : integer{nbr}
          , m_type{Type::Integer}
    {
    }

    explicit Any(const unsigned long long nbr)
        : uinteger{nbr}
          , m_type{Type::Unsigned}
    {
    }

    explicit Any(const unsigned long nbr)
        : uinteger{nbr}
          , m_type{Type::Unsigned}
    {
    }

    explicit Any(const unsigned int nbr)
        : uinteger{nbr}
          , m_type{Type::Unsigned}
    {
    }

    explicit Any(const unsigned short nbr)
        : uinteger{nbr}
          , m_type{Type::Unsigned}
    {
    }

    explicit Any(const unsigned char nbr)
        : uinteger{nbr}
          , m_type{Type::Unsigned}
    {
    }

    explicit Any(const bool b)
        : boolean{b}
          , m_type{Type::Boolean}
    {
    }

    explicit Any(const char* str)
        : string{str}
          , m_type{str == nullptr ? Type::Nil : Type::String}
    {
    }

    explicit Any(std::string&& str)
        : string{std::forward<std::string>(str)}
          , m_type{Type::String}
    {
    }

    explicit Any(std::string str)
        : string{std::move(str)}
          , m_type{Type::String}
    {
    }

    template<typename T0, typename T1, typename... Ts>
    explicit Any(T0&& arg0, T1&& arg1, Ts&& ... args)
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
    explicit Any(std::tuple<Args...>&& args)
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
