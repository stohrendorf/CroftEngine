#pragma once

#include "core/angle.h"
#include "modelobject.h"

#include <gsl/gsl-lite.hpp>
#include <string>

namespace engine
{
struct CollisionInfo;
struct Location;
} // namespace engine

namespace engine::world
{
class World;
struct Room;
struct SkeletalModelType;
} // namespace engine::world

namespace loader::file
{
struct Item;
}

namespace engine::objects
{
class Mummy final : public ModelObject
{
public:
  Mummy(const gsl::not_null<world::World*>& world, const Location& location)
      : ModelObject{world, location}
  {
  }

  Mummy(const std::string& name,
        const gsl::not_null<world::World*>& world,
        const gsl::not_null<const world::Room*>& room,
        const loader::file::Item& item,
        const gsl::not_null<const world::SkeletalModelType*>& animatedModel);

  void update() override;

  void collide(CollisionInfo& info) override;

private:
  core::Angle m_headRotation{0_deg};
};
} // namespace engine::objects
