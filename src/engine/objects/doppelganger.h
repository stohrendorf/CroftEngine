#pragma once

#include "core/magic.h"
#include "core/units.h"
#include "modelobject.h"
#include "serialization/serialization_fwd.h"

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
class Doppelganger final : public ModelObject
{
public:
  Doppelganger(const gsl::not_null<world::World*>& world, const Location& location)
      : ModelObject{world, location}
  {
  }

  Doppelganger(const std::string& name,
               const gsl::not_null<world::World*>& world,
               const gsl::not_null<const world::Room*>& room,
               const loader::file::Item& item,
               const gsl::not_null<const world::SkeletalModelType*>& animatedModel);

  void update() override;

  void serialize(const serialization::Serializer<world::World>& ser) const override;
  void deserialize(const serialization::Deserializer<world::World>& ser) override;

  void setCenter(const core::Length& x, const core::Length& z)
  {
    m_centerX = x;
    m_centerZ = z;
  }

private:
  bool m_killed = false;
  // defaults for TR1 bacon lara
  core::Length m_centerX = 72_sectors;
  core::Length m_centerZ = 120_sectors;
};
} // namespace engine::objects
