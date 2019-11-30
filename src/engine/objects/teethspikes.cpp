#include "teethspikes.h"

#include "engine/particle.h"
#include "laraobject.h"

void engine::objects::TeethSpikes::collide(CollisionInfo& collisionInfo)
{
  if(getEngine().getLara().m_state.health >= 0_hp && isNear(getEngine().getLara(), collisionInfo.collisionRadius)
     && testBoneCollision(getEngine().getLara()))
  {
    int bloodSplats = util::rand15(2);
    if(!getEngine().getLara().m_state.falling)
    {
      if(getEngine().getLara().m_state.speed < 30_spd)
      {
        return;
      }
    }
    else
    {
      if(getEngine().getLara().m_state.fallspeed > 0_spd)
      {
        // immediate death when falling into the spikes
        bloodSplats = 20;
        getEngine().getLara().m_state.health = -1_hp;
      }
    }
    getEngine().getLara().m_state.health -= 15_hp;
    while(bloodSplats-- > 0)
    {
      auto fx
        = createBloodSplat(getEngine(),
                           core::RoomBoundPosition{
                             getEngine().getLara().m_state.position.room,
                             getEngine().getLara().m_state.position.position
                               + core::TRVec{util::rand15s(128_len), -util::rand15(512_len), util::rand15s(128_len)}},
                           20_spd,
                           util::rand15(+180_deg));
      getEngine().getParticles().emplace_back(fx);
    }
    if(getEngine().getLara().m_state.health <= 0_hp)
    {
      getEngine().getLara().m_state.anim = &getEngine().getAnimation(loader::file::AnimationId::SPIKED);
      getEngine().getLara().m_state.frame_number = 3887_frame;
      getEngine().getLara().setCurrentAnimState(loader::file::LaraStateId::Death);
      getEngine().getLara().setGoalAnimState(loader::file::LaraStateId::Death);
      getEngine().getLara().m_state.falling = false;
      getEngine().getLara().m_state.position.position.Y = m_state.position.position.Y;
    }
  }
}
