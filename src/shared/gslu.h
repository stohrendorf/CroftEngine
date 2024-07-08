#pragma once

#include <gsl/gsl-lite.hpp>
#include <memory>

namespace gslu
{
template<typename T>
using nn_shared = gsl::not_null<std::shared_ptr<T>>;
template<typename T>
using nn_unique = gsl::not_null<std::unique_ptr<T>>;

template<typename T, typename U>
[[nodiscard]] inline nn_shared<T> static_pointer_cast(const nn_shared<U>& ptr) noexcept
{
  return nn_shared<T>{std::static_pointer_cast<T>(ptr.get())};
}

template<typename T, typename U>
[[nodiscard]] inline std::shared_ptr<T> dynamic_pointer_cast(const nn_shared<U>& ptr) noexcept
{
  return std::dynamic_pointer_cast<T>(ptr.get());
}
} // namespace gslu
