#pragma once

#include <functional>

namespace serialization
{
template<typename T>
struct OptionalValue final
{
  OptionalValue(const OptionalValue<T>&) = delete;
  OptionalValue(OptionalValue<T>&&) = delete;
  void operator=(OptionalValue<T>&&) = delete;
  void operator=(const OptionalValue<T>&) = delete;

  std::reference_wrapper<T> value;
  explicit OptionalValue(std::reference_wrapper<T>&& value)
      : value{std::move(value)}
  {
  }
};

#define S_NVO(name, obj)             \
  name, serialization::OptionalValue \
  {                                  \
    obj                              \
  }

} // namespace serialization
