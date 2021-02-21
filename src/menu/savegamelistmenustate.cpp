#include "savegamelistmenustate.h"

#include "engine/engine.h"
#include "engine/i18n.h"
#include "engine/presenter.h"
#include "engine/world.h"
#include "menudisplay.h"
#include "ui/label.h"

#include <boost/format.hpp>

namespace menu
{
SavegameListMenuState::SavegameListMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                             std::unique_ptr<MenuState> previous,
                                             const std::string& heading,
                                             const engine::World& world,
                                             bool loading)
    : ListDisplayMenuState{ringTransform, heading}
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
      name = world.getEngine().i18n(engine::I18n::EmptySlot, i + 1);
      m_hasSavegame.emplace_back(false);
    }
    addEntry(name);
  }
}

std::unique_ptr<MenuState> SavegameListMenuState::onSelected(size_t idx, engine::World& world, MenuDisplay& display)
{
  if(!m_loading)
    // TODO confirm overwrite if necessary
    world.save(idx);
  else if(m_hasSavegame.at(idx))
  {
    display.requestLoad = idx;
    display.result = MenuResult::RequestLoad;
  }
  return std::move(m_previous);
}

std::unique_ptr<MenuState> SavegameListMenuState::onAborted()
{
  return std::move(m_previous);
}
} // namespace menu
