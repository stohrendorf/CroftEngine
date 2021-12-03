#pragma once

#include "magic.h"
#include "units.h"

#include <glm/vec3.hpp>
#include <gsl/gsl-lite.hpp>

namespace core
{
namespace detail
{
template<typename T>
struct UnderlyingType
{
  using type = T;
};

template<typename T, typename U>
struct UnderlyingType<qs::quantity<T, U>>
{
  using type = typename qs::quantity<T, U>::type;
};

template<typename T, typename U>
constexpr auto underlyingCast(const U& v)
{
  return gsl::narrow_cast<typename UnderlyingType<T>::type>(v);
}
} // namespace detail

template<typename T>
struct GenericVec
{
  T X{};
  T Y{};
  T Z{};

  explicit constexpr GenericVec(const glm::vec3& v) noexcept
      : X{detail::underlyingCast<T>(v.x)}
      , Y{-detail::underlyingCast<T>(v.y)}
      , Z{-detail::underlyingCast<T>(v.z)}
  {
  }

  constexpr explicit GenericVec(const glm::ivec3& v) noexcept
      : X{detail::underlyingCast<T>(v.x)}
      , Y{-detail::underlyingCast<T>(v.y)}
      , Z{-detail::underlyingCast<T>(v.z)}
  {
  }

  constexpr GenericVec() noexcept = default;
  constexpr GenericVec(const GenericVec<T>&) noexcept = default;
  constexpr GenericVec(GenericVec<T>&&) noexcept = default;

  constexpr GenericVec(const T& x, const T& y, const T& z) noexcept
      : X{x}
      , Y{y}
      , Z{z}
  {
  }

  constexpr GenericVec<T>& operator=(const GenericVec<T>&) noexcept = default;

  constexpr GenericVec<T>& operator=(GenericVec<T>&&) noexcept = default;

  [[nodiscard]] constexpr GenericVec<T> operator-(const GenericVec<T>& rhs) const noexcept(noexcept(X - rhs.X))
  {
    return {X - rhs.X, Y - rhs.Y, Z - rhs.Z};
  }

  GenericVec<T>& operator-=(const GenericVec<T>& rhs) noexcept(noexcept(X -= rhs.X))
  {
    X -= rhs.X;
    Y -= rhs.Y;
    Z -= rhs.Z;
    return *this;
  }

  template<typename U>
  [[nodiscard]] constexpr auto operator/(const U& n) const noexcept(noexcept(X / n))
  {
    return GenericVec<decltype(X / n)>{X / n, Y / n, Z / n};
  }

  template<typename U, typename V>
  [[nodiscard]] constexpr auto operator*(const U& n) const noexcept(noexcept(X* n))
  {
    return GenericVec<decltype(X * n)>{X * n, Y * n, Z * n};
  }

  [[nodiscard]] constexpr GenericVec<T> operator+(const GenericVec<T>& rhs) const noexcept(noexcept(X + rhs.X))
  {
    return {X + rhs.X, Y + rhs.Y, Z + rhs.Z};
  }

  GenericVec<T>& operator+=(const GenericVec<T>& rhs) noexcept(noexcept(X += rhs.X))
  {
    X += rhs.X;
    Y += rhs.Y;
    Z += rhs.Z;
    return *this;
  }

  [[nodiscard]] glm::vec3 toRenderSystem() const noexcept
  {
    return {gsl::narrow_cast<float>(X.get()), -gsl::narrow_cast<float>(Y.get()), -gsl::narrow_cast<float>(Z.get())};
  }
};

template<typename T, typename U, typename V>
[[nodiscard]] inline constexpr auto operator*(const GenericVec<T>& v,
                                              const qs::quantity<U, V>& n) noexcept(noexcept(v.X* n))
{
  return GenericVec<decltype(v.X * n)>{v.X * n, v.Y * n, v.Z * n};
}

template<typename T>
inline constexpr bool operator==(const GenericVec<T>& lhs, const GenericVec<T>& rhs) noexcept(noexcept(lhs.X == rhs.X))
{
  return lhs.X == rhs.X && lhs.Y == rhs.Y && lhs.Z == rhs.Z;
}
} // namespace core
