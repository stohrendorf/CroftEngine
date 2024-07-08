#pragma once

#include "abstractstatehandler.h"
#include "core/units.h"
#include "engine/collisioninfo.h"
#include "hid/actions.h"

#include <gsl/gsl-lite.hpp>

namespace engine::lara
{
class StateHandler_24 final : public AbstractStateHandler
{
public:
  explicit StateHandler_24(const gsl::not_null<objects::LaraObject*>& lara)
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
