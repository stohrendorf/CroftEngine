#pragma once

#include <lua.hpp>

#include <stdexcept>
#include <iostream>
#include <string>
#include <utility>

namespace lua
{
class ExceptionBase
    : public std::exception
{
    std::string m_message;

public:
    explicit ExceptionBase(std::string msg)
        : exception{}
          , m_message{std::move(msg)}
    {
    }

    const char* what() const noexcept(noexcept(m_message.c_str())) final
    {
        return m_message.c_str();
    }
};

class LoadError
    : public ExceptionBase
{
public:
    explicit LoadError(lua_State* luaState)
        : ExceptionBase{lua_tostring(luaState, -1)}
    {
        lua_pop(luaState, 1);
    }
};

class RuntimeError
    : public ExceptionBase
{
public:
    explicit RuntimeError(lua_State* luaState)
        : ExceptionBase{lua_tostring(luaState, -1)}
    {
        lua_pop(luaState, 1);
    }
};
}
