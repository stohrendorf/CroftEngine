#pragma once

#include "core/angle.h"
#include "core/units.h"

namespace engine
{
class World;
}

namespace menu
{
struct MenuObject;

extern void rotateForSelection(MenuObject& object);
extern void idleRotation(engine::World& world, MenuObject& object);
extern void zeroRotation(MenuObject& object, const core::Angle& speed);

template<typename Unit, typename Type>
auto exactScale(const qs::quantity<Unit, Type>& value, const core::Frame& x, const core::Frame& max)
{
  const auto f = x.cast<float>() / max.cast<float>();
  return (value.template cast<float>() * f).template cast<Type>();
}
} // namespace menu
