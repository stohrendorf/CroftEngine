#pragma once

#include "aiagent.h"

#include <gsl-lite/gsl-lite.hpp>
#include <string>

namespace engine
{
struct Location;
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
class Wolf final : public AIAgent
{
public:
  Wolf(const gsl_lite::not_null<world::World*>& world, const Location& location)
      : AIAgent{world, location}
  {
  }

  Wolf(const std::string& name,
       const gsl_lite::not_null<world::World*>& world,
       const gsl_lite::not_null<const world::Room*>& room,
       const loader::file::Item& item,
       const gsl_lite::not_null<const world::SkeletalModelType*>& animatedModel);

  void updateLogic() override;
};
} // namespace engine::objects