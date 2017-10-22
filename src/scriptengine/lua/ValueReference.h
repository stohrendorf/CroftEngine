//
//  LuaRef.h
//  LuaState
//
//  Created by Simon Mikuda on 17/04/14.
//
//  See LICENSE and README.md files

#pragma once

#include "LuaValue.h"


namespace lua
{
/// Reference to Lua value. Can be created from any lua::Value
class ValueReference final
{
    /// Pointer of Lua state
    lua_State* m_luaState = nullptr;

    detail::DeallocQueue* m_deallocQueue = nullptr;

    /// Key of referenced value in LUA_REGISTRYINDEX
    int m_refKey;

    void createRefKey()
    {
        m_refKey = luaL_ref(m_luaState, LUA_REGISTRYINDEX);
    }

public:
    explicit ValueReference() = default;

    // Copy and move constructors just use operator functions
    explicit ValueReference(const Value& value)
    {
        operator=(value);
    }

    explicit ValueReference(Value&& value)
    {
        operator=(std::forward<Value>(value));
    }

    ~ValueReference()
    {
        luaL_unref(m_luaState, LUA_REGISTRYINDEX, m_refKey);
    }

    /// Copy assignment. Creates lua::Ref from lua::Value.
    void operator=(const Value& value)
    {
        m_luaState = value.m_stack->state;
        m_deallocQueue = value.m_stack->deallocQueue;

        // Duplicate top value
        lua_pushvalue(m_luaState, -1);

        // Create reference to registry
        createRefKey();
    }

    /// Move assignment. Creates lua::Ref from lua::Value from top of stack and pops it
    void operator=(Value&& value)
    {
        m_luaState = value.m_stack->state;
        m_deallocQueue = value.m_stack->deallocQueue;

        if( value.m_stack->pushed > 0 )
            value.m_stack->pushed -= 1;
        else
            value.m_stack->top -= 1;

        // Create reference to registry
        createRefKey();
    }

    /// Creates lua::Value from lua::Ref
    ///
    /// @return lua::Value with referenced value on stack
    Value unref() const
    {
        lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_refKey);
        return Value{std::make_shared<detail::StackItem>(m_luaState, m_deallocQueue, lua_gettop(m_luaState) - 1, 1, 0)};
    }

    bool isInitialized() const
    {
        return m_luaState != nullptr;
    }
};
}
