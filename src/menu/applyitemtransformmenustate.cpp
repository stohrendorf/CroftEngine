#include "applyitemtransformmenustate.h"

#include "menudisplay.h"
#include "menuring.h"
#include "selectedmenustate.h"
#include "util.h"

namespace menu
{
void ApplyItemTransformMenuState::handleObject(engine::World& world, MenuDisplay& display, MenuObject& object)
{
  if(&object != &display.getCurrentRing().getSelectedObject())
  {
    zeroRotation(object, 256_au);
    return;
  }

  display.updateMenuObjectDescription(world, object);
  object.baseRotationX = exactScale(object.selectedBaseRotationX, m_duration, Duration);
  object.rotationX = exactScale(object.selectedRotationX, m_duration, Duration);
  object.positionZ = exactScale(object.selectedPositionZ, m_duration, Duration);

  if(object.rotationY != object.selectedRotationY)
  {
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
}

std::unique_ptr<MenuState>
  ApplyItemTransformMenuState::onFrame(gl::Image<gl::SRGBA8>& /*img*/, engine::World& /*world*/, MenuDisplay& display)
{
  display.updateRingTitle();

  if(m_duration != Duration)
  {
    m_duration += 1_frame;
    return nullptr;
  }

  return create<SelectedMenuState>();
}
} // namespace menu
