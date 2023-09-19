#include "newgamemenustate.h"

#include "closepassportmenustate.h"
#include "core/i18n.h"
#include "donemenustate.h"
#include "menudisplay.h"
#include "menuring.h"
#include "selectlevelmenustate.h"
#include "ui/widgets/label.h"

namespace menu
{
NewGameMenuState::NewGameMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                   std::unique_ptr<MenuState> previous)
    : ListDisplayMenuState{ringTransform, /* translators: TR charmap encoding */ _("New Game"), 2}
    , m_previous{std::move(previous)}
{
  auto appendLabel = [this](const std::string& title)
  {
    auto tmp = gsl::make_shared<ui::widgets::Label>(title, ui::widgets::Label::Alignment::Center);
    tmp->fitToContent();
    append(tmp);
  };

  appendLabel(/* translators: TR charmap encoding */ _("New Game"));
  appendLabel(/* translators: TR charmap encoding */ _("Select Level"));
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
  default:
    BOOST_THROW_EXCEPTION(std::out_of_range("invalid menu selection"));
  }
}

std::unique_ptr<MenuState> NewGameMenuState::onAborted()
{
  return std::move(m_previous);
}
} // namespace menu
