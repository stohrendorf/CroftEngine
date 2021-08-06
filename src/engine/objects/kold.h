#pragma once

#include "aiagent.h"
#include "engine/ai/ai.h"

namespace engine::objects
{
class Kold final : public AIAgent
{
public:
  Kold(const gsl::not_null<world::World*>& world, const Location& location)
      : AIAgent{world, location}
  {
  }

  Kold(const gsl::not_null<world::World*>& world,
       const gsl::not_null<const world::Room*>& room,
       const loader::file::Item& item,
       const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
      : AIAgent{world, room, item, animatedModel}
  {
    m_state.current_anim_state = 3_as;
  }

  void update() override;

  void serialize(const serialization::Serializer<world::World>& ser) override;

private:
  bool m_shotAtLara = false;
};
} // namespace engine::objects
