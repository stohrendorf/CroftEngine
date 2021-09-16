#pragma once

#include "serialization_fwd.h"

#include <gsl/gsl-lite.hpp>

namespace serialization
{
namespace
{
template<typename T>
struct access
{
  template<typename TContext, typename T2>
  static inline auto callSerializeOrLoad(T2& data, const Serializer<TContext>& ser)
    -> decltype(data.serialize(ser), void())
  {
    return data.serialize(ser);
  }

  template<typename TContext, typename T2>
  static inline auto callSerializeOrLoad(T2& data, const Serializer<TContext>& ser)
    -> decltype(serialize(data, ser), void())
  {
    return serialize(data, ser);
  }

  template<typename TContext, typename T2>
  static inline auto callSerializeOrLoad(T2& data, const Serializer<TContext>& ser) -> decltype(data.load(ser), void())
  {
    return data.load(ser);
  }

  template<typename TContext, typename T2>
  static inline auto callSerializeOrLoad(T2& data, const Serializer<TContext>& ser) -> decltype(load(data, ser), void())
  {
    return load(data, ser);
  }

  template<typename TContext, typename T2>
  static inline auto callSerializeOrSave(T2& data, const Serializer<TContext>& ser)
    -> decltype(data.serialize(ser), void())
  {
    return data.serialize(ser);
  }

  template<typename TContext, typename T2>
  static inline auto callSerializeOrSave(T2& data, const Serializer<TContext>& ser)
    -> decltype(serialize(data, ser), void())
  {
    return serialize(data, ser);
  }

  template<typename TContext, typename T2>
  static inline auto callSerializeOrSave(T2& data, const Serializer<TContext>& ser) -> decltype(data.save(ser), void())
  {
    return data.save(ser);
  }

  template<typename TContext, typename T2>
  static inline auto callSerializeOrSave(T2& data, const Serializer<TContext>& ser) -> decltype(save(data, ser), void())
  {
    return save(data, ser);
  }

  template<typename TContext, typename T2>
  static inline void callSerialize(T2& data, const Serializer<TContext>& ser)
  {
    if(ser.loading)
      callSerializeOrLoad(data, ser);
    else
      callSerializeOrSave(data, ser);
  }

  template<typename TContext, typename T2 = T>
  static inline auto callCreate(const Serializer<TContext>& ser) -> decltype(T2::create(ser))
  {
    Expects(ser.loading);
    return T::create(ser);
  }

  template<typename TContext, typename T2 = T>
  static inline auto callCreate(const Serializer<TContext>& ser) -> decltype(create(TypeId<T2>{}, ser))
  {
    Expects(ser.loading);
    return create(TypeId<T>{}, ser);
  }
};
} // namespace
} // namespace serialization
