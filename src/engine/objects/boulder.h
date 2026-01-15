#pragma once

#include "engine/location.h"
#include "modelobject.h"
#include "serialization/serialization_fwd.h"

#include <gsl-lite/gsl-lite.hpp>
#include <string>

namespace engine
{
struct CollisionInfo;
}

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
class Boulder final : public ModelObject
{
public:
  Boulder(const gsl_lite::not_null<world::World*>& world, const Location& location)
      : ModelObject{world, location}
      , m_location{location}
  {
  }

  Boulder(const std::string& name,
          const gsl_lite::not_null<world::World*>& world,
          const gsl_lite::not_null<const world::Room*>& room,
          const loader::file::Item& item,
          const gsl_lite::not_null<const world::SkeletalModelType*>& animatedModel);

  void updateLogic() override;

  void collide(CollisionInfo& collisionInfo) override;

  void deserialize(const serialization::Deserializer<world::World>& ser) override;

private:
  Location m_location;
};
} // namespace engine::objects