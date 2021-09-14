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
class TallBlock final : public ModelObject
{
public:
  TallBlock(const gsl::not_null<world::World*>& world, const Location& location)
      : ModelObject{world, location}
  {
  }

  TallBlock(const std::string& name,
            const gsl::not_null<world::World*>& world,
            const gsl::not_null<const world::Room*>& room,
            const loader::file::Item& item,
            const gsl::not_null<const world::SkeletalModelType*>& animatedModel);

  void update() override;

  void serialize(const serialization::Serializer<world::World>& ser) override;
};
} // namespace engine::objects
