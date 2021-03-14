#pragma once

#include "aiagent.h"
#include "engine/ai/ai.h"

namespace engine::objects
{
class Bear final : public AIAgent
{
public:
  Bear(const gsl::not_null<world::World*>& world, const core::RoomBoundPosition& position)
      : AIAgent{world, position}
  {
  }

  Bear(const gsl::not_null<world::World*>& world,
       const gsl::not_null<const loader::file::Room*>& room,
       const loader::file::Item& item,
       const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
      : AIAgent{world, room, item, animatedModel}
  {
  }

  void update() override;

  void serialize(const serialization::Serializer<world::World>& ser) override;

private:
  bool m_hurt = false;
};
} // namespace engine::objects
