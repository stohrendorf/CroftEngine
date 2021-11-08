#pragma once

#include "magic.h"
#include "serialization/serialization_fwd.h"
#include "units.h"

namespace engine::world
{
class World;
}

namespace core
{
// based on https://en.wikipedia.org/wiki/Verlet_integration#Velocity_Verlet
struct Verlet
{
  Speed velocity{0_spd};
  Acceleration acceleration{0_spd / 1_frame};

  static constexpr auto dt = toRenderUnit((1_rframe).cast<float>());

  void operator=(const Speed& v)
  {
    velocity = v;
  }

  void operator+=(const Acceleration& delta)
  {
    acceleration += delta;
  }

  bool operator<(const Speed& v) const
  {
    return velocity < v;
  }

  bool operator<=(const Speed& v) const
  {
    return velocity <= v;
  }

  bool operator>(const Speed& v) const
  {
    return velocity > v;
  }

  bool operator>=(const Speed& v) const
  {
    return velocity >= v;
  }

  Length nextFrame()
  {
    const auto dx = velocity * dt + acceleration * dt * dt * 0.5f;
    velocity += acceleration * dt;
    acceleration = 0_spd / 1_frame;
    return dx;
  }

  void stop(const Speed& finalVelocity = 0_spd)
  {
    velocity = finalVelocity;
    acceleration = 0_spd / 1_frame;
  }

  void serialize(const serialization::Serializer<engine::world::World>& ser);
};
} // namespace core
