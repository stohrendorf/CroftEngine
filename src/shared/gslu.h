#pragma once

#include <gsl/gsl-lite.hpp>
#include <memory>
#include <type_traits>

namespace gslu
{
template<typename T>
using nn_shared = gsl::not_null<std::shared_ptr<T>>;
template<typename T>
using nn_unique = gsl::not_null<std::unique_ptr<T>>;

template<typename T, typename... Args>
nn_shared<T> make_nn_shared(Args&&... args)
{
  return nn_shared<T>{std::make_shared<T>(std::forward<Args>(args)...)};
}

template<typename T, typename... Args>
nn_unique<T> make_nn_unique(Args&&... args)
{
  return nn_unique<T>{std::make_unique<T>(std::forward<Args>(args)...)};
}

template<typename T, typename U>
inline nn_shared<T> static_pointer_cast(const nn_shared<U>& ptr) noexcept
{
  return nn_shared<T>{std::static_pointer_cast<T>(ptr.get())};
}
} // namespace gslu
