#pragma once

#include "abstractstatehandler.h"
#include "engine/audioengine.h"
#include "engine/collisioninfo.h"
#include "hid/inputstate.h"

namespace engine::lara
{
class StateHandler_32 final : public AbstractStateHandler
{
public:
  explicit StateHandler_32(objects::LaraObject& lara)
      : AbstractStateHandler{lara, LaraStateId::SlideBackward}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/, bool /*doPhysics*/) override
  {
    if(getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Jump))
    {
      getWorld().getAudioEngine().stopSoundEffect(TR1SoundEffect::LaraSliding, &getLara().m_state);
      setGoalAnimState(LaraStateId::JumpBack);
    }
  }

  void postprocessFrame(CollisionInfo& collisionInfo, bool doPhysics) override
  {
    if(doPhysics)
      setMovementAngle(getLara().m_state.rotation.Y + 180_deg);
    
    commonSlideHandling(collisionInfo, doPhysics);
  }
};
} // namespace engine::lara
