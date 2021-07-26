#pragma once

#include "aiagent.h"
#include "engine/ai/ai.h"

namespace engine::objects
{
class Lion final : public AIAgent
{
public:
  AIAGENT_DEFAULT_CONSTRUCTORS(Lion)

  void update() override;
};
} // namespace engine::objects
