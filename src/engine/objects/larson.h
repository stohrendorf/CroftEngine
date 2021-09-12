#pragma once

#include "aiagent.h"

namespace engine::objects
{
class Larson final : public AIAgent
{
public:
  AIAGENT_DEFAULT_CONSTRUCTORS(Larson)

  void update() override;
};
} // namespace engine::objects
