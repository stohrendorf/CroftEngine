#include "midasgoldtouch.h"

#include "engine/particle.h"
#include "engine/player.h"
#include "engine/presenter.h"
#include "engine/world/world.h"
#include "laraobject.h"

namespace engine::objects
{
void MidasGoldTouch::collide(CollisionInfo& info)
{
  auto& lara = getWorld().getObjectManager().getLara();
  if(!lara.m_state.falling && lara.getCurrentAnimState() == loader::file::LaraStateId::Stop
     && m_state.position.position.X - core::SectorSize / 2 < lara.m_state.position.position.X
     && m_state.position.position.X + core::SectorSize / 2 > lara.m_state.position.position.X
     && m_state.position.position.Z - core::SectorSize / 2 < lara.m_state.position.position.Z
     && m_state.position.position.Z + core::SectorSize / 2 > lara.m_state.position.position.Z)
  {
    lara.getSkeleton()->setAnim(&getWorld().findAnimatedModelForType(TR1ItemId::AlternativeLara)->animations[1]);
    lara.setHandStatus(HandStatus::Grabbing);
    getWorld().getPlayer().weaponType = WeaponType::None;
    lara.setCurrentAnimState(loader::file::LaraStateId::MidasDeath);
    lara.setGoalAnimState(loader::file::LaraStateId::MidasDeath);
    lara.m_state.health = core::DeadHealth;
    lara.setAir(-1_frame);
    lara.m_state.falling = false;
    getWorld().getCameraController().setMode(CameraMode::Cinematic);
    getWorld().getCameraController().m_cinematicFrame = 0;
    getWorld().getCameraController().m_cinematicPos = lara.m_state.position.position;
    getWorld().getCameraController().m_cinematicRot = lara.m_state.rotation;
    return;
  }

  if(getWorld().getPlayer().getInventory().count(TR1ItemId::LeadBar) <= 0
     || !getWorld().getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Action)
     || lara.getHandStatus() != HandStatus::None || lara.m_state.falling
     || lara.getCurrentAnimState() != loader::file::LaraStateId::Stop)
    return;

  m_state.rotation.Y = core::snapRotation(core::axisFromAngle(lara.m_state.rotation.Y));

  static const InteractionLimits limits{core::BoundingBox{{-700_len, 284_len, -700_len}, {+700_len, 996_len, +700_len}},
                                        {-10_deg, -30_deg, -10_deg},
                                        {+10_deg, +30_deg, +10_deg}};

  if(!limits.canInteract(m_state, lara.m_state))
    return;

  Expects(getWorld().getPlayer().getInventory().tryTake(TR1ItemId::LeadBar));
  getWorld().getPlayer().getInventory().put(TR1ItemId::Puzzle1Sprite);
  lara.getSkeleton()->setAnim(&getWorld().findAnimatedModelForType(TR1ItemId::AlternativeLara)->animations[0]);
  lara.setCurrentAnimState(loader::file::LaraStateId::UseMidas);
  lara.setGoalAnimState(loader::file::LaraStateId::UseMidas);
  lara.setHandStatus(HandStatus::Grabbing);
}
} // namespace engine::objects
