#pragma once

#include "aiagent.h"
#include "engine/ai/ai.h"

namespace engine::objects
{
class Raptor final : public AIAgent
{
public:
  AIAGENT_DEFAULT_CONSTRUCTORS(Raptor)

  void update() override;
};
} // namespace engine::objects
