#include "keyhole.h"

#include "hid/inputhandler.h"
#include "laraobject.h"

namespace engine::objects
{
void KeyHole::collide(CollisionInfo& /*collisionInfo*/)
{
  static const InteractionLimits limits{core::BoundingBox{{-200_len, 0_len, 312_len}, {200_len, 0_len, 512_len}},
                                        core::TRRotation{-10_deg, -30_deg, -10_deg},
                                        core::TRRotation{10_deg, 30_deg, 10_deg}};

  if(getEngine().getObjectManager().getLara().getCurrentAnimState() != loader::file::LaraStateId::Stop)
    return;

  if(!getEngine().getInputHandler().getInputState().action
     || getEngine().getObjectManager().getLara().getHandStatus() != HandStatus::None
     || getEngine().getObjectManager().getLara().m_state.falling
     || !limits.canInteract(m_state, getEngine().getObjectManager().getLara().m_state))
    return;

  if(m_state.triggerState == TriggerState::Invisible)
  {
    getEngine().getObjectManager().getLara().playSoundEffect(TR1SoundId::LaraNo);
    return;
  }

  bool hasKey = false;
  switch(m_state.type.get_as<TR1ItemId>())
  {
  case TR1ItemId::Keyhole1: hasKey = getEngine().getInventory().tryTake(TR1ItemId::Key1); break;
  case TR1ItemId::Keyhole2: hasKey = getEngine().getInventory().tryTake(TR1ItemId::Key2); break;
  case TR1ItemId::Keyhole3: hasKey = getEngine().getInventory().tryTake(TR1ItemId::Key3); break;
  case TR1ItemId::Keyhole4: hasKey = getEngine().getInventory().tryTake(TR1ItemId::Key4); break;
  default: break;
  }
  if(!hasKey)
  {
    getEngine().getObjectManager().getLara().playSoundEffect(TR1SoundId::LaraNo);
    return;
  }

  getEngine().getObjectManager().getLara().alignForInteraction(core::TRVec{0_len, 0_len, 362_len}, m_state);

  do
  {
    getEngine().getObjectManager().getLara().setGoalAnimState(loader::file::LaraStateId::InsertKey);
    getEngine().getObjectManager().getLara().updateImpl();
  } while(getEngine().getObjectManager().getLara().getCurrentAnimState() != loader::file::LaraStateId::InsertKey);

  getEngine().getObjectManager().getLara().setGoalAnimState(loader::file::LaraStateId::Stop);
  getEngine().getObjectManager().getLara().setHandStatus(HandStatus::Grabbing);
  m_state.triggerState = TriggerState::Active;
}
} // namespace engine::objects
