#pragma once

#include "aiagent.h"
#include "engine/ai/ai.h"

namespace engine::objects
{
class Crocodile final : public AIAgent
{
public:
  AIAGENT_DEFAULT_CONSTRUCTORS(Crocodile)

  void update() override;

private:
  void updateInWater();
  void updateOnLand();
};
} // namespace engine::objects
