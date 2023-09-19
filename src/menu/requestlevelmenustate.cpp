#include "requestlevelmenustate.h"

#include "menudisplay.h"

#include <optional>

namespace menu
{
class MenuState;

void RequestLevelMenuState::handleObject(ui::Ui& /*ui*/,
                                         engine::world::World& /*world*/,
                                         MenuDisplay& /*display*/,
                                         MenuObject& /*object*/)
{
}

std::unique_ptr<MenuState>
  RequestLevelMenuState::onFrame(ui::Ui& /*ui*/, engine::world::World& /*world*/, MenuDisplay& display)
{
  display.requestLevelSequenceIndex = m_sequenceIndex;
  display.result = MenuResult::RequestLevel;
  return nullptr;
}

} // namespace menu
