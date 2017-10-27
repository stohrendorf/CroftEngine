#pragma once

#include "Traits.h"

#include "LuaPrimitives.h"
#include "LuaException.h"
#include "LuaStackItem.h"
#include "LuaValue.h"
#include "LuaFunctor.h"
#include "Any.h"

#include <memory>


namespace lua
{
//////////////////////////////////////////////////////////////////////////////////////////////
/// Class that hold lua interpreter state. Lua state is managed by pointer which also is copied to lua::Ref values.
class State
{
    /// Class takes care of automaticaly closing Lua state when in destructor
    lua_State* m_luaState = nullptr;

    /// Class deletes DeallocQueue in destructor
    std::unique_ptr<detail::DeallocQueue> m_deallocQueue = nullptr;

    /// Function for metatable "__call" field. It calls stored functor pushes return values to stack.
    ///
    /// @pre In Lua C API during function calls lua_State moves stack index to place, where first element is our userdata, and next elements are returned values
    static int metatableCallFunction(lua_State* luaState)
    {
        auto functor = *static_cast<BaseFunctor **>(luaL_checkudata(luaState, 1, "luaL_Functor"));
        return functor->call(luaState);
    }

    /// Function for metatable "__gc" field. It deletes captured variables from stored functors.
    static int metatableDeleteFunction(lua_State* luaState)
    {
        const auto functor = *static_cast<BaseFunctor **>(luaL_checkudata(luaState, 1, "luaL_Functor"));
        delete functor;
        return 0;
    }

    Value executeLoadedFunction(int index) const
    {
        if( lua_pcall(m_luaState, 0, LUA_MULTRET, 0) )
            throw RuntimeError{m_luaState};

        auto pushedValues = lua_gettop(m_luaState) - index;
        return Value(std::make_shared<detail::StackItem>(m_luaState, m_deallocQueue.get(), index, pushedValues,
                                                         pushedValues > 0 ? pushedValues - 1 : 0));
    }

    void initialize(const bool loadLibs)
    {
        m_deallocQueue = std::make_unique<detail::DeallocQueue>();
        m_luaState = luaL_newstate();
        BOOST_ASSERT(m_luaState != nullptr);

        if( loadLibs )
            luaL_openlibs(m_luaState);

        // We will create metatable for Lua functors for memory management and actual function call
        luaL_newmetatable(m_luaState, "luaL_Functor");

        // Set up metatable call operator for functors
        lua_pushcfunction(m_luaState, &State::metatableCallFunction);
        lua_setfield(m_luaState, -2, "__call");

        // Set up metatable garbage collection for functors
        lua_pushcfunction(m_luaState, &State::metatableDeleteFunction);
        lua_setfield(m_luaState, -2, "__gc");

        // Pop metatable
        lua_pop(m_luaState, 1);
    }


public:
    /// Constructor creates new state and stores it to pointer.
///
/// @param loadLibs     If we want to open standard libraries - function luaL_openlibs
    explicit State(const bool loadLibs = true)
    {
        initialize(loadLibs);
    }

    ~State()
    {
        lua_close(m_luaState);
    }

    // State is non-copyable
    State(const State& other) = delete;

    State& operator=(const State&) = delete;

    /// Query global values from Lua state
    ///
    /// @return Some value with type lua::Type
    Value operator[](const String name) const
    {
        return Value{m_luaState, m_deallocQueue.get(), name};
    }

    /// Deleted compare operator
    bool operator==(Value& other) = delete;

    /// Sets global value to Lua state
    ///
    /// @param key      Stores value to _G[key]
    /// @param value    Value witch will be stored to _G[key]
    template<typename T>
    void set(const String key, T&& value) const
    {
        traits::ValueTraits<T>::push(m_luaState, std::forward<T>(value));
        lua_setglobal(m_luaState, key);
    }

    /// Executes file text on Lua state
    ///
    /// @throws lua::LoadError      When file cannot be found or loaded
    /// @throws lua::RuntimeError   When there is runtime error
    ///
    /// @param filePath File path indicating which file will be executed
    Value doFile(const std::string& filePath) const
    {
        const auto stackTop = lua_gettop(m_luaState);

        if( luaL_loadfile(m_luaState, filePath.c_str()) )
            throw LoadError(m_luaState);

        return executeLoadedFunction(stackTop);
    }

    /// Execute string on Lua state
    ///
    /// @throws lua::LoadError      When string cannot be loaded
    /// @throws lua::RuntimeError   When there is runtime error
    ///
    /// @param string   Command which will be executed
    Value doString(const std::string& string) const
    {
        const auto stackTop = lua_gettop(m_luaState);

        if( luaL_loadstring(m_luaState, string.c_str()) )
            throw LoadError{m_luaState};

        return executeLoadedFunction(stackTop);
    }

    /// Get pointer of Lua state
    ///
    /// @return Pointer of Lua state
    lua_State* getState() const
    {
        return m_luaState;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////
    // Conventional setting functions

    void setCStr(const String key, String&& value) const
    {
        set<const char*>(key, std::forward<const char*>(value));
    }

    void setData(String key, const String value, const size_t length) const
    {
        traits::ValueTraits<String>::push(m_luaState, value, length);
        lua_setglobal(m_luaState, key);
    }

    void setString(String key, const std::string& string) const
    {
        setData(key, string.c_str(), string.length());
    }

    void set(const String key, std::string&& value) const
    {
        setString(key, std::forward<std::string>(value));
    }

    void setNumber(const String key, Number&& number) const
    {
        set<Number>(key, std::forward<Number>(number));
    }

    void setInt(const String key, Integer&& number) const
    {
        set<Integer>(key, std::forward<Integer>(number));
    }

    Value createTable() const
    {
        return Value{ m_luaState, m_deallocQueue.get(), nullptr };
    }
};
}
