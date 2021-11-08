#include "util.h"

#include "engine/presenter.h"
#include "engine/world/world.h"
#include "hid/inputhandler.h"
#include "hid/inputstate.h"
#include "menuobject.h"

#include <boost/assert.hpp>

namespace menu
{
void rotateForSelection(MenuObject& object)
{
  if(object.rotationY == object.selectedRotationY)
    return;

  static constexpr auto Speed = toRenderUnit(1024_au * 2 / 1_frame) * 1_rframe;
  if(normalizeAngle(object.selectedRotationY - object.rotationY) > 0_deg)
  {
    object.rotationY += Speed;
  }
  else
  {
    object.rotationY -= Speed;
  }
  object.rotationY -= object.rotationY % Speed;
}

void idleRotation(engine::world::World& world, MenuObject& object, bool force)
{
  if(force || world.getPresenter().getInputHandler().getInputState().xMovement == hid::AxisMovement::Null)
  {
    object.rotationY += toRenderUnit(256_au * 2 / 1_frame) * 1_rframe;
  }
}

void zeroRotation(MenuObject& object, const core::Angle& speed)
{
  BOOST_ASSERT(speed >= 0_deg);

  if(object.rotationY < 0_deg)
  {
    object.rotationY += toRenderUnit(speed * 2 / 1_frame) * 1_rframe;
    if(object.rotationY > 0_deg)
      object.rotationY = 0_deg;
  }
  else if(object.rotationY > 0_deg)
  {
    object.rotationY -= toRenderUnit(speed * 2 / 1_frame) * 1_rframe;
    if(object.rotationY < 0_deg)
      object.rotationY = 0_deg;
  }
}
} // namespace menu
