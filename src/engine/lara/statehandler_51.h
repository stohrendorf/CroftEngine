#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine::lara
{
class StateHandler_51 final : public AbstractStateHandler
{
public:
  explicit StateHandler_51(objects::LaraObject& lara)
      : AbstractStateHandler{lara, LaraStateId::MidasDeath}
  {
  }

  void handleInput(CollisionInfo& collisionInfo) override
  {
    getLara().m_state.falling = false;
    collisionInfo.policyFlags &= ~CollisionInfo::SpazPushPolicy;
    const auto& alternateLara = getWorld().findAnimatedModelForType(TR1ItemId::LaraShotgunAnim);
    if(alternateLara == nullptr)
      return;

    switch(getLara().getSkeleton()->getLocalFrame().get())
    {
    case 5:
      getLara().getSkeleton()->setMeshPart(3, alternateLara->bones[3].mesh);
      getLara().getSkeleton()->setMeshPart(6, alternateLara->bones[6].mesh);
      break;
    case 70: getLara().getSkeleton()->setMeshPart(2, alternateLara->bones[2].mesh); break;
    case 90: getLara().getSkeleton()->setMeshPart(1, alternateLara->bones[1].mesh); break;
    case 100: getLara().getSkeleton()->setMeshPart(5, alternateLara->bones[5].mesh); break;
    case 120:
      getLara().getSkeleton()->setMeshPart(0, alternateLara->bones[0].mesh);
      getLara().getSkeleton()->setMeshPart(4, alternateLara->bones[4].mesh);
      break;
    case 135: getLara().getSkeleton()->setMeshPart(7, alternateLara->bones[7].mesh); break;
    case 150: getLara().getSkeleton()->setMeshPart(11, alternateLara->bones[11].mesh); break;
    case 163: getLara().getSkeleton()->setMeshPart(12, alternateLara->bones[12].mesh); break;
    case 174: getLara().getSkeleton()->setMeshPart(13, alternateLara->bones[13].mesh); break;
    case 186: getLara().getSkeleton()->setMeshPart(8, alternateLara->bones[8].mesh); break;
    case 195: getLara().getSkeleton()->setMeshPart(9, alternateLara->bones[9].mesh); break;
    case 218: getLara().getSkeleton()->setMeshPart(10, alternateLara->bones[10].mesh); break;
    case 225: getLara().getSkeleton()->setMeshPart(14, alternateLara->bones[14].mesh); break;
    default:
      // silence compiler
      break;
    }
    getLara().getSkeleton()->rebuildMesh();
    StateHandler_50::emitSparkles(getWorld());
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    collisionInfo.badPositiveDistance = core::ClimbLimit2ClickMin;
    collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
    collisionInfo.badCeilingDistance = 0_len;
    setMovementAngle(getLara().m_state.rotation.Y);
    collisionInfo.policyFlags |= CollisionInfo::SlopeBlockingPolicy;
    collisionInfo.facingAngle = getLara().m_state.rotation.Y;
    collisionInfo.initHeightInfo(getLara().m_state.position.position, getWorld(), core::LaraWalkHeight);
  }
};
} // namespace engine::lara
