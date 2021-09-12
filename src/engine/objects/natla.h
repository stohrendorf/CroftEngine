#pragma once

#include "aiagent.h"

namespace engine::objects
{
class Natla final : public AIAgent
{
public:
  AIAGENT_DEFAULT_CONSTRUCTORS(Natla)

  void update() override;

  void serialize(const serialization::Serializer<world::World>& ser) override;

private:
  bool m_attemptToFly = false;
  core::Frame m_flyTime = 0_frame;
  core::Angle m_pitchDelta = 0_deg;
};
} // namespace engine::objects
