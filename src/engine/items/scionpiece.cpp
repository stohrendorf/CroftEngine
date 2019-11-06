#include "scionpiece.h"

#include "engine/laranode.h"
#include "engine/particle.h"
#include "hid/inputhandler.h"

namespace engine
{
namespace items
{
void ScionPiece::collide(LaraNode& lara, CollisionInfo& /*collisionInfo*/)
{
  static const InteractionLimits limits{core::BoundingBox{{-256_len, 540_len, -350_len}, {256_len, 740_len, -200_len}},
                                        {-10_deg, 0_deg, 0_deg},
                                        {10_deg, 0_deg, 0_deg}};

  m_state.rotation.X = 0_deg;
  m_state.rotation.Y = lara.m_state.rotation.Y;
  m_state.rotation.Z = 0_deg;

  if(!limits.canInteract(m_state, lara.m_state))
    return;

  if(lara.getCurrentAnimState() != LaraStateId::PickUp)
  {
    if(getEngine().getInputHandler().getInputState().action && lara.getHandStatus() == HandStatus::None
       && !lara.m_state.falling && lara.getCurrentAnimState() == LaraStateId::Stop)
    {
      lara.alignForInteraction({0_len, 640_len, -310_len}, m_state);
      lara.m_state.anim = getEngine().findAnimatedModelForType(TR1ItemId::AlternativeLara)->animations;
      lara.setCurrentAnimState(LaraStateId::PickUp);
      lara.setGoalAnimState(LaraStateId::PickUp);
      lara.m_state.frame_number = lara.m_state.anim->firstFrame;
      getEngine().getCameraController().setMode(CameraMode::Cinematic);
      lara.setHandStatus(HandStatus::Grabbing);
      getEngine().getCameraController().m_cinematicFrame = 0;
      getEngine().getCameraController().m_cinematicPos = lara.m_state.position.position;
      getEngine().getCameraController().m_cinematicRot = lara.m_state.rotation;
    }
  }
  else if(lara.m_state.frame_number == lara.m_state.anim->firstFrame + 44_frame)
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
    ModelItemNode::update();
    return;
  }

  if(m_deadTime == 0_frame)
  {
    m_state.triggerState = TriggerState::Invisible;
    m_state.health = -16384_hp;

    const auto sector = loader::file::findRealFloorSector(m_state.position.position, m_state.position.room);
    const auto hi = HeightInfo::fromFloor(sector, m_state.position.position, getEngine().getItemNodes());
    getEngine().getLara().handleCommandSequence(hi.lastCommandSequenceOrDeath, true);
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
} // namespace items
} // namespace engine
