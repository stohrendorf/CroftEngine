#pragma once

#include "aiagent.h"
#include "engine/ai/ai.h"

namespace engine::objects
{
class TRex final : public AIAgent
{
public:
  AIAGENT_DEFAULT_CONSTRUCTORS(TRex)

  void update() override;
  void serialize(const serialization::Serializer<world::World>& ser) override;

private:
  bool m_wantAttack = false;
};
} // namespace engine::objects
