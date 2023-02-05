#pragma once

#include "serialization.h"

#include <gsl/gsl-lite.hpp>
#include <vector>

namespace serialization
{
template<typename T>
struct DeserializingVectorElement final
{
  const std::vector<T>& vec;
  const T*& element;

  explicit DeserializingVectorElement(const std::vector<T>& vec, T*& element)
      : vec{vec}
      , element{const_cast<const T*&>(element)}
  {
  }

  explicit DeserializingVectorElement(const std::vector<T>& vec, const T*& element)
      : vec{vec}
      , element{element}
  {
  }

  template<typename TContext>
  void deserialize(const Deserializer<TContext>& ser)
  {
    if(ser.isNull())
    {
      element = nullptr;
      return;
    }

    ser.tag("element");
    std::ptrdiff_t n = 0;
    ser.node >> n;
    element = &vec.at(n);
  }
};

template<typename T>
struct SerializingVectorElement final
{
  const std::vector<T>& vec;
  const T* const& element;

  explicit SerializingVectorElement(const std::vector<T>& vec, const T* const& element)
      : vec{vec}
      , element{element}
  {
  }

  template<typename TContext>
  void serialize(const Serializer<TContext>& ser) const
  {
    if(element == nullptr)
    {
      ser.setNull();
      return;
    }

    ser.tag("element");
    ser.node << std::distance(const_cast<const T*>(&vec.at(0)), element);
  }
};

template<typename T>
struct DeserializingNotNullVectorElement final
{
  std::vector<T>& vec;
  gsl::not_null<const T*>& element;

  explicit DeserializingNotNullVectorElement(std::vector<T>& vec, gsl::not_null<T*>& element)
      : vec{vec}
      , element{element}
  {
  }

  explicit DeserializingNotNullVectorElement(std::vector<T>& vec, gsl::not_null<const T*>& element)
      : vec{vec}
      , element{element}
  {
  }

  template<typename TContext>
  void deserialize(const Deserializer<TContext>& ser)
  {
    Expects(!ser.isNull());
    ser.tag("element");
    std::ptrdiff_t n = 0;
    ser.node >> n;
    element = gsl::not_null{&vec.at(n)};
  }
};

template<typename T>
struct SerializingNotNullVectorElement final
{
  const std::vector<T>& vec;
  const gsl::not_null<const T*>& element;

  explicit SerializingNotNullVectorElement(const std::vector<T>& vec, const gsl::not_null<const T*>& element)
      : vec{vec}
      , element{element}
  {
  }

  template<typename TContext>
  void serialize(const Serializer<TContext>& ser) const
  {
    ser.tag("element");
    ser.node << std::distance(const_cast<const T*>(&vec.at(0)), element.get());
  }
};
} // namespace serialization

#define S_NV_VECTOR_ELEMENT_SERIALIZE(name, vec, obj) \
  name, ::serialization::SerializingVectorElement     \
  {                                                   \
    vec, obj                                          \
  }
#define S_NV_VECTOR_ELEMENT_DESERIALIZE(name, vec, obj) \
  name, ::serialization::DeserializingVectorElement     \
  {                                                     \
    vec, obj                                            \
  }

#define S_NV_VECTOR_ELEMENT_NOT_NULL_SERIALIZE(name, vec, obj) \
  name, ::serialization::SerializingNotNullVectorElement       \
  {                                                            \
    vec, obj                                                   \
  }
#define S_NV_VECTOR_ELEMENT_NOT_NULL_DESERIALIZE(name, vec, obj) \
  name, ::serialization::DeserializingNotNullVectorElement       \
  {                                                              \
    vec, obj                                                     \
  }
