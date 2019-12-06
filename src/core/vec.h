#pragma once

#include "gsl-lite.hpp"
#include "serialization/serialization.h"
#include "units.h"

#include <glm/glm.hpp>
#include <sol/sol.hpp>
#include <utility>

namespace loader::file
{
struct Room;
} // namespace loader::file

namespace core
{
struct TRVec
{
  Length X = 0_len, Y = 0_len, Z = 0_len;

  constexpr TRVec() noexcept = default;

  constexpr TRVec(const TRVec&) noexcept = default;

  constexpr TRVec(TRVec&&) noexcept = default;

  explicit constexpr TRVec(const glm::vec3& v) noexcept
      : X{gsl::narrow_cast<Length::type>(v.x)}
      , Y{-gsl::narrow_cast<Length::type>(v.y)}
      , Z{-gsl::narrow_cast<Length::type>(v.z)}
  {
  }

  constexpr explicit TRVec(const glm::ivec3& v) noexcept
      : X{gsl::narrow_cast<Length::type>(v.x)}
      , Y{-gsl::narrow_cast<Length::type>(v.y)}
      , Z{-gsl::narrow_cast<Length::type>(v.z)}
  {
  }

  constexpr TRVec(const Length& x, const Length& y, const Length& z) noexcept
      : X{x}
      , Y{y}
      , Z{z}
  {
  }

  ~TRVec() noexcept = default;

  constexpr TRVec& operator=(const TRVec&) noexcept = default;

  constexpr TRVec& operator=(TRVec&&) noexcept = default;

  [[nodiscard]] constexpr TRVec operator-(const TRVec& rhs) const noexcept
  {
    return {X - rhs.X, Y - rhs.Y, Z - rhs.Z};
  }

  TRVec& operator-=(const TRVec& rhs) noexcept
  {
    X -= rhs.X;
    Y -= rhs.Y;
    Z -= rhs.Z;
    return *this;
  }

  [[nodiscard]] constexpr TRVec operator/(const Length::type n) const noexcept
  {
    return {X / n, Y / n, Z / n};
  }

  TRVec& operator/=(const int n) noexcept
  {
    X /= n;
    Y /= n;
    Z /= n;
    return *this;
  }

  [[nodiscard]] constexpr TRVec operator+(const TRVec& rhs) const noexcept
  {
    return {X + rhs.X, Y + rhs.Y, Z + rhs.Z};
  }

  TRVec& operator+=(const TRVec& rhs) noexcept
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

  [[nodiscard]] Length distanceTo(const TRVec& rhs) const
  {
    const auto dx = gsl::narrow<float>((X - rhs.X).get());
    const auto dy = gsl::narrow<float>((Y - rhs.Y).get());
    const auto dz = gsl::narrow<float>((Z - rhs.Z).get());
    return Length{static_cast<Length::type>(glm::sqrt(dx * dx + dy * dy + dz * dz))};
  }

  static void registerUserType(sol::state& lua)
  {
    lua.new_usertype<TRVec>("Vec",
                            sol::constructors<TRVec(), TRVec(Length, Length, Length)>(),
                            "x",
                            &TRVec::X,
                            "y",
                            &TRVec::Y,
                            "z",
                            &TRVec::Z);
  }

  void serialize(const serialization::Serializer& ser);

  [[nodiscard]] Length length() const
  {
    return sqrt(X * X + Y * Y + Z * Z);
  }

  [[nodiscard]] Length absMax() const
  {
    return std::max(std::max(abs(X), abs(Y)), abs(Z));
  }
};

struct RoomBoundPosition final
{
  gsl::not_null<const loader::file::Room*> room;

  TRVec position;

  explicit RoomBoundPosition(gsl::not_null<const loader::file::Room*> r, TRVec pos = {})
      : room{std::move(r)}
      , position{std::move(pos)}
  {
  }

  void serialize(const serialization::Serializer& ser);
  [[nodiscard]] static RoomBoundPosition create(const serialization::Serializer& ser);
};
} // namespace core
