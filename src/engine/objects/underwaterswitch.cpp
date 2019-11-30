#include "underwaterswitch.h"

#include "hid/inputhandler.h"
#include "laraobject.h"

namespace engine::objects
{
void UnderwaterSwitch::collide(CollisionInfo& /*collisionInfo*/)
{
  if(!getEngine().getInputHandler().getInputState().action)
    return;

  if(m_state.triggerState != TriggerState::Inactive)
    return;

  if(!getEngine().getLara().isDiving())
    return;

  if(getEngine().getLara().getCurrentAnimState() != loader::file::LaraStateId::UnderwaterStop)
    return;

  static const InteractionLimits limits{
    core::BoundingBox{{-1024_len, -1024_len, -1024_len}, {1024_len, 1024_len, 512_len}},
    {-80_deg, -80_deg, -80_deg},
    {+80_deg, +80_deg, +80_deg}};

  if(!limits.canInteract(m_state, getEngine().getLara().m_state))
    return;

  if(m_state.current_anim_state != 0_as && m_state.current_anim_state != 1_as)
    return;

  static const core::TRVec alignSpeed{0_len, 0_len, 108_len};
  if(!getEngine().getLara().alignTransform(alignSpeed, *this))
    return;

  getEngine().getLara().m_state.fallspeed = 0_spd;
  do
  {
    getEngine().getLara().setGoalAnimState(loader::file::LaraStateId::SwitchDown);
    getEngine().getLara().updateImpl();
  } while(getEngine().getLara().getCurrentAnimState() != loader::file::LaraStateId::SwitchDown);
  getEngine().getLara().setGoalAnimState(loader::file::LaraStateId::UnderwaterStop);
  getEngine().getLara().setHandStatus(HandStatus::Grabbing);
  m_state.triggerState = TriggerState::Active;

  if(m_state.current_anim_state == 1_as)
  {
    m_state.goal_anim_state = 0_as;
  }
  else
  {
    m_state.goal_anim_state = 1_as;
  }

  activate();
  ModelObject::update(); // NOLINT(bugprone-parent-virtual-call)
}
} // namespace engine
