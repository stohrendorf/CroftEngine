#pragma once

#include "serialization_fwd.h"

#include <string>
#include <type_traits>

namespace serialization::access
{
template<typename T, typename TContext>
inline auto callSerializeOrLoad(T& data, const Serializer<TContext>& ser) -> decltype(data.serialize(ser), void())
{
  return data.serialize(ser);
}

template<typename T, typename TContext>
inline auto callSerializeOrLoad(T& data, const Serializer<TContext>& ser) -> decltype(serialize(data, ser), void())
{
  return serialize(data, ser);
}

template<typename T, typename TContext>
inline auto callSerializeOrLoad(T& data, const Serializer<TContext>& ser) -> decltype(data.load(ser), void())
{
  return data.load(ser);
}

template<typename T, typename TContext>
inline auto callSerializeOrLoad(T& data, const Serializer<TContext>& ser) -> decltype(load(data, ser), void())
{
  return load(data, ser);
}

template<typename T, typename TContext>
inline auto callSerializeOrSave(T& data, const Serializer<TContext>& ser) -> decltype(data.serialize(ser), void())
{
  return data.serialize(ser);
}

template<typename T, typename TContext>
inline auto callSerializeOrSave(T& data, const Serializer<TContext>& ser) -> decltype(serialize(data, ser), void())
{
  return serialize(data, ser);
}

template<typename T, typename TContext>
inline auto callSerializeOrSave(T& data, const Serializer<TContext>& ser) -> decltype(data.save(ser), void())
{
  return data.save(ser);
}

template<typename T, typename TContext>
inline auto callSerializeOrSave(T& data, const Serializer<TContext>& ser) -> decltype(save(data, ser), void())
{
  return save(data, ser);
}

template<typename T, typename TContext>
inline void callSerialize(T& data, const Serializer<TContext>& ser)
{
  if(ser.loading)
    callSerializeOrLoad(data, ser);
  else
    callSerializeOrSave(data, ser);
}

template<typename T, typename TContext>
inline auto callCreate(const TypeId<T>&, const Serializer<TContext>& ser) -> decltype(T::create(ser))
{
  Expects(ser.loading);
  return T::create(ser);
}

template<typename T, typename TContext>
inline auto callCreate(const TypeId<T>& tid, const Serializer<TContext>& ser) -> decltype(create(tid, ser))
{
  Expects(ser.loading);
  return create(tid, ser);
}
} // namespace serialization::access
