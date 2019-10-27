#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine
{
namespace lara
{
class StateHandler_51 final : public AbstractStateHandler
{
public:
  explicit StateHandler_51(LaraNode& lara)
      : AbstractStateHandler{lara, LaraStateId::MidasDeath}
  {
  }

  void handleInput(CollisionInfo& collisionInfo) override
  {
    getLara().m_state.falling = false;
    collisionInfo.policyFlags &= ~CollisionInfo::SpazPushPolicy;
    const auto& alternateLara = getEngine().findAnimatedModelForType(TR1ItemId::LaraShotgunAnim);
    if(alternateLara == nullptr)
      return;

    const auto frameOffs = getLara().m_state.frame_number - getLara().m_state.anim->firstFrame;
    switch(frameOffs.get())
    {
    case 5:
      getLara().getNode()->getChild(3)->setDrawable(alternateLara->models[3].get());
      getLara().getNode()->getChild(6)->setDrawable(alternateLara->models[6].get());
      break;
    case 70: getLara().getNode()->getChild(2)->setDrawable(alternateLara->models[2].get()); break;
    case 90: getLara().getNode()->getChild(1)->setDrawable(alternateLara->models[1].get()); break;
    case 100: getLara().getNode()->getChild(5)->setDrawable(alternateLara->models[5].get()); break;
    case 120:
      getLara().getNode()->getChild(0)->setDrawable(alternateLara->models[0].get());
      getLara().getNode()->getChild(4)->setDrawable(alternateLara->models[4].get());
      break;
    case 135: getLara().getNode()->getChild(7)->setDrawable(alternateLara->models[7].get()); break;
    case 150: getLara().getNode()->getChild(11)->setDrawable(alternateLara->models[11].get()); break;
    case 163: getLara().getNode()->getChild(12)->setDrawable(alternateLara->models[12].get()); break;
    case 174: getLara().getNode()->getChild(13)->setDrawable(alternateLara->models[13].get()); break;
    case 186: getLara().getNode()->getChild(8)->setDrawable(alternateLara->models[8].get()); break;
    case 195: getLara().getNode()->getChild(9)->setDrawable(alternateLara->models[9].get()); break;
    case 218: getLara().getNode()->getChild(10)->setDrawable(alternateLara->models[10].get()); break;
    case 225: getLara().getNode()->getChild(14)->setDrawable(alternateLara->models[14].get()); break;
    default:
      // silence compiler
      break;
    }
    StateHandler_50::emitSparkles(getLara(), getEngine());
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    collisionInfo.badPositiveDistance = core::ClimbLimit2ClickMin;
    collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
    collisionInfo.badCeilingDistance = 0_len;
    setMovementAngle(getLara().m_state.rotation.Y);
    collisionInfo.policyFlags |= CollisionInfo::SlopeBlockingPolicy;
    collisionInfo.facingAngle = getLara().m_state.rotation.Y;
    collisionInfo.initHeightInfo(getLara().m_state.position.position, getEngine(), core::LaraWalkHeight);
  }
};
} // namespace lara
} // namespace engine
