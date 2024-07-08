#include "applyitemtransformmenustate.h"

#include "core/units.h"
#include "menu/menuobject.h"
#include "menudisplay.h"
#include "menuring.h"
#include "qs/qs.h"
#include "selectedmenustate.h"
#include "util.h"

#include <memory>

namespace menu
{
class MenuState;

void ApplyItemTransformMenuState::handleObject(ui::Ui& /*ui*/,
                                               engine::world::World& /*world*/,
                                               MenuDisplay& display,
                                               MenuObject& object)
{
  if(&object != &display.getCurrentRing().getSelectedObject())
  {
    zeroRotation(object, 256_au);
    return;
  }

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
  ApplyItemTransformMenuState::onFrame(ui::Ui& /*ui*/, engine::world::World& /*world*/, MenuDisplay& /*display*/)
{
  if(m_duration != Duration)
  {
    m_duration += 1_frame;
    return nullptr;
  }

  return create<SelectedMenuState>();
}
} // namespace menu
