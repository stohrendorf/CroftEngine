#include "switch.h"

#include "core/angle.h"
#include "core/boundingbox.h"
#include "engine/objectmanager.h"
#include "engine/presenter.h"
#include "engine/world/world.h"
#include "hid/actions.h"
#include "hid/inputhandler.h"
#include "laraobject.h"
#include "loader/file/larastateid.h"
#include "modelobject.h"
#include "object.h"
#include "objectstate.h"
#include "qs/quantity.h"

namespace engine::objects
{
void Switch::collide(CollisionInfo& /*collisionInfo*/)
{
  if(!getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Action))
    return;

  if(getWorld().getObjectManager().getLara().getHandStatus() != HandStatus::None)
    return;

  if(getWorld().getObjectManager().getLara().m_state.falling)
    return;

  if(m_state.triggerState != TriggerState::Inactive)
    return;

  if(getWorld().getObjectManager().getLara().getCurrentAnimState() != loader::file::LaraStateId::Stop)
    return;

  static const InteractionLimits limits{core::BoundingBox{{-200_len, 0_len, 312_len}, {+200_len, 0_len, 512_len}},
                                        {-10_deg, -30_deg, -10_deg},
                                        {+10_deg, +30_deg, +10_deg}};

  if(!limits.canInteract(m_state, getWorld().getObjectManager().getLara().m_state))
    return;

  getWorld().getObjectManager().getLara().m_state.rotation.Y = m_state.rotation.Y;

  if(m_state.current_anim_state == 1_as)
  {
    do
    {
      getWorld().getObjectManager().getLara().setGoalAnimState(loader::file::LaraStateId::SwitchDown);
      getWorld().getObjectManager().getLara().advanceFrame();
    } while(getWorld().getObjectManager().getLara().getCurrentAnimState() != loader::file::LaraStateId::SwitchDown);
    getWorld().getObjectManager().getLara().setGoalAnimState(loader::file::LaraStateId::Stop);
    m_state.goal_anim_state = 0_as;
    getWorld().getObjectManager().getLara().setHandStatus(HandStatus::Grabbing);
  }
  else
  {
    if(m_state.current_anim_state != 0_as)
      return;

    do
    {
      getWorld().getObjectManager().getLara().setGoalAnimState(loader::file::LaraStateId::SwitchUp);
      getWorld().getObjectManager().getLara().advanceFrame();
    } while(getWorld().getObjectManager().getLara().getCurrentAnimState() != loader::file::LaraStateId::SwitchUp);
    getWorld().getObjectManager().getLara().setGoalAnimState(loader::file::LaraStateId::Stop);
    m_state.goal_anim_state = 1_as;
    getWorld().getObjectManager().getLara().setHandStatus(HandStatus::Grabbing);
  }

  m_state.triggerState = TriggerState::Active;

  activate();
  ModelObject::update();
}
} // namespace engine::objects
