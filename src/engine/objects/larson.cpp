#include "larson.h"

#include "engine/world.h"
#include "laraobject.h"

namespace engine::objects
{
void Larson::update()
{
  activateAi();

  core::Angle tiltRot = 0_deg;
  core::Angle creatureTurn = 0_deg;
  core::Angle headRot = 0_deg;
  if(alive())
  {
    const ai::AiInfo aiInfo{getWorld(), m_state};
    if(aiInfo.ahead)
    {
      headRot = aiInfo.angle;
    }

    updateMood(getWorld(), m_state, aiInfo, false);

    creatureTurn = rotateTowardsTarget(m_state.creatureInfo->maximum_turn);
    switch(m_state.current_anim_state.get())
    {
    case 1: // standing holding gun
      if(m_state.required_anim_state != 0_as)
      {
        goal(m_state.required_anim_state);
      }
      else if(isBored())
      {
        if(util::rand15() >= 96)
          goal(2_as); // walking
        else
          goal(6_as); // standing
      }
      else if(isEscaping())
      {
        goal(3_as); // running
      }
      else
      {
        goal(2_as); // walking
      }
      break;
    case 2: // walking
      m_state.creatureInfo->maximum_turn = 3_deg;
      if(isBored() && util::rand15() < 96)
        goal(1_as, 6_as);
      else if(isEscaping())
        goal(1_as, 3_as);
      else if(canShootAtLara(aiInfo))
        goal(1_as, 4_as);
      else if(!aiInfo.ahead || aiInfo.distance > util::square(3 * core::SectorSize))
        goal(1_as, 3_as);
      break;
    case 3: // running
      m_state.creatureInfo->maximum_turn = 6_deg;
      tiltRot = creatureTurn / 2;
      if(isBored() && util::rand15() < 96)
        goal(1_as, 6_as);
      else if(canShootAtLara(aiInfo))
        goal(1_as, 4_as);
      else if(aiInfo.ahead && aiInfo.distance < util::square(3 * core::SectorSize))
        goal(1_as, 2_as);
      break;
    case 4: // aiming
      if(m_state.required_anim_state != 0_as)
        goal(m_state.required_anim_state);
      else if(canShootAtLara(aiInfo))
        goal(7_as);
      else
        goal(1_as);
      break;
    case 6: // standing
      if(!isBored())
        goal(1_as); // standing/holding gun
      else if(util::rand15() < 96)
        goal(1_as, 2_as);
      break;
    case 7: // firing
      if(m_state.required_anim_state == 0_as)
      {
        if(tryShootAtLara(*this, aiInfo.distance, core::TRVec{-60_len, 170_len, 0_len}, 14, headRot))
        {
          hitLara(50_hp);
        }
        require(4_as);
      }
      if(isEscaping())
        require(1_as);
      break;
    default: break;
    }
  }
  else if(m_state.current_anim_state != 5_as) // injured/dying
  {
    getSkeleton()->anim = &getWorld().findAnimatedModelForType(TR1ItemId::Larson)->animations[15];
    getSkeleton()->frame_number = getSkeleton()->anim->firstFrame;
    m_state.current_anim_state = 5_as;
  }
  rotateCreatureTilt(tiltRot);
  rotateCreatureHead(headRot);
  getSkeleton()->patchBone(7, core::TRRotation{0_deg, m_state.creatureInfo->head_rotation, 0_deg}.toMatrix());
  animateCreature(creatureTurn, 0_deg);
}

Larson::Larson(const gsl::not_null<World*>& world,
               const gsl::not_null<const loader::file::Room*>& room,
               const loader::file::Item& item,
               const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
    : AIAgent{world, room, item, animatedModel}
{
}
} // namespace engine::objects
