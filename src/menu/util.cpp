#include "util.h"

#include "engine/presenter.h"
#include "engine/world.h"
#include "menuobject.h"

namespace menu
{
void rotateForSelection(MenuObject& object)
{
  if(object.rotationY == object.selectedRotationY)
    return;

  if(const auto dy = object.selectedRotationY - object.rotationY; dy > 0_deg && dy < 180_deg)
  {
    object.rotationY += 1024_au;
  }
  else
  {
    object.rotationY -= 1024_au;
  }
  object.rotationY -= object.rotationY % 1024_au;
}

void idleRotation(engine::World& world, MenuObject& object)
{
  if(world.getPresenter().getInputHandler().getInputState().xMovement == hid::AxisMovement::Null)
  {
    object.rotationY += 256_au;
  }
}

void zeroRotation(MenuObject& object, const core::Angle& speed)
{
  BOOST_ASSERT(speed >= 0_deg);

  if(object.rotationY < 0_deg)
  {
    object.rotationY += speed;
    if(object.rotationY > 0_deg)
      object.rotationY = 0_deg;
  }
  else if(object.rotationY > 0_deg)
  {
    object.rotationY -= speed;
    if(object.rotationY < 0_deg)
      object.rotationY = 0_deg;
  }
}
} // namespace menu
