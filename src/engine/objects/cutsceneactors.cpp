#include "cutsceneactors.h"

#include "core/units.h"
#include "engine/cameracontroller.h"
#include "engine/world/world.h"
#include "modelobject.h"
#include "objectstate.h"

#include <gsl-lite/gsl-lite.hpp>
#include <string>

namespace engine::objects
{
CutsceneActor::CutsceneActor(const std::string& name,
                             const gsl_lite::not_null<world::World*>& world,
                             const gsl_lite::not_null<const world::Room*>& room,
                             const loader::file::Item& item,
                             const gsl_lite::not_null<const world::SkeletalModelType*>& animatedModel)
    : ModelObject{name, world, room, item, true, animatedModel, true}
{
  activate();
  m_state.rotation.Y = 0_deg;
}

void CutsceneActor::updateLogic()
{
  m_state.rotation.Y = getWorld().getCameraController().getCinematicBaseRotation().Y;
  m_state.location.position = getWorld().getCameraController().getCinematicPos();
  advanceFrame();
}

void CutsceneActor4::updateLogic()
{
  advanceFrame();
}
} // namespace engine::objects