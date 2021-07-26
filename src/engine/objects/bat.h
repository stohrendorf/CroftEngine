#pragma once

#include "aiagent.h"
#include "engine/ai/ai.h"

namespace engine::objects
{
class Bat final : public AIAgent
{
public:
  AIAGENT_DEFAULT_CONSTRUCTORS(Bat)

  void update() override;
};
} // namespace engine::objects
