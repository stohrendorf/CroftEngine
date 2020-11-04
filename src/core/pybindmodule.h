#pragma once

#include <optional>
#include <pybind11/embed.h>

namespace core
{
template<typename T, typename U>
std::optional<T> get(const pybind11::dict& d, const U& key)
{
  const auto pyKey = pybind11::cast(key);
  if(!d.contains(pyKey))
    return std::nullopt;
  return d[pyKey].template cast<T>();
}
} // namespace core
