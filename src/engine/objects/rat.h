#pragma once

#include "aiagent.h"
#include "engine/ai/ai.h"

namespace engine::objects
{
class Rat final : public AIAgent
{
public:
  AIAGENT_DEFAULT_CONSTRUCTORS(Rat)

  void update() override;
};
} // namespace engine::objects
