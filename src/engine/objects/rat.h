#pragma once

#include "aiagent.h"

namespace engine::objects
{
class Rat final : public AIAgent
{
public:
  AIAGENT_DEFAULT_CONSTRUCTORS(Rat)

  void update() override;
};
} // namespace engine::objects
