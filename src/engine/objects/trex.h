#pragma once

#include "aiagent.h"
#include "engine/ai/ai.h"

namespace engine::objects
{
class TRex final : public AIAgent
{
public:
  TRex(const gsl::not_null<world::World*>& world, const RoomBoundPosition& location)
      : AIAgent{world, location}
  {
  }

  TRex(const gsl::not_null<world::World*>& world,
       const gsl::not_null<const world::Room*>& room,
       const loader::file::Item& item,
       const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
      : AIAgent{world, room, item, animatedModel}
  {
  }

  void update() override;
  void serialize(const serialization::Serializer<world::World>& ser) override;

private:
  bool m_wantAttack = false;
};
} // namespace engine::objects
