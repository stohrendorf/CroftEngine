#pragma once

#include "aiagent.h"
#include "core/units.h"
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
class FlyingMutant : public AIAgent
{
public:
  AIAGENT_DEFAULT_CONSTRUCTORS(FlyingMutant)

  void update() final;

  void serialize(const serialization::Serializer<world::World>& ser) override;

private:
  bool m_shootBullet = false;
  bool m_throwGrenade = false;
  bool m_flying = false;
  bool m_lookingAround = false;
};

class WalkingMutant final : public FlyingMutant
{
public:
  WalkingMutant(const gsl::not_null<world::World*>& world, const Location& location)
      : FlyingMutant{world, location}
  {
  }

  WalkingMutant(const std::string& name,
                const gsl::not_null<world::World*>& world,
                const gsl::not_null<const world::Room*>& room,
                const loader::file::Item& item,
                const gsl::not_null<const world::SkeletalModelType*>& animatedModel);
};

class CentaurMutant final : public AIAgent
{
public:
  AIAGENT_DEFAULT_CONSTRUCTORS(CentaurMutant)

  void update() override;
};

class TorsoBoss final : public AIAgent
{
public:
  AIAGENT_DEFAULT_CONSTRUCTORS(TorsoBoss)

  void update() override;

  void serialize(const serialization::Serializer<world::World>& ser) override;

private:
  bool m_hasHitLara = false;
  core::RenderFrame m_turnStartFrame = 0_rframe;
};

} // namespace engine::objects
