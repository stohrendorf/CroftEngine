#pragma once

#include "access.h"

#include <gsl/gsl-lite.hpp>
#include <type_traits>

namespace serialization
{
template<typename T, typename TContext>
auto create(const TypeId<gsl::not_null<T>>&, const Serializer<TContext>& ser)
  -> std::remove_reference_t<decltype(access<T>::callCreate(ser), std::declval<gsl::not_null<T>>())>
{
  Expects(ser.loading);
  return gsl::not_null<T>{access<T>::callCreate(ser)};
}

template<typename T, typename TContext>
void save(gsl::not_null<T>& data, const Serializer<TContext>& ser)
{
  auto tmp = data.get();
  access<decltype(tmp)>::callSerializeOrSave(tmp, ser);
}

template<typename T, typename TContext>
void load(gsl::not_null<T>& data, const Serializer<TContext>& ser)
{
  data = create(TypeId<gsl::not_null<T>>{}, ser);
}
} // namespace serialization
