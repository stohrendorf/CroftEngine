#include "bear.h"

#include "aiagent.h"
#include "core/angle.h"
#include "core/id.h"
#include "core/magic.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine/ai/ai.h"
#include "engine/objectmanager.h"
#include "engine/particle.h"
#include "engine/skeletalmodelnode.h"
#include "engine/world/world.h"
#include "laraobject.h"
#include "objectstate.h"
#include "qs/qs.h"
#include "serialization/serialization.h"
#include "util/helpers.h"

#include <exception>
#include <memory>

namespace engine::objects
{
void Bear::update()
{
  activateAi();

  core::Angle turn;

  static constexpr auto Walking = 0_as;
  static constexpr auto GettingDown = 1_as;
  static constexpr auto WalkingTall = 2_as;
  static constexpr auto Running = 3_as;
  static constexpr auto RoaringStanding = 4_as;
  static constexpr auto Growling = 5_as;
  static constexpr auto RunningAttack = 6_as;
  static constexpr auto Standing = 7_as;
  static constexpr auto Biting = 8_as;
  static constexpr auto Dying = 9_as;

  if(alive())
  {
    const ai::EnemyLocation enemyLocation{*this};
    updateMood(*this, enemyLocation, true);

    turn = rotateTowardsTarget(getCreatureInfo()->maxTurnSpeed);
    if(m_state.is_hit)
      m_hurt = true;

    switch(m_state.current_anim_state.get())
    {
    case Walking.get():
      getCreatureInfo()->maxTurnSpeed = 2_deg / 1_frame;
      if(getWorld().getObjectManager().getLara().isDead() && touched(0x2406cUL) && enemyLocation.laraInView)
      {
        goal(GettingDown);
      }
      else if(!isBored())
      {
        goal(GettingDown);
        if(isEscaping())
          require(0_as);
      }
      else if(util::rand15() < 80)
      {
        goal(GettingDown, Growling);
      }
      break;
    case GettingDown.get():
      if(getWorld().getObjectManager().getLara().isDead())
      {
        if(enemyLocation.canAttackLara && enemyLocation.distance < util::square(768_len))
          goal(Biting);
        else
          goal(Walking);
      }
      else
      {
        if(m_state.required_anim_state != 0_as)
          goal(m_state.required_anim_state);
        else if(!isBored())
          goal(Running);
        else
          goal(Walking);
      }
      break;
    case WalkingTall.get():
      if(m_hurt)
        goal(RoaringStanding, 0_as); // NOLINT(bugprone-branch-clone)
      else if(enemyLocation.laraInView && touched(0x2406cUL))
        goal(RoaringStanding);
      else if(isEscaping())
        goal(RoaringStanding, 0_as);
      else if(isBored() || util::rand15() < 80)
        goal(RoaringStanding, Growling);
      else if(enemyLocation.distance > util::square(2_sectors) || util::rand15() < 1536)
        goal(RoaringStanding, GettingDown);
      break;
    case Running.get():
      getCreatureInfo()->maxTurnSpeed = 5_deg / 1_frame;
      if(touched(0x2406cUL))
      {
        hitLara(3_hp);
      }
      if(isBored() || getWorld().getObjectManager().getLara().isDead())
      {
        goal(GettingDown);
      }
      else if(enemyLocation.laraInView && m_state.required_anim_state == 0_as)
      {
        if(!m_hurt && enemyLocation.distance < util::square(2048_len) && util::rand15() < 768)
          goal(GettingDown, RoaringStanding);
        else if(enemyLocation.distance < util::square(1_sectors))
          goal(RunningAttack);
      }
      break;
    case RoaringStanding.get():
      if(m_hurt)
        goal(GettingDown, 0_as);
      else if(m_state.required_anim_state != 0_as)
        goal(m_state.required_anim_state);
      else if(isBored() || isEscaping())
        goal(GettingDown);
      else if(enemyLocation.canAttackLara && enemyLocation.distance < util::square(600_len))
        goal(Standing);
      else
        goal(WalkingTall);
      break;
    case RunningAttack.get():
      if(m_state.required_anim_state == 0_as && touched(0x2406cUL))
      {
        emitParticle(core::TRVec{0_len, 96_len, 335_len}, 14, &createBloodSplat);
        hitLara(200_hp);
        require(GettingDown);
      }
      break;
    case Standing.get():
      if(m_state.required_anim_state == 0_as && touched(0x2406cUL))
      {
        hitLara(400_hp);
        require(RoaringStanding);
      }
      break;
    default:
      break;
    }
    rotateCreatureHead(enemyLocation.visualAngleToLara);
  }
  else
  {
    turn = rotateTowardsTarget(1_deg / 1_frame);
    switch(m_state.current_anim_state.get())
    {
    case Walking.get():
    case Running.get():
      goal(GettingDown);
      break;
    case GettingDown.get():
      m_hurt = false;
      goal(Dying);
      break;
    case WalkingTall.get():
      goal(RoaringStanding);
      break;
    case RoaringStanding.get():
      m_hurt = true;
      goal(Dying);
      break;
    case Dying.get():
      if(m_hurt && touched(0x2406cUL))
      {
        hitLara(200_hp);
        m_hurt = false;
      }
      break;
    default:
      break;
    }
    rotateCreatureHead(0_deg);
  }
  getSkeleton()->patchBone(14, core::TRRotation{0_deg, getCreatureInfo()->headRotation, 0_deg}.toMatrix());
  animateCreature(turn, 0_deg);
}

void Bear::serialize(const serialization::Serializer<world::World>& ser) const
{
  AIAgent::serialize(ser);
  ser(S_NV("hurt", m_hurt));
}

void Bear::deserialize(const serialization::Deserializer<world::World>& ser)
{
  AIAgent::deserialize(ser);
  ser(S_NV("hurt", m_hurt));
}
} // namespace engine::objects
