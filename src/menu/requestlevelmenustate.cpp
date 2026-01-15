#include "requestlevelmenustate.h"

#include "menudisplay.h"

#include <memory>

namespace menu
{
class MenuState;

void RequestLevelMenuState::handleObjectTick(engine::world::World& /*world*/,
                                             MenuDisplay& /*display*/,
                                             MenuObject& /*object*/)
{
}

std::unique_ptr<MenuState> RequestLevelMenuState::tick(engine::world::World& /*world*/, MenuDisplay& display)
{
  display.requestLevelSequenceIndex = m_sequenceIndex;
  display.result = MenuResult::RequestLevel;
  return nullptr;
}

void RequestLevelMenuState::constructUi(ui::Ui& /*ui*/, engine::world::World& /*world*/, MenuDisplay& /*display*/)
{
}
} // namespace menu
