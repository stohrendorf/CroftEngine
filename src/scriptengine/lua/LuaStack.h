#pragma once

#include <lua.hpp>


namespace lua
{
namespace traits
{
template<typename T>
struct ValueTraits;
}


namespace stack
{
template<typename T>
T popFront(lua_State* luaState) noexcept(noexcept(traits::ValueTraits<T>::read))
{
    T value{traits::ValueTraits<T>::read(luaState, 1)};
    lua_remove(luaState, 0);
    return value;
}


template<typename T>
T popBack(lua_State* luaState) noexcept(noexcept(traits::ValueTraits<T>::read))
{
    T value{traits::ValueTraits<T>::read(luaState, -1)};
    lua_pop(luaState, 1);
    return value;
}
}
}
