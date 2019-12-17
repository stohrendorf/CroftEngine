#include "scionpiece.h"

#include "engine/particle.h"
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
  m_state.rotation.Y = getEngine().getLara().m_state.rotation.Y;
  m_state.rotation.Z = 0_deg;

  if(!limits.canInteract(m_state, getEngine().getLara().m_state))
    return;

  if(getEngine().getLara().getCurrentAnimState() != loader::file::LaraStateId::PickUp)
  {
    if(getEngine().getInputHandler().getInputState().action && getEngine().getLara().getHandStatus() == HandStatus::None
       && !getEngine().getLara().m_state.falling
       && getEngine().getLara().getCurrentAnimState() == loader::file::LaraStateId::Stop)
    {
      getEngine().getLara().alignForInteraction({0_len, 640_len, -310_len}, m_state);
      getEngine().getLara().m_state.anim = getEngine().findAnimatedModelForType(TR1ItemId::AlternativeLara)->animations;
      getEngine().getLara().setCurrentAnimState(loader::file::LaraStateId::PickUp);
      getEngine().getLara().setGoalAnimState(loader::file::LaraStateId::PickUp);
      getEngine().getLara().m_state.frame_number = getEngine().getLara().m_state.anim->firstFrame;
      getEngine().getCameraController().setMode(CameraMode::Cinematic);
      getEngine().getLara().setHandStatus(HandStatus::Grabbing);
      getEngine().getCameraController().m_cinematicFrame = 0;
      getEngine().getCameraController().m_cinematicPos = getEngine().getLara().m_state.position.position;
      getEngine().getCameraController().m_cinematicRot = getEngine().getLara().m_state.rotation;
    }
  }
  else if(getEngine().getLara().m_state.frame_number == getEngine().getLara().m_state.anim->firstFrame + 44_frame)
  {
    m_state.triggerState = TriggerState::Invisible;
    getEngine().getInventory().put(m_state.type);
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
    const auto hi = HeightInfo::fromFloor(sector, m_state.position.position, getEngine().getObjects());
    getEngine().handleCommandSequence(hi.lastCommandSequenceOrDeath, true);
    getNode()->removeAllChildren();
  }

  if(m_deadTime % 10_frame == 0_frame)
  {
    const auto pos = m_state.position.position
                     + core::TRVec{util::rand15s(512_len), util::rand15s(64_len) - 500_len, util::rand15s(512_len)};
    const auto particle = std::make_shared<ExplosionParticle>(
      core::RoomBoundPosition{m_state.position.room, pos}, getEngine(), 0_spd, core::TRRotation{0_deg, 0_deg, 0_deg});
    setParent(particle, m_state.position.room->node);
    getEngine().getParticles().emplace_back(particle);
    getEngine().getAudioEngine().playSound(TR1SoundId::Explosion2, particle.get());

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
  m_state.rotation = {0_deg, getEngine().getLara().m_state.rotation.Y, 0_deg};

  static const InteractionLimits limits{core::BoundingBox{{-256_len, -206_len, -862_len}, {256_len, 306_len, -200_len}},
                                        {-10_deg, 0_deg, 0_deg},
                                        {+10_deg, 0_deg, 0_deg}};

  if(!getEngine().getInputHandler().getInputState().action || getEngine().getLara().getHandStatus() != HandStatus::None
     || getEngine().getLara().m_state.falling
     || getEngine().getLara().getCurrentAnimState() != loader::file::LaraStateId::Stop
     || !limits.canInteract(m_state, getEngine().getLara().m_state))
    return;

  static const core::TRVec alignSpeed{0_len, 280_len, -407_len};

  getEngine().getLara().alignTransform(alignSpeed, *this);
  getEngine().getLara().m_state.anim = getEngine().findAnimatedModelForType(TR1ItemId::AlternativeLara)->animations;
  getEngine().getLara().m_state.frame_number = getEngine().getLara().m_state.anim->firstFrame;
  getEngine().getLara().setCurrentAnimState(loader::file::LaraStateId::PickUp);
  getEngine().getLara().setGoalAnimState(loader::file::LaraStateId::PickUp);
  getEngine().getLara().setHandStatus(HandStatus::Grabbing);
  getEngine().getCameraController().m_cinematicFrame = 0;
  getEngine().getCameraController().setMode(CameraMode::Cinematic);
  getEngine().getCameraController().m_cinematicPos = getEngine().getLara().m_state.position.position;
  getEngine().getCameraController().m_cinematicRot
    = getEngine().getLara().m_state.rotation - core::TRRotation{0_deg, 90_deg, 0_deg};
}

void ScionHolder::collide(CollisionInfo& info)
{
  if(!isNear(getEngine().getLara(), info.collisionRadius))
    return;

  if(!testBoneCollision(getEngine().getLara()))
    return;

  if(!info.policyFlags.is_set(CollisionInfo::PolicyFlags::EnableBaddiePush))
    return;

  enemyPush(info, false, true);
}
} // namespace engine::objects
