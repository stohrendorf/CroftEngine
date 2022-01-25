#include "cutsceneactors.h"

#include "core/angle.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine/cameracontroller.h"
#include "engine/location.h"
#include "engine/world/world.h"
#include "modelobject.h"
#include "objectstate.h"

namespace engine::objects
{
CutsceneActor::CutsceneActor(const std::string& name,
                             const gsl::not_null<world::World*>& world,
                             const gsl::not_null<const world::Room*>& room,
                             const loader::file::Item& item,
                             const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
    : ModelObject{name, world, room, item, true, animatedModel, true}
{
  activate();
  m_state.rotation.Y = 0_deg;
}

void CutsceneActor::update()
{
  m_state.rotation.Y = getWorld().getCameraController().getEyeRotation().Y;
  m_state.location.position = getWorld().getCameraController().getTRLocation().position;
  ModelObject::update();
}

void CutsceneActor4::update()
{
  ModelObject::update(); // NOLINT(bugprone-parent-virtual-call)
}
} // namespace engine::objects
