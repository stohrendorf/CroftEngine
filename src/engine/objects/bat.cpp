#include "bat.h"

#include "engine/ai/ai.h"
#include "engine/particle.h"
#include "laraobject.h"

namespace engine::objects
{
void Bat::update()
{
  activate();

  static constexpr uint16_t StartingToFly = 1;
  static constexpr uint16_t FlyingStraight = 2;
  static constexpr uint16_t Biting = 3;
  static constexpr uint16_t Circling = 4;
  static constexpr uint16_t Dying = 5;

  core::Angle rotationToMoveTarget = 0_deg;
  if(alive())
  {
    const ai::AiInfo aiInfo{getEngine(), m_state};
    updateMood(getEngine(), m_state, aiInfo, false);

    rotationToMoveTarget = rotateTowardsTarget(20_deg);
    switch(m_state.current_anim_state.get())
    {
    case StartingToFly: goal(FlyingStraight); break;
    case FlyingStraight:
      if(touched())
        goal(Biting);
      break;
    case Biting:
      if(touched())
      {
        emitParticle(core::TRVec{0_len, 16_len, 45_len}, 4, &createBloodSplat);
        hitLara(2_hp);
      }
      else
      {
        goal(FlyingStraight);
        bored();
      }
      break;
    default: break;
    }
  }
  else
  {
    if(m_state.position.position.Y >= m_state.floor)
    {
      goal(Dying);
      settle();
    }
    else
    {
      goal(Circling);
      m_state.speed = 0_spd;
      m_state.falling = true;
    }
  }
  animateCreature(rotationToMoveTarget, 0_deg);
}
} // namespace engine::objects
