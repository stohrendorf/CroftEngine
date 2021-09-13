#include "scionpiece.h"

#include "engine/audioengine.h"
#include "engine/engine.h"
#include "engine/particle.h"
#include "engine/player.h"
#include "engine/presenter.h"
#include "engine/skeletalmodelnode.h"
#include "engine/world/skeletalmodeltype.h"
#include "engine/world/world.h"
#include "hid/inputhandler.h"
#include "laraobject.h"

namespace engine::objects
{
void ScionPiece::collide(CollisionInfo& /*collisionInfo*/)
{
  static const InteractionLimits limits{core::BoundingBox{{-256_len, 540_len, -350_len}, {256_len, 740_len, -200_len}},
                                        {-10_deg, 0_deg, 0_deg},
                                        {10_deg, 0_deg, 0_deg}};

  auto& lara = getWorld().getObjectManager().getLara();
  m_state.rotation.X = 0_deg;
  m_state.rotation.Y = lara.m_state.rotation.Y;
  m_state.rotation.Z = 0_deg;

  if(!limits.canInteract(m_state, lara.m_state))
    return;

  if(lara.getCurrentAnimState() != loader::file::LaraStateId::PickUp)
  {
    if(getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Action)
       && lara.getHandStatus() == HandStatus::None && !lara.m_state.falling
       && lara.getCurrentAnimState() == loader::file::LaraStateId::Stop)
    {
      lara.alignForInteraction({0_len, 640_len, -310_len}, m_state);
      lara.getSkeleton()->setAnim(getWorld().findAnimatedModelForType(TR1ItemId::AlternativeLara)->animations);
      lara.setCurrentAnimState(loader::file::LaraStateId::PickUp);
      lara.setGoalAnimState(loader::file::LaraStateId::PickUp);
      lara.setHandStatus(HandStatus::Grabbing);
      auto& cameraController = getWorld().getCameraController();
      cameraController.setMode(CameraMode::Cinematic);
      cameraController.m_cinematicFrame = 0;
      cameraController.m_cinematicPos = lara.m_state.location.position;
      cameraController.m_cinematicRot = lara.m_state.rotation;
    }
  }
  else if(lara.getSkeleton()->getLocalFrame() == 44_frame)
  {
    m_state.triggerState = TriggerState::Invisible;
    getWorld().getPlayer().getInventory().put(m_state.type);
    ++getWorld().getPlayer().pickups;
    getWorld().addPickupWidget(getSprite());
    setParent(getNode(), nullptr);
    m_state.collidable = false;
  }
}

void ScionPiece3::update()
{
  if(!m_state.isDead())
  {
    m_deadTime = 0_frame;
    ModelObject::update();
    return;
  }

  if(m_deadTime == 0_frame)
  {
    m_state.triggerState = TriggerState::Invisible;
    m_state.health = core::DeadHealth;

    const auto sector = m_state.location.moved({}).updateRoom();
    const auto hi
      = HeightInfo::fromFloor(sector, m_state.location.position, getWorld().getObjectManager().getObjects());
    getWorld().handleCommandSequence(hi.lastCommandSequenceOrDeath, true);
  }

  if(m_deadTime % 10_frame == 0_frame)
  {
    const auto pos = m_state.location.position
                     + core::TRVec{util::rand15s(512_len), util::rand15s(64_len) - 500_len, util::rand15s(512_len)};
    const auto particle = std::make_shared<ExplosionParticle>(
      Location{m_state.location.room, pos}, getWorld(), 0_spd, core::TRRotation{});
    setParent(particle, m_state.location.room->node);
    getWorld().getObjectManager().registerParticle(particle);
    getWorld().getAudioEngine().playSoundEffect(TR1SoundEffect::Explosion2, particle.get());

    getWorld().getCameraController().setBounce(-200_len);
  }

  m_deadTime += 1_frame;
  if(m_deadTime == 90_frame)
  {
    deactivate();
  }
}

void ScionPiece4::collide(CollisionInfo& /*info*/)
{
  auto& lara = getWorld().getObjectManager().getLara();
  m_state.rotation = {0_deg, lara.m_state.rotation.Y, 0_deg};

  static const InteractionLimits limits{core::BoundingBox{{-256_len, -206_len, -862_len}, {256_len, 306_len, -200_len}},
                                        {-10_deg, 0_deg, 0_deg},
                                        {+10_deg, 0_deg, 0_deg}};

  if(!getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Action)
     || lara.getHandStatus() != HandStatus::None || lara.m_state.falling
     || lara.getCurrentAnimState() != loader::file::LaraStateId::Stop || !limits.canInteract(m_state, lara.m_state))
    return;

  static const core::TRVec alignSpeed{0_len, 280_len, -407_len};

  lara.alignTransform(alignSpeed, *this);
  lara.getSkeleton()->setAnim(getWorld().findAnimatedModelForType(TR1ItemId::AlternativeLara)->animations);
  lara.setCurrentAnimState(loader::file::LaraStateId::PickUp);
  lara.setGoalAnimState(loader::file::LaraStateId::PickUp);
  lara.setHandStatus(HandStatus::Grabbing);
  auto& cameraController = getWorld().getCameraController();
  cameraController.m_cinematicFrame = 0;
  cameraController.setMode(CameraMode::Cinematic);
  cameraController.m_cinematicPos = lara.m_state.location.position;
  cameraController.m_cinematicRot = lara.m_state.rotation - core::TRRotation{0_deg, 90_deg, 0_deg};
}

void ScionHolder::collide(CollisionInfo& info)
{
  if(!isNear(getWorld().getObjectManager().getLara(), info.collisionRadius))
    return;

  if(!testBoneCollision(getWorld().getObjectManager().getLara()))
    return;

  if(!info.policies.is_set(CollisionInfo::PolicyFlags::EnableBaddiePush))
    return;

  enemyPush(info, false, true);
}
} // namespace engine::objects
