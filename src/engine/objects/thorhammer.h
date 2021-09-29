#pragma once

#include "modelobject.h"
#include "serialization/serialization_fwd.h"

#include <gsl/gsl-lite.hpp>
#include <memory>
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
class ThorHammerBlock final : public ModelObject
{
public:
  ThorHammerBlock(const gsl::not_null<world::World*>& world, const Location& location)
      : ModelObject{world, location}
  {
  }

  ThorHammerBlock(const std::string& name,
                  const gsl::not_null<world::World*>& world,
                  const gsl::not_null<const world::Room*>& room,
                  const loader::file::Item& item,
                  const gsl::not_null<const world::SkeletalModelType*>& animatedModel);

  void collide(CollisionInfo& info) override;

  void serialize(const serialization::Serializer<world::World>& ser) override;
};

class ThorHammerHandle final : public ModelObject
{
public:
  ThorHammerHandle(const gsl::not_null<world::World*>& world, const Location& location);

  ThorHammerHandle(const std::string& name,
                   const gsl::not_null<world::World*>& world,
                   const gsl::not_null<const world::Room*>& room,
                   loader::file::Item item,
                   const gsl::not_null<const world::SkeletalModelType*>& animatedModel);

  void update() override;

  void collide(CollisionInfo& info) override;

  void serialize(const serialization::Serializer<world::World>& ser) override;

private:
  std::shared_ptr<ThorHammerBlock> m_block;
};
} // namespace engine::objects
