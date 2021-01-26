#include "scionpiece.h"

#include "engine/audioengine.h"
#include "engine/particle.h"
#include "engine/presenter.h"
#include "engine/world.h"
#include "hid/inputhandler.h"
#include "laraobject.h"

namespace engine::objects
{
void ScionPiece::collide(CollisionInfo& /*collisionInfo*/)
{
  static const InteractionLimits limits{core::BoundingBox{{-256_len, 540_len, -350_len}, {256_len, 740_len, -200_len}},
                                        {-10_deg, 0_deg, 0_deg},
                                        {10_deg, 0_deg, 0_deg}};

  m_state.rotation.X = 0_deg;
  m_state.rotation.Y = getWorld().getObjectManager().getLara().m_state.rotation.Y;
  m_state.rotation.Z = 0_deg;

  if(!limits.canInteract(m_state, getWorld().getObjectManager().getLara().m_state))
    return;

  if(getWorld().getObjectManager().getLara().getCurrentAnimState() != loader::file::LaraStateId::PickUp)
  {
    if(getWorld().getPresenter().getInputHandler().getInputState().action
       && getWorld().getObjectManager().getLara().getHandStatus() == HandStatus::None
       && !getWorld().getObjectManager().getLara().m_state.falling
       && getWorld().getObjectManager().getLara().getCurrentAnimState() == loader::file::LaraStateId::Stop)
    {
      getWorld().getObjectManager().getLara().alignForInteraction({0_len, 640_len, -310_len}, m_state);
      getWorld().getObjectManager().getLara().getSkeleton()->anim
        = getWorld().findAnimatedModelForType(TR1ItemId::AlternativeLara)->animations;
      getWorld().getObjectManager().getLara().getSkeleton()->frame_number
        = getWorld().getObjectManager().getLara().getSkeleton()->anim->firstFrame;
      getWorld().getObjectManager().getLara().setCurrentAnimState(loader::file::LaraStateId::PickUp);
      getWorld().getObjectManager().getLara().setGoalAnimState(loader::file::LaraStateId::PickUp);
      getWorld().getCameraController().setMode(CameraMode::Cinematic);
      getWorld().getObjectManager().getLara().setHandStatus(HandStatus::Grabbing);
      getWorld().getCameraController().m_cinematicFrame = 0;
      getWorld().getCameraController().m_cinematicPos
        = getWorld().getObjectManager().getLara().m_state.position.position;
      getWorld().getCameraController().m_cinematicRot = getWorld().getObjectManager().getLara().m_state.rotation;
    }
  }
  else if(getWorld().getObjectManager().getLara().getSkeleton()->frame_number
          == getWorld().getObjectManager().getLara().getSkeleton()->anim->firstFrame + 44_frame)
  {
    m_state.triggerState = TriggerState::Invisible;
    getWorld().getInventory().put(getWorld().getObjectManager().getLara(), m_state.type);
    getWorld().addPickupWidget(getCroppedImage());
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

    const auto sector = findRealFloorSector(m_state.position.position, m_state.position.room);
    const auto hi
      = HeightInfo::fromFloor(sector, m_state.position.position, getWorld().getObjectManager().getObjects());
    getWorld().handleCommandSequence(hi.lastCommandSequenceOrDeath, true);
    getSkeleton()->removeAllChildren();
    getSkeleton()->clearParts();
  }

  if(m_deadTime % 10_frame == 0_frame)
  {
    const auto pos = m_state.position.position
                     + core::TRVec{util::rand15s(512_len), util::rand15s(64_len) - 500_len, util::rand15s(512_len)};
    const auto particle = std::make_shared<ExplosionParticle>(
      core::RoomBoundPosition{m_state.position.room, pos}, getWorld(), 0_spd, core::TRRotation{0_deg, 0_deg, 0_deg});
    setParent(particle, m_state.position.room->node);
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
  m_state.rotation = {0_deg, getWorld().getObjectManager().getLara().m_state.rotation.Y, 0_deg};

  static const InteractionLimits limits{core::BoundingBox{{-256_len, -206_len, -862_len}, {256_len, 306_len, -200_len}},
                                        {-10_deg, 0_deg, 0_deg},
                                        {+10_deg, 0_deg, 0_deg}};

  if(!getWorld().getPresenter().getInputHandler().getInputState().action
     || getWorld().getObjectManager().getLara().getHandStatus() != HandStatus::None
     || getWorld().getObjectManager().getLara().m_state.falling
     || getWorld().getObjectManager().getLara().getCurrentAnimState() != loader::file::LaraStateId::Stop
     || !limits.canInteract(m_state, getWorld().getObjectManager().getLara().m_state))
    return;

  static const core::TRVec alignSpeed{0_len, 280_len, -407_len};

  getWorld().getObjectManager().getLara().alignTransform(alignSpeed, *this);
  getWorld().getObjectManager().getLara().getSkeleton()->anim
    = getWorld().findAnimatedModelForType(TR1ItemId::AlternativeLara)->animations;
  getWorld().getObjectManager().getLara().getSkeleton()->frame_number
    = getWorld().getObjectManager().getLara().getSkeleton()->anim->firstFrame;
  getWorld().getObjectManager().getLara().setCurrentAnimState(loader::file::LaraStateId::PickUp);
  getWorld().getObjectManager().getLara().setGoalAnimState(loader::file::LaraStateId::PickUp);
  getWorld().getObjectManager().getLara().setHandStatus(HandStatus::Grabbing);
  getWorld().getCameraController().m_cinematicFrame = 0;
  getWorld().getCameraController().setMode(CameraMode::Cinematic);
  getWorld().getCameraController().m_cinematicPos = getWorld().getObjectManager().getLara().m_state.position.position;
  getWorld().getCameraController().m_cinematicRot
    = getWorld().getObjectManager().getLara().m_state.rotation - core::TRRotation{0_deg, 90_deg, 0_deg};
}

void ScionHolder::collide(CollisionInfo& info)
{
  if(!isNear(getWorld().getObjectManager().getLara(), info.collisionRadius))
    return;

  if(!testBoneCollision(getWorld().getObjectManager().getLara()))
    return;

  if(!info.policyFlags.is_set(CollisionInfo::PolicyFlags::EnableBaddiePush))
    return;

  enemyPush(info, false, true);
}
} // namespace engine::objects
