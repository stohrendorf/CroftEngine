#pragma once

#include <functional>

namespace serialization
{
template<typename T>
struct Default
{
  Default(const Default&) = delete;
  Default(Default&&) = delete;
  void operator=(Default&&) = delete;
  void operator=(const Default&) = delete;

  std::reference_wrapper<T> value;
  T defaultValue;

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