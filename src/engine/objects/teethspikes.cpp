#include "teethspikes.h"

#include "engine/particle.h"
#include "laraobject.h"

void engine::objects::TeethSpikes::collide(CollisionInfo& collisionInfo)
{
  if(getEngine().getObjectManager().getLara().m_state.health >= 0_hp
     && isNear(getEngine().getObjectManager().getLara(), collisionInfo.collisionRadius)
     && testBoneCollision(getEngine().getObjectManager().getLara()))
  {
    int bloodSplats = util::rand15(2);
    if(!getEngine().getObjectManager().getLara().m_state.falling)
    {
      if(getEngine().getObjectManager().getLara().m_state.speed < 30_spd)
      {
        return;
      }
    }
    else
    {
      if(getEngine().getObjectManager().getLara().m_state.fallspeed > 0_spd)
      {
        // immediate death when falling into the spikes
        bloodSplats = 20;
        getEngine().getObjectManager().getLara().m_state.health = -1_hp;
      }
    }
    getEngine().getObjectManager().getLara().m_state.health -= 15_hp;
    while(bloodSplats-- > 0)
    {
      auto fx
        = createBloodSplat(getEngine(),
                           core::RoomBoundPosition{
                             getEngine().getObjectManager().getLara().m_state.position.room,
                             getEngine().getObjectManager().getLara().m_state.position.position
                               + core::TRVec{util::rand15s(128_len), -util::rand15(512_len), util::rand15s(128_len)}},
                           20_spd,
                           util::rand15(+180_deg));
      getEngine().getObjectManager().registerParticle(fx);
    }
    if(getEngine().getObjectManager().getLara().m_state.health <= 0_hp)
    {
      getEngine().getObjectManager().getLara().getSkeleton()->anim
        = &getEngine().getAnimation(loader::file::AnimationId::SPIKED);
      getEngine().getObjectManager().getLara().getSkeleton()->frame_number = 3887_frame;
      getEngine().getObjectManager().getLara().setCurrentAnimState(loader::file::LaraStateId::Death);
      getEngine().getObjectManager().getLara().setGoalAnimState(loader::file::LaraStateId::Death);
      getEngine().getObjectManager().getLara().m_state.falling = false;
      getEngine().getObjectManager().getLara().m_state.position.position.Y = m_state.position.position.Y;
    }
  }
}
