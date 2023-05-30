#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/world/skeletalmodeltype.h"

namespace engine::lara
{
class StateHandler_51 final : public AbstractStateHandler
{
public:
  explicit StateHandler_51(const gsl::not_null<objects::LaraObject*>& lara)
      : AbstractStateHandler{lara, LaraStateId::MidasDeath}
  {
  }

  void handleInput(CollisionInfo& collisionInfo) override
  {
    getLara().m_state.falling = false;
    collisionInfo.policies &= ~CollisionInfo::SpazPushPolicy;
    const auto& alternateLara = getWorld().getWorldGeometry().findAnimatedModelForType(TR1ItemId::AlternativeLara);
    if(alternateLara == nullptr)
      return;

    static constexpr gl::SRGBA8 GoldColor{255, 192, 64, 255};
    const auto skeleton = getLara().getSkeleton();
    switch(skeleton->getLocalFrame().get())
    {
    case 5:
      skeleton->setMesh(3, alternateLara->bones[3].mesh);
      skeleton->setReflective(3, GoldColor);
      skeleton->setMesh(6, alternateLara->bones[6].mesh);
      skeleton->setReflective(6, GoldColor);
      break;
    case 70:
      skeleton->setMesh(2, alternateLara->bones[2].mesh);
      skeleton->setReflective(2, GoldColor);
      break;
    case 90:
      skeleton->setMesh(1, alternateLara->bones[1].mesh);
      skeleton->setReflective(1, GoldColor);
      break;
    case 100:
      skeleton->setMesh(5, alternateLara->bones[5].mesh);
      skeleton->setReflective(5, GoldColor);
      break;
    case 120:
      skeleton->setMesh(0, alternateLara->bones[0].mesh);
      skeleton->setReflective(0, GoldColor);
      skeleton->setMesh(4, alternateLara->bones[4].mesh);
      skeleton->setReflective(4, GoldColor);
      break;
    case 135:
      skeleton->setMesh(7, alternateLara->bones[7].mesh);
      skeleton->setReflective(7, GoldColor);
      break;
    case 150:
      skeleton->setMesh(11, alternateLara->bones[11].mesh);
      skeleton->setReflective(11, GoldColor);
      break;
    case 163:
      skeleton->setMesh(12, alternateLara->bones[12].mesh);
      skeleton->setReflective(12, GoldColor);
      break;
    case 174:
      skeleton->setMesh(13, alternateLara->bones[13].mesh);
      skeleton->setReflective(13, GoldColor);
      break;
    case 186:
      skeleton->setMesh(8, alternateLara->bones[8].mesh);
      skeleton->setReflective(8, GoldColor);
      break;
    case 195:
      skeleton->setMesh(9, alternateLara->bones[9].mesh);
      skeleton->setReflective(9, GoldColor);
      break;
    case 218:
      skeleton->setMesh(10, alternateLara->bones[10].mesh);
      skeleton->setReflective(10, GoldColor);
      break;
    case 225:
      skeleton->setMesh(14, alternateLara->bones[14].mesh);
      skeleton->setReflective(14, GoldColor);
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
    collisionInfo.validFloorHeight = {-core::ClimbLimit2ClickMin, core::ClimbLimit2ClickMin};
    collisionInfo.validCeilingHeightMin = 0_len;
    collisionInfo.policies |= CollisionInfo::SlopeBlockingPolicy;
    collisionInfo.facingAngle = getLara().m_state.rotation.Y;
    collisionInfo.initHeightInfo(getLara().m_state.location.position, getWorld(), core::LaraWalkHeight);

    setMovementAngle(collisionInfo.facingAngle);
  }
};
} // namespace engine::lara
