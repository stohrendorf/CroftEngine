#pragma once

#include <gsl/gsl-lite.hpp>
#include <memory>
#include <type_traits>

namespace gslu
{
template<typename T, typename... Args>
gsl::not_null<std::shared_ptr<T>> make_nn_shared(Args&&... args)
{
  return gsl::not_null{std::make_shared<T>(std::forward<Args>(args)...)};
}

template<typename T, typename... Args>
gsl::not_null<std::unique_ptr<T>> make_nn_unique(Args&&... args)
{
  return gsl::not_null{std::make_unique<T>(std::forward<Args>(args)...)};
}

template<typename T, typename U>
inline gsl::not_null<std::shared_ptr<T>> static_pointer_cast(const gsl::not_null<std::shared_ptr<U>>& ptr) noexcept
{
  return gsl::not_null{std::static_pointer_cast<T>(ptr.get())};
}
} // namespace gslu
