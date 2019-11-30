#include "pierre.h"

#include "engine/cameracontroller.h"
#include "laraobject.h"
#include "pickupobject.h"

namespace engine::objects
{
void Pierre::update()
{
  if(getEngine().m_pierre == nullptr)
  {
    getEngine().m_pierre = this;
  }
  else if(this != getEngine().m_pierre)
  {
    if(m_state.activationState.isOneshot())
    {
      if(getEngine().m_pierre != nullptr)
        getEngine().m_pierre->kill();

      getEngine().m_pierre = nullptr;
    }
    else
    {
      kill();
    }
  }

  if(m_state.triggerState == TriggerState::Invisible)
  {
    m_state.triggerState = TriggerState::Active;
  }

  m_state.initCreatureInfo(getEngine());

  core::Angle tiltRot = 0_deg;
  core::Angle creatureTurn = 0_deg;
  core::Angle headRot = 0_deg;
  if(m_state.health <= 40_hp && !m_state.activationState.isOneshot())
  {
    m_state.health = 40_hp;
    ++m_state.creatureInfo->flags;
  }
  if(m_state.health > 0_hp)
  {
    ai::AiInfo aiInfo{getEngine(), m_state};
    if(aiInfo.ahead)
    {
      headRot = aiInfo.angle;
    }
    if(m_state.creatureInfo->flags != 0)
    {
      aiInfo.enemy_zone = -1;
      aiInfo.enemy_unreachable = true;
      m_state.is_hit = true;
    }

    updateMood(getEngine(), m_state, aiInfo, false);

    creatureTurn = rotateTowardsTarget(m_state.creatureInfo->maximum_turn);
    switch(m_state.current_anim_state.get())
    {
    case 1:
      if(m_state.required_anim_state != 0_as)
      {
        m_state.goal_anim_state = m_state.required_anim_state;
      }
      else if(m_state.creatureInfo->mood == ai::Mood::Bored)
      {
        if(util::rand15() >= 96)
        {
          m_state.goal_anim_state = 2_as;
        }
        else
        {
          m_state.goal_anim_state = 6_as;
        }
      }
      else if(m_state.creatureInfo->mood == ai::Mood::Escape)
      {
        m_state.goal_anim_state = 3_as;
      }
      else
      {
        m_state.goal_anim_state = 2_as;
      }
      break;
    case 2:
      m_state.creatureInfo->maximum_turn = 3_deg;
      if(m_state.creatureInfo->mood == ai::Mood::Bored && util::rand15() < 96)
      {
        m_state.required_anim_state = 6_as;
        m_state.goal_anim_state = 1_as;
      }
      else if(m_state.creatureInfo->mood == ai::Mood::Escape)
      {
        m_state.required_anim_state = 3_as;
        m_state.goal_anim_state = 1_as;
      }
      else if(canShootAtLara(aiInfo))
      {
        m_state.required_anim_state = 4_as;
        m_state.goal_anim_state = 1_as;
      }
      else if(!aiInfo.ahead || aiInfo.distance > util::square(3 * core::SectorSize))
      {
        m_state.required_anim_state = 3_as;
        m_state.goal_anim_state = 1_as;
      }
      break;
    case 3:
      m_state.creatureInfo->maximum_turn = 6_deg;
      tiltRot = creatureTurn / 2;
      if(m_state.creatureInfo->mood != ai::Mood::Bored || util::rand15() >= 96)
      {
        if(canShootAtLara(aiInfo))
        {
          m_state.required_anim_state = 4_as;
          m_state.goal_anim_state = 1_as;
        }
        else if(aiInfo.ahead && aiInfo.distance < util::square(3 * core::SectorSize))
        {
          m_state.required_anim_state = 2_as;
          m_state.goal_anim_state = 1_as;
        }
      }
      else
      {
        m_state.required_anim_state = 6_as;
        m_state.goal_anim_state = 1_as;
      }
      break;
    case 4:
      if(m_state.required_anim_state != 0_as)
      {
        m_state.goal_anim_state = m_state.required_anim_state;
      }
      else if(canShootAtLara(aiInfo))
      {
        m_state.goal_anim_state = 7_as;
      }
      else
      {
        m_state.goal_anim_state = 1_as;
      }
      break;
    case 6:
      if(m_state.creatureInfo->mood != ai::Mood::Bored)
      {
        m_state.goal_anim_state = 1_as;
      }
      else if(util::rand15() < 96)
      {
        m_state.required_anim_state = 2_as;
        m_state.goal_anim_state = 1_as;
      }
      break;
    case 7:
      if(m_state.required_anim_state == 0_as)
      {
        if(tryShootAtLara(*this, aiInfo.distance, {60_len, 200_len, 0_len}, 11, headRot))
        {
          getEngine().getLara().m_state.health -= 25_hp;
          getEngine().getLara().m_state.is_hit = true;
        }
        if(tryShootAtLara(*this, aiInfo.distance, {-57_len, 200_len, 0_len}, 14, headRot))
        {
          getEngine().getLara().m_state.health -= 25_hp;
          getEngine().getLara().m_state.is_hit = true;
        }
        m_state.required_anim_state = 4_as;
      }
      if(m_state.creatureInfo->mood == ai::Mood::Escape && util::rand15() > 8192)
      {
        m_state.required_anim_state = 1_as;
      }
      break;
    default: break;
    }
  }
  else if(m_state.current_anim_state != 5_as) // injured/dying
  {
    m_state.anim = &getEngine().findAnimatedModelForType(TR1ItemId::Pierre)->animations[12];
    m_state.frame_number = m_state.anim->firstFrame;
    m_state.current_anim_state = 5_as;
    getEngine().createPickup(TR1ItemId::MagnumsSprite, m_state.position.room, m_state.position.position);
    getEngine().createPickup(TR1ItemId::ScionPiece2, m_state.position.room, m_state.position.position);
    getEngine().createPickup(TR1ItemId::Key1Sprite, m_state.position.room, m_state.position.position);
  }
  rotateCreatureTilt(tiltRot);
  rotateCreatureHead(headRot);
  animateCreature(creatureTurn, 0_deg);
  getSkeleton()->patchBone(7, core::TRRotation{0_deg, m_state.creatureInfo->head_rotation, 0_deg}.toMatrix());
  if(m_state.creatureInfo->flags != 0)
  {
    auto camPos = m_state.position;
    camPos.position.Y -= core::SectorSize;
    const auto target = getEngine().getCameraController().getTRPosition();
    if(CameraController::clampPosition(target, camPos, getEngine()))
    {
      m_state.creatureInfo->flags = 1;
    }
    else if(m_state.creatureInfo->flags > 10)
    {
      m_state.health = -16384_hp;
      m_state.creatureInfo = nullptr;
      kill();
      getEngine().m_pierre = nullptr;
    }
  }
  if(getWaterSurfaceHeight().has_value())
  {
    m_state.health = -16384_hp;
    m_state.creatureInfo = nullptr;
    kill();
    getEngine().m_pierre = nullptr;
  }
}

Pierre::Pierre(const gsl::not_null<Engine*>& engine,
               const gsl::not_null<const loader::file::Room*>& room,
               const loader::file::Item& item,
               const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
    : AIAgent{engine, room, item, animatedModel}
{
}
} // namespace engine
