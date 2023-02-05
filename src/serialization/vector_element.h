#pragma once

#include "serialization.h"

#include <gsl/gsl-lite.hpp>
#include <vector>

namespace serialization
{
template<typename T>
struct VectorElement final
{
  VectorElement(const VectorElement<T>&) = delete;
  VectorElement(VectorElement<T>&&) = delete;
  void operator=(VectorElement<T>&&) = delete;
  void operator=(const VectorElement<T>&) = delete;

  const std::vector<T>& vec;
  const T*& element;

  explicit VectorElement(const std::vector<T>& vec, T*& element)
      : vec{vec}
      , element{const_cast<const T*&>(element)}
  {
  }

  explicit VectorElement(const std::vector<T>& vec, const T*& element)
      : vec{vec}
      , element{element}
  {
  }

  explicit VectorElement(const std::vector<T>& vec, const T* const& element)
      : vec{vec}
      , element{const_cast<const T*&>(element)}
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
  DeserializingNotNullVectorElement(const OptionalValue<T>&) = delete;
  DeserializingNotNullVectorElement(DeserializingNotNullVectorElement<T>&&) = delete;
  void operator=(DeserializingNotNullVectorElement<T>&&) = delete;
  void operator=(const DeserializingNotNullVectorElement<T>&) = delete;

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
  SerializingNotNullVectorElement(const SerializingNotNullVectorElement<T>&) = delete;
  SerializingNotNullVectorElement(SerializingNotNullVectorElement<T>&&) = delete;
  void operator=(SerializingNotNullVectorElement<T>&&) = delete;
  void operator=(const SerializingNotNullVectorElement<T>&) = delete;

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

#define S_NV_VECTOR_ELEMENT(name, vec, obj) \
  name, ::serialization::VectorElement      \
  {                                         \
    vec, obj                                \
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
