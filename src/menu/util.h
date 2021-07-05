#pragma once

#include "core/angle.h"
#include "core/units.h"

#include <glm/fwd.hpp>
#include <memory>

namespace engine::world
{
class World;
}

namespace menu
{
struct MenuObject;

constexpr int RingInfoYMargin = 28;

extern void rotateForSelection(MenuObject& object);
extern void idleRotation(engine::world::World& world, MenuObject& object, bool force);
extern void zeroRotation(MenuObject& object, const core::Angle& speed);

template<typename Unit, typename Type>
auto exactScale(const qs::quantity<Unit, Type>& value, const core::Frame& x, const core::Frame& max)
{
  const auto f = x.cast<float>() / max.cast<float>();
  return (value.template cast<float>() * f).template cast<Type>();
}
} // namespace menu
