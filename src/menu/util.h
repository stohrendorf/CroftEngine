#pragma once

#include "core/units.h"
#include "qs/qs.h"

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
auto exactScale(const qs::quantity<Unit, Type>& value, const core::RenderFrame& x, const core::RenderFrame& max)
{
  const auto f = x.cast<float>() / max.cast<float>();
  return (value.template cast<float>() * f).template cast<Type>();
}
} // namespace menu
