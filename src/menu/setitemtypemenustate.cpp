#include "setitemtypemenustate.h"

#include "menudisplay.h"
#include "menuobject.h"
#include "menuring.h"
#include "menustate.h"

namespace menu
{
std::unique_ptr<MenuState>
  SetItemTypeMenuState::onFrame(ui::Ui& /*ui*/, engine::world::World& /*world*/, MenuDisplay& /*display*/)
{
  return std::move(m_next);
}

void SetItemTypeMenuState::handleObject(ui::Ui& /*ui*/,
                                        engine::world::World& world,
                                        MenuDisplay& display,
                                        MenuObject& object)
{
  if(&object == &display.getCurrentRing().getSelectedObject())
  {
    object.type = m_type;
    object.initModel(world, display.getLightsBuffer());
  }
}
} // namespace menu
