#include "slammingdoors.h"

#include "engine/particle.h"
#include "engine/world/world.h"
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
      getWorld().getObjectManager().getLara().m_state.health -= 400_hp;
      getWorld().getObjectManager().getLara().m_state.is_hit = true;

      const auto objectSpheres = getSkeleton()->getBoneCollisionSpheres(
        m_state, *getSkeleton()->getInterpolationInfo().getNearestFrame(), nullptr);

      const auto emitBlood = [&objectSpheres, this](const core::TRVec& bitePos, size_t boneId) {
        const auto position
          = core::TRVec{glm::vec3{translate(objectSpheres.at(boneId).m, bitePos.toRenderSystem())[3]}};

        auto blood = createBloodSplat(
          getWorld(), RoomBoundPosition{m_state.position.room, position}, m_state.speed, m_state.rotation.Y);
        getWorld().getObjectManager().registerParticle(std::move(blood));
      };

      for(const auto& x : {-23_len, 71_len})
      {
        for(const auto& y : {0_len, 10_len, -10_len})
          emitBlood({x, y, -1718_len}, x > 0_len ? 1 : 0);
      }
    }
  }
  ModelObject::update();
}

void SlammingDoors::collide(CollisionInfo& collisionInfo)
{
  if(m_state.triggerState == TriggerState::Active)
  {
    if(isNear(getWorld().getObjectManager().getLara(), collisionInfo.collisionRadius))
    {
      testBoneCollision(getWorld().getObjectManager().getLara());
    }
  }
  else if(m_state.triggerState != TriggerState::Invisible
          && isNear(getWorld().getObjectManager().getLara(), collisionInfo.collisionRadius)
          && testBoneCollision(getWorld().getObjectManager().getLara()))
  {
    if(collisionInfo.policies.is_set(CollisionInfo::PolicyFlags::EnableBaddiePush))
    {
      enemyPush(collisionInfo, false, true);
    }
  }
}
} // namespace engine::objects
