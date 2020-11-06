#pragma once

#include "serialization_fwd.h"

namespace serialization
{
template<typename T>
auto serialize(T* const& data, const Serializer& ser) -> decltype(ptrSave(data, ser), void())
{
  Expects(!ser.loading);
  auto tmp = ptrSave(data, ser);
  access::callSerialize(tmp, ser);
}

template<typename T>
auto serialize(T*& data, const Serializer& ser)
  -> decltype(ptrSave(ptrLoad(TypeId<T*>{}, ptrSave(data, ser), ser), ser), void())
{
  if(ser.loading)
  {
    using IdxType = decltype(ptrSave(data, ser));
    data = ptrLoad(TypeId<T*>{}, access::callCreate(TypeId<IdxType>{}, ser), ser);
  }
  else
  {
    access::callSerialize(const_cast<T* const&>(data), ser);
  }
}

template<typename T>
auto create(const TypeId<T*>&, const Serializer& ser)
  -> decltype(ptrSave(ptrLoad(TypeId<T*>{}, ptrSave(std::declval<T*>(), ser), ser), ser), detail::Result<T*>())
{
  using IdxType = decltype(ptrSave(std::declval<T*>(), ser));
  return ptrLoad(TypeId<T*>{}, access::callCreate(TypeId<IdxType>{}, ser), ser);
}

template<typename T>
auto create(const TypeId<T* const>&, const Serializer& ser) -> decltype(access::callCreate(TypeId<T*>{}, ser))
{
  return access::callCreate(TypeId<T*>{}, ser);
}

template<typename T>
auto serialize(T* const& data, const Serializer& ser) -> decltype(T::ptrSave(data, ser), void())
{
  Expects(!ser.loading);
  auto tmp = T::ptrSave(data, ser);
  access::callSerialize(tmp, ser);
}

template<typename T>
auto save(const T*& data, const Serializer& ser)
  -> decltype(T::ptrSave(T::ptrLoad(T::ptrSave(data, ser), ser), ser), void())
{
  access::callSerializeOrSave(const_cast<T* const&>(data), ser);
}

template<typename T>
auto load(T*& data, const Serializer& ser) -> decltype(T::ptrSave(T::ptrLoad(T::ptrSave(data, ser), ser), ser), void())
{
  data = T::ptrLoad(access::callCreate(TypeId<decltype(T::ptrSave(data, ser))>{}, ser), ser);
}
} // namespace serialization
