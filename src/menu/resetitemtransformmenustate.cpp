#include "resetitemtransformmenustate.h"

#include "menudisplay.h"
#include "menuring.h"
#include "util.h"

namespace menu
{
void ResetItemTransformMenuState::handleObject(engine::World& world, MenuDisplay& display, MenuObject& object)
{
  if(&object == &display.getCurrentRing().getSelectedObject())
  {
    display.updateMenuObjectDescription(world, object);
    object.baseRotationX = exactScale(object.selectedBaseRotationX, m_duration, Duration);
    object.rotationX = exactScale(object.selectedRotationX, m_duration, Duration);
    object.positionZ = exactScale(object.selectedPositionZ, m_duration, Duration);
  }
  else
  {
    zeroRotation(object, 256_au);
  }
}

std::unique_ptr<MenuState> ResetItemTransformMenuState::onFrame(gl::Image<gl::SRGBA8>& /*img*/,
                                                                engine::World& /*world*/,
                                                                MenuDisplay& /*display*/)
{
  if(m_duration != 0_frame)
  {
    m_duration -= 1_frame;
    return nullptr;
  }

  return std::move(m_next);
}
} // namespace menu
