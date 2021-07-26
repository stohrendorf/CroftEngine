#pragma once

#include "modelobject.h"

namespace engine::objects
{
class Switch : public ModelObject
{
public:
  MODELOBJECT_DEFAULT_CONSTRUCTORS(Switch, true)

  void collide(CollisionInfo& collisionInfo) override;

  void update() final
  {
    m_state.activationState.fullyActivate();
    if(!m_state.updateActivationTimeout())
    {
      // reset switch if a timeout is active
      m_state.goal_anim_state = 1_as;
      m_state.timer = 0_frame;
    }

    ModelObject::update();
  }
};
} // namespace engine::objects
