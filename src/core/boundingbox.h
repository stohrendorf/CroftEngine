#pragma once

#include "interval.h"
#include "vec.h"

namespace core
{
struct BoundingBox
{
  core::Interval<core::Length> x{0_len, 0_len};
  core::Interval<core::Length> y{0_len, 0_len};
  core::Interval<core::Length> z{0_len, 0_len};

  explicit BoundingBox() = default;

  explicit BoundingBox(const core::Length& minX,
                       const core::Length& maxX,
                       const core::Length& minY,
                       const core::Length& maxY,
                       const core::Length& minZ,
                       const core::Length& maxZ) noexcept
      : x{minX, maxX}
      , y{minY, maxY}
      , z{minZ, maxZ}
  {
  }

  explicit BoundingBox(const BoundingBox& a, const BoundingBox& b, const float bias)
      : x{lerp(a.x.min, b.x.min, bias), lerp(a.x.max, b.x.max, bias)}
      , y{lerp(a.y.min, b.y.min, bias), lerp(a.y.max, b.y.max, bias)}
      , z{lerp(a.z.min, b.z.min, bias), lerp(a.z.max, b.z.max, bias)}
  {
  }

  explicit BoundingBox(const TRVec& min, const TRVec& max) noexcept
      : x{min.X, max.X}
      , y{min.Y, max.Y}
      , z{min.Z, max.Z}
  {
  }

  void sanitize()
  {
    x = x.sanitized();
    y = y.sanitized();
    z = z.sanitized();
  }

  [[nodiscard]] bool contains(const TRVec& v) const noexcept
  {
    return x.contains(v.X) && y.contains(v.Y) && z.contains(v.Z);
  }

  [[nodiscard]] bool intersectsExclusive(const core::BoundingBox& b) const noexcept
  {
    return x.intersectsExclusive(b.x) && y.intersectsExclusive(b.y) && z.intersectsExclusive(b.z);
  }
};
} // namespace core
