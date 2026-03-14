#pragma once

#include "concepts.h"

namespace serialization::access
{
template<typename T, typename TContext>
requires Serializable<T, TContext> void dispatchSerialize(const T& data, const Serializer<TContext>& ser)
{
  if constexpr(MemberSerializable<T, TContext>)
    data.serialize(ser);
  else
    serialize(data, ser);
}

template<typename T, typename TContext>
requires Deserializable<T, TContext> void dispatchDeserialize(T& data, const Deserializer<TContext>& ser)
{
  if constexpr(MemberDeserializable<T, TContext>)
    data.deserialize(ser);
  else
    deserialize(data, ser);
}

template<typename T, typename TContext>
requires Creatable<T, TContext> T dispatchCreate(const Deserializer<TContext>& ser)
{
  if constexpr(MemberCreatable<T, TContext>)
    return T::create(ser);
  else
    return create(TypeId<T>{}, ser);
}
} // namespace serialization::access
