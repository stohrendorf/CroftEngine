#pragma once

#include "aiagent.h"
#include "engine/ai/ai.h"

namespace engine::objects
{
class Pierre final : public AIAgent
{
public:
  Pierre(const gsl::not_null<Engine*>& engine, const core::RoomBoundPosition& position)
      : AIAgent{engine, position}
  {
  }

  Pierre(const gsl::not_null<Engine*>& engine,
         const gsl::not_null<const loader::file::Room*>& room,
         const loader::file::Item& item,
         const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel);

  void update() override;
};
} // namespace engine
