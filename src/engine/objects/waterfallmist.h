#pragma once

#include "modelobject.h"

#include <gsl/gsl-lite.hpp>
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
class WaterfallMist final : public NullRenderModelObject
{
public:
  WaterfallMist(const gsl::not_null<world::World*>& world, const Location& location)
      : NullRenderModelObject{world, location}
  {
  }

  WaterfallMist(const std::string& name,
                const gsl::not_null<world::World*>& world,
                const gsl::not_null<const world::Room*>& room,
                const loader::file::Item& item,
                const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
      : NullRenderModelObject{name, world, room, item, true, animatedModel}
  {
  }

  void update() override;
};
} // namespace engine::objects
