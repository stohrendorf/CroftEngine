#pragma once

#include "serialization.h"

#include <variant>

namespace serialization
{
namespace detail
{
template<size_t I, typename TContext, typename... Ts>
void trySerialize(std::variant<Ts...>& data, const Serializer<TContext>& ser)
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
      if(ser.loading)
        access<Alternative>::callSerializeOrLoad(std::get<I>(data), ser);
      else if(std::holds_alternative<Alternative>(data))
        access<Alternative>::callSerializeOrSave(std::get<I>(data), ser);
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
std::variant<Ts...> tryCreate(const TypeId<std::variant<Ts...>>& tid, const Serializer<TContext>& ser)
{
  Expects(ser.loading);
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
      return access<Alternative>::callCreate(ser);
    }
    catch(Exception&)
    {
      return tryCreate<I + 1u>(tid, ser);
    }
  }
}
} // namespace detail

template<typename... Ts, typename TContext>
void serialize(std::variant<Ts...>& data, const Serializer<TContext>& ser)
{
  detail::trySerialize<0>(data, ser);
}

template<typename TContext, typename... Ts>
std::variant<Ts...> create(const TypeId<std::variant<Ts...>>& tid, const Serializer<TContext>& ser)
{
  return detail::tryCreate<0>(tid, ser);
}
} // namespace serialization
