#pragma once

#include <optional>
#include <pybind11/cast.h>
#include <pybind11/pytypes.h>

namespace engine::script
{
template<typename T, typename U>
std::optional<T> get(const pybind11::dict& d, const U& key)
{
  const auto pyKey = pybind11::cast(key);
  if(!d.contains(pyKey))
    return std::nullopt;
  return d[pyKey].template cast<T>();
}
} // namespace engine::script
