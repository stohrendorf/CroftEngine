#pragma once

#include "serialization.h"

#include <gsl-lite.hpp>
#include <vector>

namespace serialization
{
template<typename T, typename U>
struct VectorElement
{
  const std::vector<T>& vec;
  const T*& element;

  explicit VectorElement(const std::vector<T>& vec, U*& element)
      : vec{vec}
      , element{const_cast<const T*&>(element)}
  {
  }

  template<typename TContext>
  void load(const Serializer<TContext>& ser)
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
  void save(const Serializer<TContext>& ser) const
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

template<typename T, typename U>
struct NotNullVectorElement
{
  const std::vector<T>& vec;
  gsl::not_null<U*>& element;

  explicit NotNullVectorElement(const std::vector<T>& vec, gsl::not_null<U*>& element)
      : vec{vec}
      , element{element}
  {
  }

  template<typename TContext>
  void load(const Serializer<TContext>& ser)
  {
    Expects(!ser.isNull());
    ser.tag("element");
    std::ptrdiff_t n = 0;
    ser.node >> n;
    element = &vec.at(n);
  }

  template<typename TContext>
  void save(const Serializer<TContext>& ser) const
  {
    ser.tag("element");
    ser.node << std::distance(const_cast<const T*>(&vec.at(0)), element.get());
  }
};
} // namespace serialization

#define S_NVVE(name, vec, obj)         \
  name, ::serialization::VectorElement \
  {                                    \
    vec, obj                           \
  }

#define S_NVVENN(name, vec, obj)              \
  name, ::serialization::NotNullVectorElement \
  {                                           \
    vec, obj                                  \
  }
