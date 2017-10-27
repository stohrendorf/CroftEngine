#pragma once

#include <tuple>


namespace lua
{
template<typename ... Ts>
class Return final
{
    std::tuple<Ts&...> m_tiedValues;

public:
    explicit Return(Ts&... args)
        : m_tiedValues{args...}
    {
    }
};


template<typename ... Ts>
Return<Ts&...> tie(Ts&... args)
{
    return Return<Ts&...>(args...);
}
}
