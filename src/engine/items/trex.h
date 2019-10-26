#pragma once

#include "aiagent.h"
#include "engine/ai/ai.h"

namespace engine
{
namespace items
{
class TRex final : public AIAgent
{
  public:
  TRex(const gsl::not_null<Engine*>& engine,
       const gsl::not_null<const loader::file::Room*>& room,
       const loader::file::Item& item,
       const loader::file::SkeletalModelType& animatedModel)
      : AIAgent{engine, room, item, animatedModel}
  {
  }

  void update() override;
};
} // namespace items
} // namespace engine
