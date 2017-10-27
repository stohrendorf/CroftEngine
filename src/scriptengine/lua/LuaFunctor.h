#pragma once

#include <functional>
#include <type_traits>


namespace lua
{
namespace detail
{
template<typename T>
T readValue(lua_State* luaState,
            DeallocQueue* deallocQueue,
            const int stackTop)
{
    static_assert(std::is_same<traits::RemoveCVR<T>, T>::value, "T must not be CV-qualified or a reference");
    return Value{std::make_shared<StackItem>(luaState, deallocQueue, stackTop - 1, 1, 0)}.to<T>();
}

template<typename... Ts, size_t... Is>
std::tuple<Ts...> unpackMultiValues(lua_State* luaState,
                                    DeallocQueue* deallocQueue,
                                    int stackTop,
                                    traits::Indices<Is...>)
{
    return std::make_tuple(readValue<Ts>(luaState, deallocQueue, Is + stackTop)...);
}

/// Function expects that number of elements in tuple and number of pushed values in stack are same. Applications takes care of this requirement by popping overlapping values before calling this function
template<typename... Ts>
std::tuple<Ts...> getAndPop(lua_State* luaState,
                            DeallocQueue* deallocQueue,
                            int stackTop)
{
    return unpackMultiValues<Ts...>(luaState, deallocQueue, stackTop,
                                    typename traits::MakeIndices<sizeof...(Ts)>::Type());
}

template<>
inline std::tuple<> getAndPop<>(lua_State*,
                                DeallocQueue*,
                                int)
{
    return {};
}
}


//////////////////////////////////////////////////////////////////////////////////////////////
/// Base functor class with call function. It is used for registering lamdas, or regular functions
struct BaseFunctor
{
    template<typename ReturnType, typename... Args, size_t... Indices>
    static ReturnType callHelper(std::function<ReturnType(Args ...)> callee,
                                 std::tuple<Args...>&& args,
                                 const traits::IndexTuple<Indices...>&)
    {
        return callee(std::forward<Args>(std::get<Indices>(args))...);
    }

    template<typename ReturnType, typename... Args>
    static ReturnType call(std::function<ReturnType(Args ...)> callee, std::tuple<Args...>&& args)
    {
        return callHelper(callee, std::forward<std::tuple<Args...>>(args),
                          typename traits::MakeIndexTuple<Args...>::Type());
    }

    explicit BaseFunctor() = default;

    virtual ~BaseFunctor() noexcept = default;

    /// In Lua numbers of argumens can be different so here we will handle these situations.
    ///
    /// @param luaState     Pointer of Lua state
    static void prepareFunctionCall(lua_State* luaState, const int requiredValues)
    {
        // First item is our pushed userdata
        if( lua_gettop(luaState) > requiredValues + 1 )
        {
            lua_settop(luaState, requiredValues + 1);
        }
    }

    /// Virtual function that will make Lua call to our functor.
    ///
    /// @param luaState     Pointer of Lua state
    virtual int call(lua_State* luaState) = 0;
};


//////////////////////////////////////////////////////////////////////////////////////////////
/// Functor with return values
template<typename ReturnType, typename... Args>
struct Functor : public BaseFunctor
{
    std::function<ReturnType(Args...)> function;

    /// Constructor creates functor to be pushed to Lua interpret
    explicit Functor(std::function<ReturnType(Args...)> function)
        : BaseFunctor{}
        , function{std::move(function)}
    {
    }

    /// We will make Lua call to our functor.
    ///
    /// @note When we call function from Lua to C, they have their own stack, where in the first position is our binded userdata and next position are pushed arguments
    ///
    /// @param luaState     Pointer of Lua state
    int call(lua_State* luaState) override
    {
        ReturnType value = BaseFunctor::call(function, detail::getAndPop<Args...>(luaState, nullptr, 2));
        return traits::ValueTraits<ReturnType>::push(luaState, std::forward<ReturnType>(value));
    }
};


//////////////////////////////////////////////////////////////////////////////////////////////
/// Functor with no return values
template<typename... Args>
struct Functor<void, Args...> : public BaseFunctor
{
    std::function<void(Args...)> function;

    /// Constructor creates functor to be pushed to Lua interpret
    explicit Functor(std::function<void(Args...)> function)
        : BaseFunctor{}
        , function{std::move(function)}
    {
    }

    /// We will make Lua call to our functor.
    ///
    /// @note When we call function from Lua to C, they have their own stack, where in the first position is our binded userdata and next position are pushed arguments
    ///
    /// @param luaState     Pointer of Lua state
    int call(lua_State* luaState) override
    {
        BaseFunctor::call(function, detail::getAndPop<Args...>(luaState, nullptr, 2));
        return 0;
    }
};


namespace traits
{
template<typename ReturnType, typename... Args>
struct ValueTraits<std::function<ReturnType(Args ...)>>
{
    static int push(lua_State* luaState, std::function<ReturnType(Args ...)> function)
    {
        const auto udata{static_cast<BaseFunctor**>(lua_newuserdata(luaState, sizeof(BaseFunctor *)))};
        *udata = new Functor<ReturnType, RemoveCVR<Args>...>(function);

        luaL_getmetatable(luaState, "luaL_Functor");
        lua_setmetatable(luaState, -2);
        return 1;
    }
};


template<typename ReturnType, typename... Args>
struct ValueTraits<ReturnType(Args ...)> : ValueTraits<std::function<ReturnType(Args ...)>>
{
};


template<typename ReturnType, typename... Args>
struct ValueTraits<ReturnType(*)(Args ...)> : ValueTraits<std::function<ReturnType(Args ...)>>
{
};


template<typename Class, typename ReturnType, typename... Args>
struct ValueTraits<ReturnType(Class::*)(Args ...)> : ValueTraits<ReturnType(Args ...)>
{
};


template<typename Class, typename ReturnType, typename... Args>
struct ValueTraits<ReturnType(Class::*)(Args ...) const> : ValueTraits<ReturnType(Args ...)>
{
};
}
}
