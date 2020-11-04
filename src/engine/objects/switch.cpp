#include "switch.h"

#include "engine/presenter.h"
#include "hid/inputhandler.h"
#include "laraobject.h"

namespace engine::objects
{
void Switch::collide(CollisionInfo& /*collisionInfo*/)
{
  if(!getEngine().getPresenter().getInputHandler().getInputState().action.justChangedTo(true))
    return;

  if(getEngine().getObjectManager().getLara().getHandStatus() != HandStatus::None)
    return;

  if(getEngine().getObjectManager().getLara().m_state.falling)
    return;

  if(m_state.triggerState != TriggerState::Inactive)
    return;

  if(getEngine().getObjectManager().getLara().getCurrentAnimState() != loader::file::LaraStateId::Stop)
    return;

  static const InteractionLimits limits{core::BoundingBox{{-200_len, 0_len, 312_len}, {+200_len, 0_len, 512_len}},
                                        {-10_deg, -30_deg, -10_deg},
                                        {+10_deg, +30_deg, +10_deg}};

  if(!limits.canInteract(m_state, getEngine().getObjectManager().getLara().m_state))
    return;

  getEngine().getObjectManager().getLara().m_state.rotation.Y = m_state.rotation.Y;

  if(m_state.current_anim_state == 1_as)
  {
    do
    {
      getEngine().getObjectManager().getLara().setGoalAnimState(loader::file::LaraStateId::SwitchDown);
      getEngine().getObjectManager().getLara().updateImpl();
    } while(getEngine().getObjectManager().getLara().getCurrentAnimState() != loader::file::LaraStateId::SwitchDown);
    getEngine().getObjectManager().getLara().setGoalAnimState(loader::file::LaraStateId::Stop);
    m_state.goal_anim_state = 0_as;
    getEngine().getObjectManager().getLara().setHandStatus(HandStatus::Grabbing);
  }
  else
  {
    if(m_state.current_anim_state != 0_as)
      return;

    do
    {
      getEngine().getObjectManager().getLara().setGoalAnimState(loader::file::LaraStateId::SwitchUp);
      getEngine().getObjectManager().getLara().updateImpl();
    } while(getEngine().getObjectManager().getLara().getCurrentAnimState() != loader::file::LaraStateId::SwitchUp);
    getEngine().getObjectManager().getLara().setGoalAnimState(loader::file::LaraStateId::Stop);
    m_state.goal_anim_state = 1_as;
    getEngine().getObjectManager().getLara().setHandStatus(HandStatus::Grabbing);
  }

  m_state.triggerState = TriggerState::Active;

  activate();
  ModelObject::update();
}
} // namespace engine::objects
