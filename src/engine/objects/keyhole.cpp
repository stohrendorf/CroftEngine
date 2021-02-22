#include "keyhole.h"

#include "engine/engine.h"
#include "engine/player.h"
#include "engine/presenter.h"
#include "engine/world.h"
#include "hid/inputhandler.h"
#include "laraobject.h"

namespace engine::objects
{
void KeyHole::collide(CollisionInfo& /*collisionInfo*/)
{
  static const InteractionLimits limits{core::BoundingBox{{-200_len, 0_len, 312_len}, {200_len, 0_len, 512_len}},
                                        core::TRRotation{-10_deg, -30_deg, -10_deg},
                                        core::TRRotation{10_deg, 30_deg, 10_deg}};

  if(getWorld().getObjectManager().getLara().getCurrentAnimState() != loader::file::LaraStateId::Stop)
    return;

  if(!getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Action)
     || getWorld().getObjectManager().getLara().getHandStatus() != HandStatus::None
     || getWorld().getObjectManager().getLara().m_state.falling
     || !limits.canInteract(m_state, getWorld().getObjectManager().getLara().m_state))
    return;

  if(m_state.triggerState == TriggerState::Invisible)
  {
    getWorld().getObjectManager().getLara().playSoundEffect(TR1SoundEffect::LaraNo);
    return;
  }

  bool hasKey = false;
  switch(m_state.type.get_as<TR1ItemId>())
  {
  case TR1ItemId::Keyhole1: hasKey = getWorld().getPlayer().getInventory().tryTake(TR1ItemId::Key1); break;
  case TR1ItemId::Keyhole2: hasKey = getWorld().getPlayer().getInventory().tryTake(TR1ItemId::Key2); break;
  case TR1ItemId::Keyhole3: hasKey = getWorld().getPlayer().getInventory().tryTake(TR1ItemId::Key3); break;
  case TR1ItemId::Keyhole4: hasKey = getWorld().getPlayer().getInventory().tryTake(TR1ItemId::Key4); break;
  default: break;
  }
  if(!hasKey)
  {
    getWorld().getObjectManager().getLara().playSoundEffect(TR1SoundEffect::LaraNo);
    return;
  }

  getWorld().getObjectManager().getLara().alignForInteraction(core::TRVec{0_len, 0_len, 362_len}, m_state);

  do
  {
    getWorld().getObjectManager().getLara().setGoalAnimState(loader::file::LaraStateId::InsertKey);
    getWorld().getObjectManager().getLara().updateImpl();
  } while(getWorld().getObjectManager().getLara().getCurrentAnimState() != loader::file::LaraStateId::InsertKey);

  getWorld().getObjectManager().getLara().setGoalAnimState(loader::file::LaraStateId::Stop);
  getWorld().getObjectManager().getLara().setHandStatus(HandStatus::Grabbing);
  m_state.triggerState = TriggerState::Active;
}
} // namespace engine::objects
