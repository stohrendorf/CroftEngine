#include "scionpiece.h"

#include "engine/audioengine.h"
#include "engine/particle.h"
#include "engine/presenter.h"
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
  m_state.rotation.Y = getEngine().getObjectManager().getLara().m_state.rotation.Y;
  m_state.rotation.Z = 0_deg;

  if(!limits.canInteract(m_state, getEngine().getObjectManager().getLara().m_state))
    return;

  if(getEngine().getObjectManager().getLara().getCurrentAnimState() != loader::file::LaraStateId::PickUp)
  {
    if(getEngine().getPresenter().getInputHandler().getInputState().action
       && getEngine().getObjectManager().getLara().getHandStatus() == HandStatus::None
       && !getEngine().getObjectManager().getLara().m_state.falling
       && getEngine().getObjectManager().getLara().getCurrentAnimState() == loader::file::LaraStateId::Stop)
    {
      getEngine().getObjectManager().getLara().alignForInteraction({0_len, 640_len, -310_len}, m_state);
      getEngine().getObjectManager().getLara().getSkeleton()->anim
        = getEngine().findAnimatedModelForType(TR1ItemId::AlternativeLara)->animations;
      getEngine().getObjectManager().getLara().getSkeleton()->frame_number
        = getEngine().getObjectManager().getLara().getSkeleton()->anim->firstFrame;
      getEngine().getObjectManager().getLara().setCurrentAnimState(loader::file::LaraStateId::PickUp);
      getEngine().getObjectManager().getLara().setGoalAnimState(loader::file::LaraStateId::PickUp);
      getEngine().getCameraController().setMode(CameraMode::Cinematic);
      getEngine().getObjectManager().getLara().setHandStatus(HandStatus::Grabbing);
      getEngine().getCameraController().m_cinematicFrame = 0;
      getEngine().getCameraController().m_cinematicPos
        = getEngine().getObjectManager().getLara().m_state.position.position;
      getEngine().getCameraController().m_cinematicRot = getEngine().getObjectManager().getLara().m_state.rotation;
    }
  }
  else if(getEngine().getObjectManager().getLara().getSkeleton()->frame_number
          == getEngine().getObjectManager().getLara().getSkeleton()->anim->firstFrame + 44_frame)
  {
    m_state.triggerState = TriggerState::Invisible;
    getEngine().getInventory().put(getEngine().getObjectManager().getLara(), m_state.type);
    setParent(getNode(), nullptr);
    m_state.collidable = false;
  }
}

void ScionPiece3::update()
{
  if(m_state.health > 0_hp)
  {
    m_deadTime = 0_frame;
    ModelObject::update();
    return;
  }

  if(m_deadTime == 0_frame)
  {
    m_state.triggerState = TriggerState::Invisible;
    m_state.health = -16384_hp;

    const auto sector = findRealFloorSector(m_state.position.position, m_state.position.room);
    const auto hi
      = HeightInfo::fromFloor(sector, m_state.position.position, getEngine().getObjectManager().getObjects());
    getEngine().handleCommandSequence(hi.lastCommandSequenceOrDeath, true);
    getSkeleton()->removeAllChildren();
    getSkeleton()->clearParts();
  }

  if(m_deadTime % 10_frame == 0_frame)
  {
    const auto pos = m_state.position.position
                     + core::TRVec{util::rand15s(512_len), util::rand15s(64_len) - 500_len, util::rand15s(512_len)};
    const auto particle = std::make_shared<ExplosionParticle>(
      core::RoomBoundPosition{m_state.position.room, pos}, getEngine(), 0_spd, core::TRRotation{0_deg, 0_deg, 0_deg});
    setParent(particle, m_state.position.room->node);
    getEngine().getObjectManager().registerParticle(particle);
    getEngine().getPresenter().getAudioEngine().playSoundEffect(TR1SoundEffect::Explosion2, particle.get());

    getEngine().getCameraController().setBounce(-200_len);
  }

  m_deadTime += 1_frame;
  if(m_deadTime == 90_frame)
  {
    deactivate();
  }
}

void ScionPiece4::collide(CollisionInfo& /*info*/)
{
  m_state.rotation = {0_deg, getEngine().getObjectManager().getLara().m_state.rotation.Y, 0_deg};

  static const InteractionLimits limits{core::BoundingBox{{-256_len, -206_len, -862_len}, {256_len, 306_len, -200_len}},
                                        {-10_deg, 0_deg, 0_deg},
                                        {+10_deg, 0_deg, 0_deg}};

  if(!getEngine().getPresenter().getInputHandler().getInputState().action
     || getEngine().getObjectManager().getLara().getHandStatus() != HandStatus::None
     || getEngine().getObjectManager().getLara().m_state.falling
     || getEngine().getObjectManager().getLara().getCurrentAnimState() != loader::file::LaraStateId::Stop
     || !limits.canInteract(m_state, getEngine().getObjectManager().getLara().m_state))
    return;

  static const core::TRVec alignSpeed{0_len, 280_len, -407_len};

  getEngine().getObjectManager().getLara().alignTransform(alignSpeed, *this);
  getEngine().getObjectManager().getLara().getSkeleton()->anim
    = getEngine().findAnimatedModelForType(TR1ItemId::AlternativeLara)->animations;
  getEngine().getObjectManager().getLara().getSkeleton()->frame_number
    = getEngine().getObjectManager().getLara().getSkeleton()->anim->firstFrame;
  getEngine().getObjectManager().getLara().setCurrentAnimState(loader::file::LaraStateId::PickUp);
  getEngine().getObjectManager().getLara().setGoalAnimState(loader::file::LaraStateId::PickUp);
  getEngine().getObjectManager().getLara().setHandStatus(HandStatus::Grabbing);
  getEngine().getCameraController().m_cinematicFrame = 0;
  getEngine().getCameraController().setMode(CameraMode::Cinematic);
  getEngine().getCameraController().m_cinematicPos = getEngine().getObjectManager().getLara().m_state.position.position;
  getEngine().getCameraController().m_cinematicRot
    = getEngine().getObjectManager().getLara().m_state.rotation - core::TRRotation{0_deg, 90_deg, 0_deg};
}

void ScionHolder::collide(CollisionInfo& info)
{
  if(!isNear(getEngine().getObjectManager().getLara(), info.collisionRadius))
    return;

  if(!testBoneCollision(getEngine().getObjectManager().getLara()))
    return;

  if(!info.policyFlags.is_set(CollisionInfo::PolicyFlags::EnableBaddiePush))
    return;

  enemyPush(info, false, true);
}
} // namespace engine::objects
