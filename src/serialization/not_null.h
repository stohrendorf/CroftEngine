#pragma once

#include "access.h"

#include <gsl/gsl-lite.hpp>
#include <type_traits>

namespace serialization
{
template<typename T, typename TContext>
auto create(const TypeId<gsl::not_null<T>>&, const Deserializer<TContext>& ser)
  -> decltype(access<T, true>::dispatch(ser), std::declval<gsl::not_null<T>>())
{
  return gsl::not_null<T>{access<T, true>::dispatch(ser)};
}

template<typename T, typename TContext>
void serialize(const gsl::not_null<T>& data, const Serializer<TContext>& ser)
{
  auto tmp = data.get();
  access<decltype(tmp), false>::dispatch(tmp, ser);
}

template<typename T, typename TContext>
void deserialize(gsl::not_null<T>& data, const Deserializer<TContext>& ser)
{
  data = create(TypeId<gsl::not_null<T>>{}, ser);
}
} // namespace serialization
