#pragma once

#include <concepts>

namespace serialization
{
template<typename T>
struct TypeId;

template<bool Loading, typename TContext>
class BaseSerializer;

template<typename TContext>
using Serializer = BaseSerializer<false, TContext>;
template<typename TContext>
using Deserializer = BaseSerializer<true, TContext>;

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
concept Serializable = MemberSerializable<T, TContext> || NonMemberSerializable<T, TContext>;

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
concept Deserializable = MemberDeserializable<T, TContext> || NonMemberDeserializable<T, TContext>;

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
concept Creatable = MemberCreatable<T, TContext> || NonMemberCreatable<T, TContext>;

} // namespace serialization
