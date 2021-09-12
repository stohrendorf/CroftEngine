#include "slammingdoors.h"

#include "engine/particle.h"
#include "engine/skeletalmodelnode.h"
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

      const auto objectSpheres = getSkeleton()->getBoneCollisionSpheres();

      const auto emitBlood = [&objectSpheres, this](const core::TRVec& bitePos, size_t boneId)
      {
        const auto position = core::TRVec{objectSpheres.at(boneId).relative(bitePos.toRenderSystem())};
        auto blood
          = createBloodSplat(getWorld(), Location{m_state.location.room, position}, m_state.speed, m_state.rotation.Y);
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
  trapCollideWithLara(collisionInfo);
}
} // namespace engine::objects
