#pragma once

#include "modelobject.h"

#include <gsl/gsl-lite.hpp>
#include <memory>
#include <string>

namespace engine
{
class Particle;
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
class FlameEmitter final : public NullRenderModelObject
{
public:
  FlameEmitter(const gsl::not_null<world::World*>& world, const Location& location)
      : NullRenderModelObject{world, location}
  {
  }

  FlameEmitter(const std::string& name,
               const gsl::not_null<world::World*>& world,
               const gsl::not_null<const world::Room*>& room,
               const loader::file::Item& item,
               const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
      : NullRenderModelObject{name, world, room, item, true, animatedModel}
  {
  }

  void update() override;

private:
  std::shared_ptr<Particle> m_flame;

  void removeParticle();
};
} // namespace engine::objects
