#include "slammingdoors.h"

#include "engine/particle.h"
#include "laraobject.h"

namespace engine::objects
{
void SlammingDoors::update()
{
  if(!m_state.updateActivationTimeout())
  {
    m_state.goal_anim_state = 0_as;
  }
  else
  {
    m_state.goal_anim_state = 1_as;
    if(m_state.touch_bits.any() && m_state.current_anim_state == 1_as)
    {
      getEngine().getObjectManager().getLara().m_state.health -= 400_hp;
      getEngine().getObjectManager().getLara().m_state.is_hit = true;

      const auto objectSpheres = getSkeleton()->getBoneCollisionSpheres(
        m_state, *getSkeleton()->getInterpolationInfo().getNearestFrame(), nullptr);

      const auto emitBlood = [&objectSpheres, this](const core::TRVec& bitePos, size_t boneId) {
        const auto position
          = core::TRVec{glm::vec3{translate(objectSpheres.at(boneId).m, bitePos.toRenderSystem())[3]}};

        auto blood = createBloodSplat(
          getEngine(), core::RoomBoundPosition{m_state.position.room, position}, m_state.speed, m_state.rotation.Y);
        getEngine().getObjectManager().registerParticle(std::move(blood));
      };

      for(const auto& x : {-23_len, 71_len})
      {
        for(const auto& y : {0_len, 10_len, -10_len})
          emitBlood({x, y, -1718_len}, 0);
      }
    }
  }
  ModelObject::update();
}

void SlammingDoors::collide(CollisionInfo& collisionInfo)
{
  if(m_state.triggerState == TriggerState::Active)
  {
    if(isNear(getEngine().getObjectManager().getLara(), collisionInfo.collisionRadius))
    {
      testBoneCollision(getEngine().getObjectManager().getLara());
    }
  }
  else if(m_state.triggerState != TriggerState::Invisible
          && isNear(getEngine().getObjectManager().getLara(), collisionInfo.collisionRadius)
          && testBoneCollision(getEngine().getObjectManager().getLara()))
  {
    if(collisionInfo.policyFlags.is_set(CollisionInfo::PolicyFlags::EnableBaddiePush))
    {
      enemyPush(collisionInfo, false, true);
    }
  }
}
} // namespace engine::objects
