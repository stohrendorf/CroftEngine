#pragma once

#include "aiagent.h"
#include "engine/ai/ai.h"

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
       const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
      : AIAgent{name, world, room, item, animatedModel}
  {
    getSkeleton()->getRenderState().setScissorTest(false);
  }

  void update() override;
  void serialize(const serialization::Serializer<world::World>& ser) override;

private:
  bool m_wantAttack = false;
};
} // namespace engine::objects
