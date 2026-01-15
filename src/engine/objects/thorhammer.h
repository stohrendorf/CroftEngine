#pragma once

#include "modelobject.h"
#include "serialization/serialization_fwd.h"

#include <gsl-lite/gsl-lite.hpp>
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
  ThorHammerBlock(const gsl_lite::not_null<world::World*>& world, const Location& location)
      : ModelObject{world, location}
  {
  }

  ThorHammerBlock(const std::string& name,
                  const gsl_lite::not_null<world::World*>& world,
                  const gsl_lite::not_null<const world::Room*>& room,
                  const loader::file::Item& item,
                  const gsl_lite::not_null<const world::SkeletalModelType*>& animatedModel);

  void collide(CollisionInfo& info) override;

  void serialize(const serialization::Serializer<world::World>& ser) const override;
  void deserialize(const serialization::Deserializer<world::World>& ser) override;

  void updateLogic() override;
};

class ThorHammerHandle final : public ModelObject
{
public:
  ThorHammerHandle(const gsl_lite::not_null<world::World*>& world, const Location& location);

  ThorHammerHandle(const std::string& name,
                   const gsl_lite::not_null<world::World*>& world,
                   const gsl_lite::not_null<const world::Room*>& room,
                   loader::file::Item item,
                   const gsl_lite::not_null<const world::SkeletalModelType*>& animatedModel);

  void updateLogic() override;

  void collide(CollisionInfo& info) override;

  void serialize(const serialization::Serializer<world::World>& ser) const override;
  void deserialize(const serialization::Deserializer<world::World>& ser) override;

private:
  std::shared_ptr<ThorHammerBlock> m_block;
};
} // namespace engine::objects