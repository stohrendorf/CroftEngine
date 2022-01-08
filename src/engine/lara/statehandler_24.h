#pragma once

#include "abstractstatehandler.h"
#include "engine/audioengine.h"
#include "engine/collisioninfo.h"
#include "hid/inputstate.h"

namespace engine::lara
{
class StateHandler_24 final : public AbstractStateHandler
{
public:
  explicit StateHandler_24(objects::LaraObject& lara)
      : AbstractStateHandler{lara, LaraStateId::SlideForward}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
    getWorld().getCameraController().setModifier(CameraModifier::AllowSteepSlants);
    getWorld().getCameraController().setRotationAroundLaraX(-45_deg);
    if(getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Jump))
    {
      getWorld().getAudioEngine().stopSoundEffect(TR1SoundEffect::LaraSliding, &getLara().m_state);
      setGoalAnimState(LaraStateId::JumpForward);
    }
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    setMovementAngle(getLara().m_state.rotation.Y);
    commonSlideHandling(collisionInfo);
  }
};
} // namespace engine::lara
