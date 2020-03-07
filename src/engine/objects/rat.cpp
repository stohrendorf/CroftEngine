#include "rat.h"

#include "engine/particle.h"
#include "laraobject.h"

namespace engine::objects
{
void Rat::update()
{
  activate();

  if(m_state.type == TR1ItemId::RatInWater)
  {
    if(alive())
    {
      const ai::AiInfo aiInfo{getEngine(), m_state};
      core::Angle headRot = 0_deg;
      if(aiInfo.ahead)
      {
        headRot = aiInfo.angle;
      }
      updateMood(getEngine(), m_state, aiInfo, true);
      const auto turn = rotateTowardsTarget(3_deg);

      if(m_state.current_anim_state == 1_as && aiInfo.ahead)
      {
        if(touched(0x300018ful))
          goal(2_as);
      }
      else if(m_state.current_anim_state == 2_as)
      {
        if(m_state.required_anim_state == 0_as && aiInfo.ahead)
        {
          if(touched(0x300018ful))
          {
            emitParticle({0_len, -11_len, 108_len}, 3, &createBloodSplat);
            hitLara(20_hp);
            require(1_as);
          }
        }
        goal(0_as);
      }

      rotateCreatureHead(headRot);

      const auto waterHeight = getWaterSurfaceHeight();
      if(!waterHeight.has_value())
      {
        m_state.type = TR1ItemId::RatOnLand;
        getSkeleton()->anim = &getEngine().findAnimatedModelForType(TR1ItemId::RatOnLand)->animations[0];
        getSkeleton()->frame_number = getSkeleton()->anim->firstFrame;
        goal(getSkeleton()->anim->state_id);
        m_state.current_anim_state = getSkeleton()->anim->state_id;

        loadObjectInfo(true);
      }
      const auto prevY = std::exchange(m_state.position.position.Y, m_state.floor);
      animateCreature(turn, 0_deg);
      if(prevY != -core::HeightLimit)
      {
        const auto w = *waterHeight;
        if(w < prevY - 32_len)
        {
          m_state.position.position.Y = prevY - 32_len;
        }
        else if(w > prevY + 32_len)
        {
          m_state.position.position.Y = prevY + 32_len;
        }
        else
        {
          m_state.position.position.Y = w;
        }
      }
      applyTransform();
    }
    else
    {
      if(m_state.current_anim_state != 3_as)
      {
        getSkeleton()->anim = &getEngine().findAnimatedModelForType(TR1ItemId::RatInWater)->animations[2];
        getSkeleton()->frame_number = getSkeleton()->anim->firstFrame;
        m_state.current_anim_state = 3_as;
      }
      rotateCreatureHead(0_deg);
      getSkeleton()->patchBone(2, core::TRRotation{0_deg, m_state.creatureInfo->head_rotation, 0_deg}.toMatrix());
      ModelObject::update();
      if(m_state.triggerState == TriggerState::Deactivated)
      {
        m_state.collidable = false;
        m_state.health = -16384_hp;
        m_state.triggerState = TriggerState::Active;
      }
      if(!getWaterSurfaceHeight().has_value())
      {
        m_state.type = TR1ItemId::RatOnLand;
        getSkeleton()->anim = &getEngine().findAnimatedModelForType(TR1ItemId::RatOnLand)->animations[8];
        getSkeleton()->frame_number = getSkeleton()->anim->firstFrame;
        goal(5_as);
        m_state.current_anim_state = m_state.goal_anim_state;
        m_state.position.position.Y = m_state.floor;
        applyTransform();

        loadObjectInfo(true);
      }
    }
  }
  else
  {
    BOOST_ASSERT(m_state.type == TR1ItemId::RatOnLand);
    core::Angle turn = 0_deg;
    core::Angle headRot = 0_deg;
    if(alive())
    {
      const ai::AiInfo aiInfo{getEngine(), m_state};
      if(aiInfo.ahead)
      {
        headRot = aiInfo.angle;
      }
      updateMood(getEngine(), m_state, aiInfo, false);
      turn = rotateTowardsTarget(6_deg);

      switch(m_state.current_anim_state.get())
      {
      case 1:
        if(m_state.required_anim_state != 0_as)
          goal(m_state.required_anim_state);
        else if(aiInfo.bite && aiInfo.distance < util::square(341_len))
          goal(4_as);
        else
          goal(3_as);
        break;
      case 2:
        if(m_state.required_anim_state == 0_as && aiInfo.ahead && touched(0x300018ful))
        {
          emitParticle({0_len, -11_len, 108_len}, 3, &createBloodSplat);
          hitLara(20_hp);
          require(3_as);
        }
        break;
      case 3:
        if(aiInfo.ahead && touched(0x300018ful))
          goal(1_as);
        else if(aiInfo.bite && aiInfo.distance < util::square(1536_len))
          goal(2_as);
        else if(aiInfo.ahead && util::rand15() < 256)
          goal(1_as, 6_as);
        break;
      case 4:
        if(m_state.required_anim_state == 0_as && aiInfo.ahead && touched(0x300018ful))
        {
          emitParticle({0_len, -11_len, 108_len}, 3, &createBloodSplat);
          hitLara(20_hp);
          require(1_as);
        }
        break;
      case 6:
        if(!isBored() || util::rand15() < 256)
          goal(1_as);
        break;
      default: break;
      }
    }
    else if(m_state.current_anim_state != 5_as)
    {
      getSkeleton()->anim = &getEngine().findAnimatedModelForType(TR1ItemId::RatOnLand)->animations[8];
      getSkeleton()->frame_number = getSkeleton()->anim->firstFrame;
      m_state.current_anim_state = 5_as;
    }
    rotateCreatureHead(headRot);
    if(const auto waterHeight = getWaterSurfaceHeight())
    {
      m_state.type = TR1ItemId::RatInWater;
      getSkeleton()->anim = &getEngine().findAnimatedModelForType(TR1ItemId::RatInWater)->animations[0];
      getSkeleton()->frame_number = getSkeleton()->anim->firstFrame;
      goal(getSkeleton()->anim->state_id);
      m_state.current_anim_state = getSkeleton()->anim->state_id;
      m_state.position.position.Y = waterHeight.value();
      applyTransform();

      loadObjectInfo(true);
    }
    getSkeleton()->patchBone(2, core::TRRotation{0_deg, m_state.creatureInfo->head_rotation, 0_deg}.toMatrix());
    animateCreature(turn, 0_deg);
  }
}
} // namespace engine::objects
