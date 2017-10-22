#pragma once

#include <lua.hpp>

#include <stdexcept>
#include <iostream>
#include <string>


namespace lua
{
namespace stack
{
inline void dump(lua_State* L)
{
    const auto top = lua_gettop(L);
    for( int i = 1; i <= top; i++ )
    {
        /* repeat for each level */
        const auto t = lua_type(L, i);
        switch( t )
        {
            case LUA_TSTRING: /* strings */
                std::cout << '`' << lua_tostring(L, i) << '\'';
                break;

            case LUA_TBOOLEAN: /* booleans */
                std::cout << (lua_toboolean(L, i) ? "true" : "false");
                break;

            case LUA_TNUMBER: /* numbers */
                std::cout << lua_tonumber(L, i);
                break;

            default: /* other values */
                std::cout << lua_typename(L, t);
                break;
        }
        std::cout << "  "; /* put a separator */
    }
    std::cout << std::endl; /* end the listing */
}
}


class ExceptionBase : public std::exception
{
    std::string m_message;

public:
    explicit ExceptionBase(const std::string& msg)
        : exception{}
        , m_message{msg}
    {
    }
    
    const char* what() const noexcept(noexcept(m_message.c_str())) override final
    {
        return m_message.c_str();
    }
};


//////////////////////////////////////////////////////////////////////////////////////////////
class LoadError : public ExceptionBase
{
public:
    explicit LoadError(lua_State* luaState)
        : ExceptionBase{lua_tostring(luaState, -1)}
    {
        lua_pop(luaState, 1);
    }
};


//////////////////////////////////////////////////////////////////////////////////////////////
class RuntimeError : public ExceptionBase
{
public:
    explicit RuntimeError(lua_State* luaState)
        : ExceptionBase{lua_tostring(luaState, -1)}
    {
        lua_pop(luaState, 1);
    }
};


//////////////////////////////////////////////////////////////////////////////////////////////
class TypeMismatchError : public ExceptionBase
{
public:
    explicit TypeMismatchError(lua_State*, const int index)
        : ExceptionBase{"Type mismatch error at index " + std::to_string(index)}
    {
    }
};
}
