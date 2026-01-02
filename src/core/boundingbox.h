#pragma once

#include "core/units.h"
#include "interval.h"
#include "vec.h"

namespace core
{
struct BoundingBox
{
  Interval<Length> x{0_len, 0_len};
  Interval<Length> y{0_len, 0_len};
  Interval<Length> z{0_len, 0_len};

  explicit BoundingBox() = default;

  explicit BoundingBox(const Length& minX,
                       // NOLINTNEXTLINE(*-easily-swappable-parameters)
                       const Length& maxX,
                       const Length& minY,
                       // NOLINTNEXTLINE(*-easily-swappable-parameters)
                       const Length& maxY,
                       const Length& minZ,
                       const Length& maxZ) noexcept
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

  [[nodiscard]] bool intersectsExclusive(const BoundingBox& b) const noexcept
  {
    return x.intersectsExclusive(b.x) && y.intersectsExclusive(b.y) && z.intersectsExclusive(b.z);
  }
};
} // namespace core
