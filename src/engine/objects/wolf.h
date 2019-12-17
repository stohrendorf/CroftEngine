#pragma once

#include "aiagent.h"
#include "engine/ai/ai.h"

namespace engine::objects
{
class Wolf final : public AIAgent
{
public:
  Wolf(const gsl::not_null<Engine*>& engine, const core::RoomBoundPosition& position)
      : AIAgent{engine, position}
  {
  }

  Wolf(const gsl::not_null<Engine*>& engine,
       const gsl::not_null<const loader::file::Room*>& room,
       const loader::file::Item& item,
       const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
      : AIAgent{engine, room, item, animatedModel}
  {
  }

  void update() override;
};
} // namespace engine::objects
