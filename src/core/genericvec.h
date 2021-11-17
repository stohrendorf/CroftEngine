#pragma once

#include "magic.h"
#include "units.h"

#include <glm/vec3.hpp>
#include <gsl/gsl-lite.hpp>

namespace core
{
template<typename T>
struct GenericVec
{
  T X{static_cast<typename T::type>(0)};
  T Y{static_cast<typename T::type>(0)};
  T Z{static_cast<typename T::type>(0)};

  explicit constexpr GenericVec(const glm::vec3& v) noexcept
      : X{gsl::narrow_cast<typename T::type>(v.x)}
      , Y{-gsl::narrow_cast<typename T::type>(v.y)}
      , Z{-gsl::narrow_cast<typename T::type>(v.z)}
  {
  }

  constexpr explicit GenericVec(const glm::ivec3& v) noexcept
      : X{gsl::narrow_cast<typename T::type>(v.x)}
      , Y{-gsl::narrow_cast<typename T::type>(v.y)}
      , Z{-gsl::narrow_cast<typename T::type>(v.z)}
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

  [[nodiscard]] constexpr GenericVec<T> operator-(const GenericVec<T>& rhs) const noexcept
  {
    return {X - rhs.X, Y - rhs.Y, Z - rhs.Z};
  }

  GenericVec<T>& operator-=(const GenericVec<T>& rhs) noexcept
  {
    X -= rhs.X;
    Y -= rhs.Y;
    Z -= rhs.Z;
    return *this;
  }

  template<typename U>
  [[nodiscard]] constexpr auto operator/(const U& n) const noexcept
  {
    return GenericVec<decltype(X / n)>{X / n, Y / n, Z / n};
  }

  template<typename U>
  [[nodiscard]] constexpr auto operator*(const U& n) const noexcept
  {
    return GenericVec<decltype(X * n)>{X * n, Y * n, Z * n};
  }

  [[nodiscard]] constexpr GenericVec<T> operator+(const GenericVec<T>& rhs) const noexcept
  {
    return {X + rhs.X, Y + rhs.Y, Z + rhs.Z};
  }

  GenericVec<T>& operator+=(const GenericVec<T>& rhs) noexcept
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

template<typename T>
inline constexpr bool operator==(const GenericVec<T>& lhs, const GenericVec<T>& rhs) noexcept
{
  return lhs.X == rhs.X && lhs.Y == rhs.Y && lhs.Z == rhs.Z;
}
} // namespace core
