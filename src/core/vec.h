#pragma once

#include "genericvec.h"
#include "qs/qs.h"
#include "serialization/serialization_fwd.h"
#include "units.h"

#include <algorithm>
#include <glm/vec3.hpp>
#include <gsl/gsl-lite.hpp>
#include <iosfwd>

namespace engine::world
{
class World;
} // namespace engine::world

namespace core
{
using TRVec = GenericVec<Length>;

void serialize(TRVec& v, const serialization::Serializer<engine::world::World>& ser);

[[nodiscard]] inline Length distanceTo(const TRVec& lhs, const TRVec& rhs)
{
  const auto dx = gsl::narrow_cast<float>((lhs.X - rhs.X).get());
  const auto dy = gsl::narrow_cast<float>((lhs.Y - rhs.Y).get());
  const auto dz = gsl::narrow_cast<float>((lhs.Z - rhs.Z).get());
  return Length{static_cast<Length::type>(std::sqrt(dx * dx + dy * dy + dz * dz))};
}

[[nodiscard]] constexpr Length absMax(const TRVec& v)
{
  return std::max(std::max(abs(v.X), abs(v.Y)), abs(v.Z));
}

[[nodiscard]] inline Length length(const TRVec& v)
{
  return sqrt(v.X * v.X + v.Y * v.Y + v.Z * v.Z);
}

extern std::ostream& operator<<(std::ostream& stream, const TRVec& rhs);
} // namespace core
