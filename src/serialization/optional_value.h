#pragma once

#include "access.h"

#include <ryml.hpp>

namespace serialization
{
template<typename T>
struct OptionalValue
{
  T& value;
  explicit OptionalValue(T& value)
      : value{value}
  {
  }
};

#define S_NVO(name, obj)             \
  name, serialization::OptionalValue \
  {                                  \
    obj                              \
  }

} // namespace serialization
