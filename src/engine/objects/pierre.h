#pragma once

#include "aiagent.h"
#include "engine/ai/ai.h"

namespace engine::objects
{
class Pierre final : public AIAgent
{
public:
  Pierre(const gsl::not_null<world::World*>& world, const RoomBoundPosition& location)
      : AIAgent{world, location}
  {
  }

  Pierre(const gsl::not_null<world::World*>& world,
         const gsl::not_null<const world::Room*>& room,
         const loader::file::Item& item,
         const gsl::not_null<const world::SkeletalModelType*>& animatedModel);

  void update() override;

  void serialize(const serialization::Serializer<world::World>& ser) override;

private:
  core::Frame m_fleeTime = 0_frame;
};
} // namespace engine::objects
