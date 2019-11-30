#pragma once

#include "gsl-lite.hpp"
#include "serialization.h"

namespace serialization
{
template<typename T>
auto create(const TypeId<gsl::not_null<T>>&, const Serializer& ser)
  -> decltype(access::callCreate(TypeId<T>{}, ser), detail::Result<gsl::not_null<T>>())
{
  Expects(ser.loading);
  return gsl::not_null<T>{access::callCreate(TypeId<T>{}, ser)};
}

template<typename T>
void save(gsl::not_null<T>& data, const Serializer& ser)
{
  auto tmp = data.get();
  access::callSerializeOrSave(tmp, ser);
}

template<typename T>
void load(gsl::not_null<T>& data, const Serializer& ser)
{
  data = create(TypeId<gsl::not_null<T>>{}, ser);
}
} // namespace serialization
