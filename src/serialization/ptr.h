#pragma once

#include "access.h"
#include "serialization.h"

#include <gsl-lite/gsl-lite.hpp>

namespace serialization
{
template<typename T, typename TContext>
auto serialize(T* const& data, const Serializer<TContext>& ser) -> decltype(ptrSave(data, ser), void())
{
  auto tmp = ptrSave(data, ser);
  access::dispatchSerialize(tmp, ser);
  return;
}

template<typename T, typename TContext>
auto deserialize(T*& data, const Deserializer<TContext>& ser)
  -> decltype(ptrSave(ptrLoad(TypeId<T*>{}, ptrSave(data, std::declval<Serializer<TContext>>()), ser),
                      std::declval<Serializer<TContext>>()),
              void())
{
  using IdxType = decltype(ptrSave(data, std::declval<Serializer<TContext>>()));
  data = ptrLoad(TypeId<T*>{}, access::dispatchCreate<IdxType>(ser), ser);
  return;
}

template<typename T, typename TContext>
auto create(const TypeId<T*>&, const Deserializer<TContext>& ser)
  -> decltype(ptrSave(ptrLoad(TypeId<T*>{}, ptrSave(std::declval<T*>(), std::declval<Serializer<TContext>>()), ser),
                      std::declval<Serializer<TContext>>()),
              static_cast<T*>(nullptr))
{
  using IdxType = decltype(ptrSave(std::declval<T*>(), std::declval<Serializer<TContext>>()));
  return ptrLoad(TypeId<T*>{}, access::dispatchCreate<IdxType>(ser), ser);
}

template<typename T, typename TContext>
auto create(const TypeId<T* const>&, const Deserializer<TContext>& ser) -> decltype(access::dispatchCreate<T*>(ser))
{
  return access::dispatchCreate<T*>(ser);
}

template<typename T, typename TContext>
auto serialize(T* const& data, const Serializer<TContext>& ser) -> decltype(T::ptrSave(data, ser), void())
{
  gsl_Expects(!ser.loading);
  auto tmp = T::ptrSave(data, ser);
  access::dispatchSerialize(tmp, ser);
  return;
}

template<typename T, typename TContext>
auto serialize(T* const& data,

               const Serializer<TContext>& ser)
  -> decltype(T::ptrSave(T::ptrLoad(T::ptrSave(data, ser), std::declval<Deserializer<TContext>>()), ser), void())
{
  access::dispatchSerialize(const_cast<T* const&>(data), ser);
  return;
}

template<typename T, typename TContext>
auto deserialize(T*& data, const Deserializer<TContext>& ser)
  -> decltype(T::ptrSave(T::ptrLoad(T::ptrSave(data, std::declval<Serializer<TContext>>()), ser),
                         std::declval<Serializer<TContext>>()),
              void())
{
  data = T::ptrLoad(access::dispatchDeserialize<decltype(T::ptrSave(data, std::declval<Serializer<TContext>>()))>(ser),
                    ser);
  return;
}
} // namespace serialization