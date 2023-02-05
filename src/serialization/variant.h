#pragma once

#include "serialization.h"

#include <variant>

namespace serialization
{
namespace detail
{
template<size_t I, typename TContext, bool Loading, typename... Ts>
void trySerialize(const std::variant<Ts...>& data, const BaseSerializer<Loading, TContext>& ser)
{
  if constexpr(I >= sizeof...(Ts))
  {
    (void)data;
    (void)ser;
    SERIALIZER_EXCEPTION("No variant could be serialized");
  }
  else
  {
    try
    {
      using Alternative = std::variant_alternative_t<I, std::variant<Ts...>>;
      if constexpr(Loading)
        access<Alternative, true>::dispatch(std::get<I>(data), ser);
      else if(std::holds_alternative<Alternative>(data))
        access<Alternative, false>::dispatch(std::get<I>(data), ser);
      else
        trySerialize<I + 1u>(data, ser);
    }
    catch(Exception&)
    {
      trySerialize<I + 1u>(data, ser);
    }
  }
}

template<size_t I, typename TContext, typename... Ts>
std::variant<Ts...> tryCreate(const TypeId<std::variant<Ts...>>& tid, const Deserializer<TContext>& ser)
{
  if constexpr(I >= sizeof...(Ts))
  {
    (void)tid;
    (void)ser;
    SERIALIZER_EXCEPTION("No variant could be serialized");
  }
  else
  {
    try
    {
      using Alternative = std::variant_alternative_t<I, std::variant<Ts...>>;
      return access<Alternative, true>::dispatch(ser);
    }
    catch(Exception&)
    {
      return tryCreate<I + 1u>(tid, ser);
    }
  }
}
} // namespace detail

template<typename... Ts, typename TContext, bool Loading>
void serialize(const std::variant<Ts...>& data, const BaseSerializer<Loading, TContext>& ser)
{
  detail::trySerialize<0>(data, ser);
}

template<typename TContext, typename... Ts>
std::variant<Ts...> create(const TypeId<std::variant<Ts...>>& tid, const Deserializer<TContext>& ser)
{
  return detail::tryCreate<0>(tid, ser);
}
} // namespace serialization
