#pragma once

#include "aiagent.h"

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
