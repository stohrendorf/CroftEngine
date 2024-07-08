#pragma once

#include "serialization_fwd.h"

namespace serialization
{
template<typename T, bool Loading>
struct access;

template<typename T>
struct access<T, false>
{
  template<typename TContext, typename T2>
  static auto dispatch(const T2& data, const Serializer<TContext>& ser) -> decltype(data.serialize(ser), void())
  {
    static_assert(
      std::is_same_v<std::remove_cv_t<std::remove_reference_t<T>>, std::remove_cv_t<std::remove_reference_t<T2>>>);
    return data.serialize(ser);
  }

  template<typename TContext, typename T2>
  static auto dispatch(const T2& data, const Serializer<TContext>& ser) -> decltype(serialize(data, ser), void())
  {
    static_assert(
      std::is_same_v<std::remove_cv_t<std::remove_reference_t<T>>, std::remove_cv_t<std::remove_reference_t<T2>>>);
    return serialize(data, ser);
  }
};

template<typename T>
struct access<T, true>
{
  template<typename TContext, typename T2>
  static auto dispatch(T2& data, const Deserializer<TContext>& ser) -> decltype(data.deserialize(ser), void())
  {
    static_assert(
      std::is_same_v<std::remove_cv_t<std::remove_reference_t<T>>, std::remove_cv_t<std::remove_reference_t<T2>>>);
    return data.deserialize(ser);
  }

  template<typename TContext, typename T2>
  static auto dispatch(T2& data, const Deserializer<TContext>& ser) -> decltype(deserialize(data, ser), void())
  {
    static_assert(
      std::is_same_v<std::remove_cv_t<std::remove_reference_t<T>>, std::remove_cv_t<std::remove_reference_t<T2>>>);
    return deserialize(data, ser);
  }

  template<typename TContext, typename T2 = T>
  static auto dispatch(const Deserializer<TContext>& ser) -> decltype(T2::create(ser))
  {
    static_assert(
      std::is_same_v<std::remove_cv_t<std::remove_reference_t<T>>, std::remove_cv_t<std::remove_reference_t<T2>>>);
    return T::create(ser);
  }

  template<typename TContext, typename T2 = T>
  static auto dispatch(const Deserializer<TContext>& ser) -> decltype(create(TypeId<T2>{}, ser))
  {
    static_assert(
      std::is_same_v<std::remove_cv_t<std::remove_reference_t<T>>, std::remove_cv_t<std::remove_reference_t<T2>>>);
    return create(TypeId<T>{}, ser);
  }
};
} // namespace serialization
