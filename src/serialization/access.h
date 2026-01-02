#pragma once

#include "serialization_fwd.h"

namespace serialization::access
{
template<typename T, typename TContext>
concept MemberSerializable = requires(const T& data, const Serializer<TContext>& ser)
{
  data.serialize(ser);
};

template<typename T, typename TContext>
concept NonMemberSerializable = requires(const T& data, const Serializer<TContext>& ser)
{
  serialize(data, ser);
};

template<typename T, typename TContext>
concept MemberDeserializable = requires(T & data, const Deserializer<TContext>& ser)
{
  data.deserialize(ser);
};

template<typename T, typename TContext>
concept NonMemberDeserializable = requires(T & data, const Deserializer<TContext>& ser)
{
  deserialize(data, ser);
};

template<typename T, typename TContext>
concept MemberCreatable = requires(const Deserializer<TContext>& ser)
{
  T::create(ser);
};

template<typename T, typename TContext>
concept NonMemberCreatable = requires(const Deserializer<TContext>& ser)
{
  create(TypeId<T>{}, ser);
};

template<typename T, typename TContext>
requires MemberSerializable<T, TContext> void dispatch(const T& data, const Serializer<TContext>& ser)
{
  data.serialize(ser);
}

template<typename T, typename TContext>
requires NonMemberSerializable<T, TContext> void dispatch(const T& data, const Serializer<TContext>& ser)
{
  serialize(data, ser);
}

template<typename T, typename TContext>
requires MemberDeserializable<T, TContext> void dispatch(T& data, const Deserializer<TContext>& ser)
{
  data.deserialize(ser);
}

template<typename T, typename TContext>
requires NonMemberDeserializable<T, TContext> void dispatch(T& data, const Deserializer<TContext>& ser)
{
  deserialize(data, ser);
}

template<typename T, typename TContext>
requires MemberCreatable<T, TContext> T dispatch(const Deserializer<TContext>& ser)
{
  return T::create(ser);
}

template<typename T, typename TContext>
requires NonMemberCreatable<T, TContext> T dispatch(const Deserializer<TContext>& ser)
{
  return create(TypeId<T>{}, ser);
}
} // namespace serialization::access
