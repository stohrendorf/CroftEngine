#pragma once

#include "serialization.h"

#include <functional>
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

  std::reference_wrapper<const std::vector<T>> vec;
  std::reference_wrapper<const T*> element;

  explicit VectorElement(std::reference_wrapper<const std::vector<T>>&& vec, const std::reference_wrapper<T*>& element)
      : vec{std::move(vec)}
      , element{const_cast<const T*&>(element.get())}
  {
  }

  explicit VectorElement(std::reference_wrapper<const std::vector<T>>&& vec, std::reference_wrapper<const T*>&& element)
      : vec{std::move(vec)}
      , element{std::move(element)}
  {
  }

  explicit VectorElement(std::reference_wrapper<const std::vector<T>>&& vec,
                         const std::reference_wrapper<T* const>& element)
      : vec{std::move(vec)}
      , element{const_cast<const T*&>(element.get())}
  {
  }

  explicit VectorElement(std::reference_wrapper<const std::vector<T>>&& vec,
                         const std::reference_wrapper<const T* const>& element)
      : vec{std::move(vec)}
      , element{const_cast<const T*&>(element.get())}
  {
  }

  template<typename TContext>
  void deserialize(const Deserializer<TContext>& ser)
  {
    if(ser.isNull())
    {
      element.get() = nullptr;
      return;
    }

    ser.tag("element");
    std::ptrdiff_t n = 0;
    ser.node >> n;
    element.get() = &vec.get().at(n);
  }

  template<typename TContext>
  void serialize(const Serializer<TContext>& ser) const
  {
    if(element.get() == nullptr)
    {
      ser.setNull();
      return;
    }

    ser.tag("element");
    ser.node << std::distance(const_cast<const T*>(&vec.get().at(0)), element.get());
  }
};

template<typename T>
struct DeserializingNotNullVectorElement final
{
  DeserializingNotNullVectorElement(const DeserializingNotNullVectorElement<T>&) = delete;
  DeserializingNotNullVectorElement(DeserializingNotNullVectorElement<T>&&) = delete;
  void operator=(DeserializingNotNullVectorElement<T>&&) = delete;
  void operator=(const DeserializingNotNullVectorElement<T>&) = delete;

  std::reference_wrapper<std::vector<T>> vec;
  std::reference_wrapper<gsl::not_null<const T*>> element;

  explicit DeserializingNotNullVectorElement(std::reference_wrapper<std::vector<T>>&& vec,
                                             std::reference_wrapper<gsl::not_null<T*>>&& element)
      : vec{std::move(vec)}
      , element{std::move(element)}
  {
  }

  explicit DeserializingNotNullVectorElement(std::reference_wrapper<std::vector<T>>&& vec,
                                             std::reference_wrapper<gsl::not_null<const T*>>&& element)
      : vec{std::move(vec)}
      , element{std::move(element)}
  {
  }

  template<typename TContext>
  void deserialize(const Deserializer<TContext>& ser)
  {
    gsl_Expects(!ser.isNull());
    ser.tag("element");
    std::ptrdiff_t n = 0;
    ser.node >> n;
    element.get() = gsl::not_null{&vec.get().at(n)};
  }
};

template<typename T>
struct SerializingNotNullVectorElement final
{
  SerializingNotNullVectorElement(const SerializingNotNullVectorElement<T>&) = delete;
  SerializingNotNullVectorElement(SerializingNotNullVectorElement<T>&&) = delete;
  void operator=(SerializingNotNullVectorElement<T>&&) = delete;
  void operator=(const SerializingNotNullVectorElement<T>&) = delete;

  std::reference_wrapper<const std::vector<T>> vec;
  std::reference_wrapper<const gsl::not_null<const T*>> element;

  explicit SerializingNotNullVectorElement(std::reference_wrapper<const std::vector<T>>&& vec,
                                           std::reference_wrapper<const gsl::not_null<const T*>>&& element)
      : vec{std::move(vec)}
      , element{std::move(element)}
  {
  }

  template<typename TContext>
  void serialize(const Serializer<TContext>& ser) const
  {
    ser.tag("element");
    ser.node << std::distance(const_cast<const T*>(&vec.get().at(0)), element.get().get());
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
    std::cref(vec), std::cref(obj)                             \
  }
#define S_NV_VECTOR_ELEMENT_NOT_NULL_DESERIALIZE(name, vec, obj) \
  name, ::serialization::DeserializingNotNullVectorElement       \
  {                                                              \
    std::ref(vec), std::ref(obj)                                 \
  }
