#include "teethspikes.h"

#include "core/angle.h"
#include "core/magic.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine/collisioninfo.h"
#include "engine/location.h"
#include "engine/objectmanager.h"
#include "engine/particle.h"
#include "engine/skeletalmodelnode.h"
#include "engine/world/world.h"
#include "laraobject.h"
#include "loader/file/animationid.h"
#include "loader/file/larastateid.h"
#include "objectstate.h"
#include "qs/qs.h"
#include "util/helpers.h"

#include <memory>

void engine::objects::TeethSpikes::collide(CollisionInfo& collisionInfo)
{
  if(!getWorld().getObjectManager().getLara().isDead()
     && isNear(getWorld().getObjectManager().getLara(), collisionInfo.collisionRadius)
     && testBoneCollision(getWorld().getObjectManager().getLara()))
  {
    int bloodSplats = util::rand15(2);
    if(!getWorld().getObjectManager().getLara().m_state.falling)
    {
      if(getWorld().getObjectManager().getLara().m_state.speed < 30_spd)
      {
        return;
      }
    }
    else
    {
      if(getWorld().getObjectManager().getLara().m_state.fallspeed > 0_spd)
      {
        // immediate death when falling into the spikes
        bloodSplats = 20;
        getWorld().getObjectManager().getLara().m_state.health = core::DeadHealth;
      }
    }
    getWorld().getObjectManager().getLara().m_state.health -= 15_hp;
    while(bloodSplats-- > 0)
    {
      auto fx = createBloodSplat(
        getWorld(),
        Location{getWorld().getObjectManager().getLara().m_state.location.room,
                 getWorld().getObjectManager().getLara().m_state.location.position
                   + core::TRVec{util::rand15s(128_len), -util::rand15(512_len), util::rand15s(128_len)}},
        20_spd,
        util::rand15(+180_deg));
      getWorld().getObjectManager().registerParticle(fx);
    }
    if(getWorld().getObjectManager().getLara().isDead())
    {
      getWorld().getObjectManager().getLara().getSkeleton()->setAnim(
        &getWorld().getAnimation(loader::file::AnimationId::SPIKED));
      getWorld().getObjectManager().getLara().setCurrentAnimState(loader::file::LaraStateId::Death);
      getWorld().getObjectManager().getLara().setGoalAnimState(loader::file::LaraStateId::Death);
      getWorld().getObjectManager().getLara().m_state.falling = false;
      getWorld().getObjectManager().getLara().m_state.location.position.Y = m_state.location.position.Y;
    }
  }
}
