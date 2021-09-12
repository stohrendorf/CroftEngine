#pragma once

#include "aiagent.h"

namespace engine::objects
{
class Lion final : public AIAgent
{
public:
  AIAGENT_DEFAULT_CONSTRUCTORS(Lion)

  void update() override;
};
} // namespace engine::objects
