#pragma once

#include "access.h"
#include "serialization.h"

#include <gsl-lite/gsl-lite.hpp>
#include <type_traits>

namespace serialization
{
template<typename T, typename TContext>
auto create(const TypeId<gsl_lite::not_null<T>>&, const Deserializer<TContext>& ser)
  -> std::remove_reference_t<decltype(access::dispatchCreate<T>(ser), std::declval<gsl_lite::not_null<T>>())>
{
  return gsl_lite::not_null<T>{access::dispatchCreate<T>(ser)};
}

template<typename T, typename TContext>
void serialize(const gsl_lite::not_null<T>& data, const Serializer<TContext>& ser)
{
  auto tmp = data.get();
  access::dispatchSerialize(tmp, ser);
}

template<typename T, typename TContext>
void deserialize(gsl_lite::not_null<T>& data, const Deserializer<TContext>& ser)
{
  data = create(TypeId<gsl_lite::not_null<T>>{}, ser);
}
} // namespace serialization
