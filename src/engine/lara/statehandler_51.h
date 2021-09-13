#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/world/skeletalmodeltype.h"

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
    collisionInfo.policies &= ~CollisionInfo::SpazPushPolicy;
    const auto& alternateLara = getWorld().findAnimatedModelForType(TR1ItemId::AlternativeLara);
    if(alternateLara == nullptr)
      return;

    const auto skeleton = getLara().getSkeleton();
    switch(skeleton->getLocalFrame().get())
    {
    case 5:
      skeleton->setMeshPart(3, alternateLara->bones[3].mesh);
      skeleton->setMeshPart(6, alternateLara->bones[6].mesh);
      break;
    case 70: skeleton->setMeshPart(2, alternateLara->bones[2].mesh); break;
    case 90: skeleton->setMeshPart(1, alternateLara->bones[1].mesh); break;
    case 100: skeleton->setMeshPart(5, alternateLara->bones[5].mesh); break;
    case 120:
      skeleton->setMeshPart(0, alternateLara->bones[0].mesh);
      skeleton->setMeshPart(4, alternateLara->bones[4].mesh);
      break;
    case 135: skeleton->setMeshPart(7, alternateLara->bones[7].mesh); break;
    case 150: skeleton->setMeshPart(11, alternateLara->bones[11].mesh); break;
    case 163: skeleton->setMeshPart(12, alternateLara->bones[12].mesh); break;
    case 174: skeleton->setMeshPart(13, alternateLara->bones[13].mesh); break;
    case 186: skeleton->setMeshPart(8, alternateLara->bones[8].mesh); break;
    case 195: skeleton->setMeshPart(9, alternateLara->bones[9].mesh); break;
    case 218: skeleton->setMeshPart(10, alternateLara->bones[10].mesh); break;
    case 225:
      skeleton->setMeshPart(14, alternateLara->bones[14].mesh);
      getLara().m_state.health = core::DeadHealth;
      break;
    default:
      // silence compiler
      break;
    }
    skeleton->rebuildMesh();
    StateHandler_50::emitSparkles(getWorld());
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    collisionInfo.floorCollisionRangeMin = core::ClimbLimit2ClickMin;
    collisionInfo.floorCollisionRangeMax = -core::ClimbLimit2ClickMin;
    collisionInfo.ceilingCollisionRangeMin = 0_len;
    setMovementAngle(getLara().m_state.rotation.Y);
    collisionInfo.policies |= CollisionInfo::SlopeBlockingPolicy;
    collisionInfo.facingAngle = getLara().m_state.rotation.Y;
    collisionInfo.initHeightInfo(getLara().m_state.location.position, getWorld(), core::LaraWalkHeight);
  }
};
} // namespace engine::lara
