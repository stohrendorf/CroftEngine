#include "setitemtypemenustate.h"

#include "menudisplay.h"
#include "menuring.h"

namespace menu
{
std::unique_ptr<MenuState>
  SetItemTypeMenuState::onFrame(gl::Image<gl::SRGBA8>& /*img*/, engine::World& /*world*/, MenuDisplay& /*display*/)
{
  return std::move(m_next);
}

void SetItemTypeMenuState::handleObject(engine::World& world, MenuDisplay& display, MenuObject& object)
{
  if(&object == &display.getCurrentRing().getSelectedObject())
  {
    object.type = m_type;
    object.initModel(world);
  }
}
} // namespace menu
