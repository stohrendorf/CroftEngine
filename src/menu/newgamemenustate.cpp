#include "newgamemenustate.h"

#include "closepassportmenustate.h"
#include "core/i18n.h"
#include "donemenustate.h"
#include "engine/engine.h"
#include "engine/world/world.h"
#include "gameplayrulesmenustate.h"
#include "gsl/gsl-lite.hpp"
#include "listdisplaymenustate.h"
#include "menudisplay.h"
#include "menuring.h"
#include "selectlevelmenustate.h"
#include "ui/widgets/label.h"

#include <boost/throw_exception.hpp>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>

namespace menu
{
NewGameMenuState::NewGameMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                   std::unique_ptr<MenuState> previous)
    : ListDisplayMenuState{ringTransform, /* translators: TR charmap encoding */ _("New Game"), 3}
    , m_previous{std::move(previous)}
{
  auto appendLabel = [this](const std::string& title)
  {
    append(gsl::make_shared<ui::widgets::Label>(title, ui::widgets::Label::Alignment::Center));
  };

  appendLabel(/* translators: TR charmap encoding */ _("New Game"));
  appendLabel(/* translators: TR charmap encoding */ _("Select Level"));
  appendLabel(/* translators: TR charmap encoding */ _("Gameplay Rules"));
}

std::unique_ptr<MenuState> NewGameMenuState::onSelected(size_t idx, engine::world::World& world, MenuDisplay& display)
{
  switch(idx)
  {
  case 0:
    // new game
    return create<ClosePassportMenuState>(display.getCurrentRing().getSelectedObject(),
                                          create<DoneMenuState>(MenuResult::NewGame));
  case 1:
    // select level
    return create<SelectLevelMenuState>(std::move(display.m_currentState), world);
  case 2:
    // custom rules
    return create<GameplayRulesMenuState>(std::move(display.m_currentState), world.getEngine().getGameplayRules());
  default:
    BOOST_THROW_EXCEPTION(std::out_of_range("invalid menu selection"));
  }
}

std::unique_ptr<MenuState> NewGameMenuState::onAborted()
{
  return std::move(m_previous);
}
} // namespace menu
