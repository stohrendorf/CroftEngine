#include "switch.h"

#include "engine/laranode.h"
#include "hid/inputhandler.h"

namespace engine
{
namespace items
{
void Switch::collide(CollisionInfo& /*collisionInfo*/)
{
  if(!getEngine().getInputHandler().getInputState().action)
    return;

  if(getEngine().getLara().getHandStatus() != HandStatus::None)
    return;

  if(getEngine().getLara().m_state.falling)
    return;

  if(m_state.triggerState != TriggerState::Inactive)
    return;

  if(getEngine().getLara().getCurrentAnimState() != LaraStateId::Stop)
    return;

  static const InteractionLimits limits{core::BoundingBox{{-200_len, 0_len, 312_len}, {+200_len, 0_len, 512_len}},
                                        {-10_deg, -30_deg, -10_deg},
                                        {+10_deg, +30_deg, +10_deg}};

  if(!limits.canInteract(m_state, getEngine().getLara().m_state))
    return;

  getEngine().getLara().m_state.rotation.Y = m_state.rotation.Y;

  if(m_state.current_anim_state == 1_as)
  {
    do
    {
      getEngine().getLara().setGoalAnimState(LaraStateId::SwitchDown);
      getEngine().getLara().updateImpl();
    } while(getEngine().getLara().getCurrentAnimState() != LaraStateId::SwitchDown);
    getEngine().getLara().setGoalAnimState(LaraStateId::Stop);
    m_state.goal_anim_state = 0_as;
    getEngine().getLara().setHandStatus(HandStatus::Grabbing);
  }
  else
  {
    if(m_state.current_anim_state != 0_as)
      return;

    do
    {
      getEngine().getLara().setGoalAnimState(LaraStateId::SwitchUp);
      getEngine().getLara().updateImpl();
    } while(getEngine().getLara().getCurrentAnimState() != LaraStateId::SwitchUp);
    getEngine().getLara().setGoalAnimState(LaraStateId::Stop);
    m_state.goal_anim_state = 1_as;
    getEngine().getLara().setHandStatus(HandStatus::Grabbing);
  }

  m_state.triggerState = TriggerState::Active;

  activate();
  ModelItemNode::update();
}
} // namespace items
} // namespace engine
