#pragma once

#include "LuaValue.h"
#include "Traits.h"

#include <tuple>


namespace lua
{
//////////////////////////////////////////////////////////////////////////////////////////////
namespace stack
{
//////////////////////////////////////////////////////////////////////////////////////////////
/// Function get single value from lua stack
template<typename T>
T readValue(lua_State* luaState,
            detail::DeallocQueue* deallocQueue,
            const int stackTop)
{
    static_assert(std::is_same<traits::RemoveCVR<T>, T>::value, "T must not be CV-qualified or a reference");
    return Value{std::make_shared<detail::StackItem>(luaState, deallocQueue, stackTop - 1, 1, 0)}.to<T>();
}

/// Function creates indexes for mutli values and get them from stack
template<typename... Ts, size_t... Is>
std::tuple<Ts...> unpackMultiValues(lua_State* luaState,
                                    detail::DeallocQueue* deallocQueue,
                                    int stackTop,
                                    traits::Indices<Is...>)
{
    return std::make_tuple(readValue<Ts>(luaState, deallocQueue, Is + stackTop)...);
}

/// Function expects that number of elements in tuple and number of pushed values in stack are same. Applications takes care of this requirement by popping overlapping values before calling this function
template<typename... Ts>
std::tuple<Ts...> getAndPop(lua_State* luaState,
                            detail::DeallocQueue* deallocQueue,
                            int stackTop)
{
    return unpackMultiValues<Ts...>(luaState, deallocQueue, stackTop,
                                    typename traits::MakeIndices<sizeof...(Ts)>::Type());
}

template<>
inline std::tuple<> getAndPop<>(lua_State*,
                                detail::DeallocQueue*,
                                int)
{
    return {};
}
}


//////////////////////////////////////////////////////////////////////////////////////////////
/// Class for automaticly cas lua::Function instance to multiple return values with lua::tie
template<typename ... Ts>
class Return final
{
    /// Return values
    std::tuple<Ts&...> m_tiedValues;

public:
    explicit Return(Ts&... args)
        : m_tiedValues{args...}
    {
    }

    void operator=(const Value& value)
    {
        const auto requiredValues = std::min<int>(sizeof...(Ts), value.m_stack->pushed);

        // When there are more returned values than variables in tuple, we will clear values that are not needed
        if( requiredValues < value.m_stack->grouped + 1 )
        {
            const auto currentStackTop = lua_gettop(value.m_stack->state);

            // We will check if we haven't pushed some other new lua::Value to stack
            if( value.m_stack->top + value.m_stack->pushed == currentStackTop )
                lua_settop(value.m_stack->state, value.m_stack->top + requiredValues);
            else
                value.m_stack->deallocQueue->push(detail::DeallocStackItem(value.m_stack->top, value.m_stack->pushed));
        }

        // We will take pushed values and distribute them to returned lua::Values
        value.m_stack->pushed = 0;

        m_tiedValues = stack::getAndPop<traits::RemoveCVR<Ts>...>(value.m_stack->state, value.m_stack->deallocQueue,
                                                                  value.m_stack->top + 1);
    }
};


/// Use this function when you want to retrieve multiple return values from lua::Function
template<typename ... Ts>
Return<Ts&...> tie(Ts&... args)
{
    return Return<Ts&...>(args...);
}
}
