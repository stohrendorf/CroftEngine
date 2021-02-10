#pragma once

#include "access.h"

namespace serialization
{
template<typename T, typename TContext>
auto serialize(T* const& data, const Serializer<TContext>& ser) -> decltype(ptrSave(data, ser), void())
{
  Expects(!ser.loading);
  auto tmp = ptrSave(data, ser);
  access<decltype(tmp)>::callSerialize(tmp, ser);
}

template<typename T, typename TContext>
auto serialize(T*& data, const Serializer<TContext>& ser)
  -> decltype(ptrSave(ptrLoad(TypeId<T*>{}, ptrSave(data, ser), ser), ser), void())
{
  if(ser.loading)
  {
    using IdxType = decltype(ptrSave(data, ser));
    data = ptrLoad(TypeId<T*>{}, access<IdxType>::callCreate(ser), ser);
  }
  else
  {
    access<T* const>::callSerialize(const_cast<T* const&>(data), ser);
  }
}

template<typename T, typename TContext>
auto create(const TypeId<T*>&, const Serializer<TContext>& ser)
  -> decltype(ptrSave(ptrLoad(TypeId<T*>{}, ptrSave(std::declval<T*>(), ser), ser), ser), static_cast<T*>(nullptr))
{
  using IdxType = decltype(ptrSave(std::declval<T*>(), ser));
  return ptrLoad(TypeId<T*>{}, access<IdxType>::callCreate(ser), ser);
}

template<typename T, typename TContext>
auto create(const TypeId<T* const>&, const Serializer<TContext>& ser) -> decltype(access<T*>::callCreate(ser))
{
  return access<T*>::callCreate(ser);
}

template<typename T, typename TContext>
auto serialize(T* const& data, const Serializer<TContext>& ser) -> decltype(T::ptrSave(data, ser), void())
{
  Expects(!ser.loading);
  auto tmp = T::ptrSave(data, ser);
  access<decltype(tmp)>::callSerialize(tmp, ser);
}

template<typename T, typename TContext>
auto save(const T*& data, const Serializer<TContext>& ser)
  -> decltype(T::ptrSave(T::ptrLoad(T::ptrSave(data, ser), ser), ser), void())
{
  access<T* const>::callSerializeOrSave(const_cast<T* const&>(data), ser);
}

template<typename T, typename TContext>
auto load(T*& data, const Serializer<TContext>& ser)
  -> decltype(T::ptrSave(T::ptrLoad(T::ptrSave(data, ser), ser), ser), void())
{
  data = T::ptrLoad(access<decltype(T::ptrSave(data, ser))>::callCreate(ser), ser);
}
} // namespace serialization
