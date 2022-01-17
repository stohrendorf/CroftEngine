#include "skateboardkid.h"

#include "aiagent.h"
#include "core/angle.h"
#include "core/id.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine/ai/ai.h"
#include "engine/audioengine.h"
#include "engine/engine.h"
#include "engine/items_tr1.h"
#include "engine/lighting.h"
#include "engine/location.h"
#include "engine/skeletalmodelnode.h"
#include "engine/tracks_tr1.h"
#include "engine/world/animation.h"
#include "engine/world/skeletalmodeltype.h"
#include "engine/world/world.h"
#include "objectstate.h"
#include "qs/qs.h"
#include "render/scene/node.h"
#include "serialization/serialization.h"
#include "util/helpers.h"

#include <algorithm>
#include <exception>
#include <iterator>
#include <optional>

namespace engine::objects
{
SkateboardKid::SkateboardKid(const gsl::not_null<world::World*>& world, const Location& location)
    : AIAgent{world, location}
    , m_skateboard{std::make_shared<SkeletalModelNode>(
        "skateboard", world, gsl::not_null{world->findAnimatedModelForType(TR1ItemId::Skateboard).get()})}
{
}

SkateboardKid::SkateboardKid(const std::string& name,
                             const gsl::not_null<world::World*>& world,
                             const gsl::not_null<const world::Room*>& room,
                             const loader::file::Item& item,
                             const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
    : AIAgent{name, world, room, item, animatedModel}
    , m_skateboard{std::make_shared<SkeletalModelNode>(
        "skateboard", world, gsl::not_null{world->findAnimatedModelForType(TR1ItemId::Skateboard).get()})}
{
  m_state.current_anim_state = 2_as;
  SkeletalModelNode::buildMesh(m_skateboard, m_state.current_anim_state);
  m_lighting.bind(*m_skateboard);
}

void SkateboardKid::update()
{
  // FIXME this is done somewhere else in the original engine, so this should be replaced with an "Expects" instead of
  //       initializing it here.
  initCreatureInfo();

  core::Angle headRot = 0_deg;
  core::Angle turn = 0_deg;
  if(alive())
  {
    const ai::EnemyLocation enemyLocation{*this};
    if(enemyLocation.enemyAhead)
    {
      headRot = enemyLocation.angleToEnemy;
    }
    updateMood(*this, enemyLocation, false);
    turn = rotateTowardsTarget(4_deg / 1_frame);

    if(m_state.health < 120_hp && getWorld().getAudioEngine().getCurrentTrack() != TR1TrackId::LaraTalk30)
    {
      getWorld().getAudioEngine().playStopCdTrack(
        getWorld().getEngine().getScriptEngine().getGameflow(), TR1TrackId::LaraTalk30, false);
    }

    switch(m_state.current_anim_state.get())
    {
    case 0:
      m_triedShoot = false;
      if(m_state.required_anim_state != 0_as)
        goal(m_state.required_anim_state);
      else if(!canShootAtLara(enemyLocation))
        goal(2_as);
      else
        goal(1_as);
      break;
    case 1:
      [[fallthrough]];
    case 4:
      if(!m_triedShoot && canShootAtLara(enemyLocation))
      {
        if(tryShootAtLara(*this, enemyLocation.enemyDistance, core::TRVec{0_len, 150_len, 34_len}, 7, headRot))
        {
          if(m_state.current_anim_state == 1_as)
            hitLara(50_hp);
          else
            hitLara(40_hp);
        }
        if(tryShootAtLara(*this, enemyLocation.enemyDistance, core::TRVec{0_len, 150_len, 37_len}, 4, headRot))
        {
          if(m_state.current_anim_state == 1_as)
            hitLara(50_hp);
          else
            hitLara(40_hp);
        }
        m_triedShoot = true;
      }

      if(getCreatureInfo()->mood == ai::Mood::Escape || enemyLocation.enemyDistance < util::square(1024_len))
      {
        require(2_as);
      }
      break;
    case 2:
      m_triedShoot = false;
      if(util::rand15() < 512)
      {
        goal(3_as);
      }
      else if(canShootAtLara(enemyLocation))
      {
        if(getCreatureInfo()->mood == ai::Mood::Escape || enemyLocation.enemyDistance <= util::square(2560_len)
           || enemyLocation.enemyDistance >= util::square(4096_len))
        {
          goal(4_as);
        }
        else
        {
          goal(0_as);
        }
      }
      break;
    case 3:
      if(util::rand15() < 1024)
      {
        goal(2_as);
      }
      break;
    }
  }
  else if(m_state.current_anim_state != 5_as)
  {
    getSkeleton()->setAnim(
      gsl::not_null{&getWorld().findAnimatedModelForType(TR1ItemId::SkateboardKid)->animations[13]});
    m_state.current_anim_state = 5_as;
    getWorld().createPickup(TR1ItemId::UzisSprite, m_state.location.room, m_state.location.position);
  }

  rotateCreatureHead(headRot);
  getSkeleton()->patchBone(0, core::TRRotation{0_deg, getCreatureInfo()->headRotation, 0_deg}.toMatrix());
  animateCreature(turn, 0_deg);

  const auto animIdx = std::distance(getWorld().findAnimatedModelForType(TR1ItemId::SkateboardKid)->animations,
                                     getSkeleton()->getAnim());
  const auto& skateboardAnim = getWorld().findAnimatedModelForType(TR1ItemId::Skateboard)->animations[animIdx];
  const auto animFrame = skateboardAnim.firstFrame + getSkeleton()->getLocalFrame();
  m_skateboard->setAnim(gsl::not_null{&skateboardAnim}, std::min(skateboardAnim.lastFrame, animFrame));
  m_skateboard->updatePose();
  setParent(gsl::not_null{m_skateboard}, getNode());
}

void SkateboardKid::serialize(const serialization::Serializer<world::World>& ser)
{
  AIAgent::serialize(ser);
  ser(S_NV("triedShoot", m_triedShoot), S_NV("skateboard", m_skateboard));
}
} // namespace engine::objects
