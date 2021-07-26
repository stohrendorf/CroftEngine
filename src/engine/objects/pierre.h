#pragma once

#include "aiagent.h"
#include "engine/ai/ai.h"

namespace engine::objects
{
class Pierre final : public AIAgent
{
public:
  AIAGENT_DEFAULT_CONSTRUCTORS(Pierre)

  void update() override;

  void serialize(const serialization::Serializer<world::World>& ser) override;

private:
  core::Frame m_fleeTime = 0_frame;
};
} // namespace engine::objects
