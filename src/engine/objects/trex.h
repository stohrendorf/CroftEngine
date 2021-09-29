#pragma once

#include "aiagent.h"
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
class TRex final : public AIAgent
{
public:
  TRex(const gsl::not_null<world::World*>& world, const Location& location)
      : AIAgent{world, location}
  {
  }

  TRex(const std::string& name,
       const gsl::not_null<world::World*>& world,
       const gsl::not_null<const world::Room*>& room,
       const loader::file::Item& item,
       const gsl::not_null<const world::SkeletalModelType*>& animatedModel);

  void update() override;
  void serialize(const serialization::Serializer<world::World>& ser) override;

private:
  bool m_wantAttack = false;
};
} // namespace engine::objects
