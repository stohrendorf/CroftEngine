#pragma once

#include "access.h"

#include <ryml.hpp>

namespace serialization
{
template<typename T>
struct Default
{
  Default(const Default<T>&) = delete;
  Default(Default<T>&&) = delete;
  void operator=(Default<T>&&) = delete;
  void operator=(const Default<T>&) = delete;

  std::reference_wrapper<T> value;
  const T defaultValue;
  explicit Default(std::reference_wrapper<T>&& value, const T& defaultValue)
      : value{std::move(value)}
      , defaultValue{defaultValue}
  {
  }
};

#define S_NVD(name, obj, default) \
  name, serialization::Default    \
  {                               \
    obj, default                  \
  }

} // namespace serialization
