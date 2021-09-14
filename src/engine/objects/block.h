#pragma once

#include "core/angle.h"
#include "core/units.h"
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
class Block final : public ModelObject
{
public:
  Block(const gsl::not_null<world::World*>& world, const Location& location)
      : ModelObject{world, location}
  {
  }

  Block(const std::string& name,
        const gsl::not_null<world::World*>& world,
        const gsl::not_null<const world::Room*>& room,
        const loader::file::Item& item,
        const gsl::not_null<const world::SkeletalModelType*>& animatedModel);

  void collide(CollisionInfo& collisionInfo) override;

  void update() override;

  void serialize(const serialization::Serializer<world::World>& ser) override;

private:
  bool isOnFloor(const core::Length& height) const;

  bool canPushBlock(const core::Length& height, core::Axis axis) const;

  bool canPullBlock(const core::Length& height, core::Axis axis) const;
};
} // namespace engine::objects
