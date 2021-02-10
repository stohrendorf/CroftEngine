#pragma once

#include "access.h"

#include <ryml.hpp>

namespace serialization
{
template<typename T>
struct Default
{
  T& value;
  const T defaultValue;
  explicit Default(T& value, const T& defaultValue)
      : value{value}
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
