#include "savegamelistmenustate.h"

#include "closepassportmenustate.h"
#include "core/i18n.h"
#include "deflateringmenustate.h"
#include "donemenustate.h"
#include "engine/engine.h"
#include "engine/presenter.h"
#include "engine/world/world.h"
#include "menudisplay.h"
#include "menuring.h"
#include "requestloadmenustate.h"
#include "ui/widgets/label.h"
#include "ui/widgets/vbox.h"

namespace menu
{
SavegameListMenuState::SavegameListMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                             std::unique_ptr<MenuState> previous,
                                             const std::string& heading,
                                             const engine::world::World& world,
                                             bool loading)
    : ListDisplayMenuState{ringTransform, heading, 10}
    , m_previous{std::move(previous)}
    , m_loading{loading}
{
  const auto savedGames = world.getSavedGames();

  for(size_t i = 0; i < TotalSlots; ++i)
  {
    std::string name;
    if(auto it = savedGames.find(i); it != savedGames.end())
    {
      name = it->second.title;
      m_hasSavegame.emplace_back(true);
    }
    else
    {
      name = _("- EMPTY SLOT %1%", i + 1);
      m_hasSavegame.emplace_back(false);
    }
    auto label = std::make_shared<ui::widgets::Label>(glm::ivec2{0, 0}, name, 0);
    append(label);
  }
}

std::unique_ptr<MenuState>
  SavegameListMenuState::onSelected(size_t idx, engine::world::World& world, MenuDisplay& display)
{
  if(!m_loading)
  {
    // TODO confirm overwrite if necessary
    world.save(idx);
    return create<ClosePassportMenuState>(display.getCurrentRing().getSelectedObject(),
                                          create<DeflateRingMenuState>(DeflateRingMenuState::Direction::Backpack,
                                                                       create<DoneMenuState>(MenuResult::Closed)));
  }
  else if(m_hasSavegame.at(idx))
  {
    return create<ClosePassportMenuState>(
      display.getCurrentRing().getSelectedObject(),
      create<DeflateRingMenuState>(DeflateRingMenuState::Direction::Backpack, create<RequestLoadMenuState>(idx)));
  }
  return std::move(m_previous);
}

std::unique_ptr<MenuState> SavegameListMenuState::onAborted()
{
  return std::move(m_previous);
}
} // namespace menu
