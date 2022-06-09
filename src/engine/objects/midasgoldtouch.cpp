#include "midasgoldtouch.h"

#include "core/angle.h"
#include "core/boundingbox.h"
#include "core/magic.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine/cameracontroller.h"
#include "engine/inventory.h"
#include "engine/items_tr1.h"
#include "engine/location.h"
#include "engine/objectmanager.h"
#include "engine/player.h"
#include "engine/presenter.h"
#include "engine/skeletalmodelnode.h"
#include "engine/weapontype.h"
#include "engine/world/animation.h"
#include "engine/world/skeletalmodeltype.h"
#include "engine/world/world.h"
#include "hid/actions.h"
#include "hid/inputhandler.h"
#include "laraobject.h"
#include "loader/file/larastateid.h"
#include "object.h"
#include "objectstate.h"
#include "qs/qs.h"

#include <memory>

namespace engine::objects
{
void MidasGoldTouch::collide(CollisionInfo& /*info*/)
{
  auto& lara = getWorld().getObjectManager().getLara();
  if(!lara.m_state.falling && lara.getCurrentAnimState() == loader::file::LaraStateId::Stop
     && m_state.location.position.X - 1_sectors / 2 < lara.m_state.location.position.X
     && m_state.location.position.X + 1_sectors / 2 > lara.m_state.location.position.X
     && m_state.location.position.Z - 1_sectors / 2 < lara.m_state.location.position.Z
     && m_state.location.position.Z + 1_sectors / 2 > lara.m_state.location.position.Z)
  {
    lara.getSkeleton()->setAnim(
      gsl::not_null{&getWorld().findAnimatedModelForType(TR1ItemId::AlternativeLara)->animations[1]});
    lara.setCurrentAnimState(loader::file::LaraStateId::MidasDeath);
    lara.setGoalAnimState(loader::file::LaraStateId::MidasDeath);
    lara.setHandStatus(HandStatus::Grabbing);
    getWorld().getPlayer().selectedWeaponType = WeaponType::None;
    lara.setAir(-1_frame);
    lara.m_state.falling = false;
    auto& cameraController = getWorld().getCameraController();
    cameraController.setMode(CameraMode::Cinematic);
    cameraController.m_cinematicFrame = 0_frame;
    cameraController.m_cinematicPos = lara.m_state.location.position;
    cameraController.m_cinematicRot = lara.m_state.rotation;
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
  getWorld().getPlayer().getInventory().put(TR1ItemId::Puzzle1Sprite, &getWorld());
  lara.getSkeleton()->setAnim(
    gsl::not_null{&getWorld().findAnimatedModelForType(TR1ItemId::AlternativeLara)->animations[0]});
  lara.setCurrentAnimState(loader::file::LaraStateId::UseMidas);
  lara.setGoalAnimState(loader::file::LaraStateId::UseMidas);
  lara.setHandStatus(HandStatus::Grabbing);
}
} // namespace engine::objects
