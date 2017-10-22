#pragma once

#include <lua.hpp>

#include <type_traits>


namespace lua
{
/// Lua table type
struct Table
{
};


/// Any Lua function, C function, or table/userdata with __call metamethod
struct Callable
{
};


using Number = lua_Number;

using Integer = lua_Integer;

using Unsigned = std::make_unsigned<lua_Integer>::type;

using Boolean = bool;

using String = const char*;

using Nil = nullptr_t;

using Pointer = void*;
}
