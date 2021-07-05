#pragma once

#include "core/magic.h"
#include "core/units.h"
#include "serialization/serialization_fwd.h"

#include <gsl/gsl-lite.hpp>
#include <vector>

namespace engine::world
{
class World;

using ZoneId = uint32_t;

struct Box
{
  core::Length zmin = 0_len;
  core::Length zmax = 0_len;
  core::Length xmin = 0_len;
  core::Length xmax = 0_len;

  core::Length floor = 0_len;

  mutable bool blocked = true;
  bool blockable = true;

  std::vector<gsl::not_null<Box*>> overlaps{};

  constexpr bool containsX(const core::Length& x) const noexcept
  {
    return x >= xmin && x <= xmax;
  }

  constexpr bool containsZ(const core::Length& z) const noexcept
  {
    return z >= zmin && z <= zmax;
  }

  constexpr bool contains(const core::Length& x, const core::Length& z) const noexcept
  {
    return containsX(x) && containsZ(z);
  }

  ZoneId zoneFly = 0;
  ZoneId zoneFlySwapped = 0;
  ZoneId zoneGround1 = 0;
  ZoneId zoneGround1Swapped = 0;
  ZoneId zoneGround2 = 0;
  ZoneId zoneGround2Swapped = 0;

  static const ZoneId Box::*getZoneRef(const bool swapped, bool isFlying, const core::Length& step)
  {
    if(isFlying)
    {
      return swapped ? &Box::zoneFlySwapped : &Box::zoneFly;
    }
    else if(step == core::QuarterSectorSize)
    {
      return swapped ? &Box::zoneGround1Swapped : &Box::zoneGround1;
    }
    else
    {
      return swapped ? &Box::zoneGround2Swapped : &Box::zoneGround2;
    }
  }

  void serialize(const serialization::Serializer<World>& ser);
};
} // namespace engine::world
