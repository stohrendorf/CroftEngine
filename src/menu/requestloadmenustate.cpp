#include "requestloadmenustate.h"

#include "menudisplay.h"

#include <memory>

namespace menu
{
class MenuState;

void RequestLoadMenuState::handleObject(ui::Ui& /*ui*/,
                                        engine::world::World& /*world*/,
                                        MenuDisplay& /*display*/,
                                        MenuObject& /*object*/)
{
}

std::unique_ptr<MenuState>
  RequestLoadMenuState::onFrame(ui::Ui& /*ui*/, engine::world::World& /*world*/, MenuDisplay& display)
{
  display.requestLoad = m_slot;
  display.result = MenuResult::RequestLoad;
  return nullptr;
}

} // namespace menu
