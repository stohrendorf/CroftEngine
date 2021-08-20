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
#include "ui/widgets/listbox.h"

#include <ctime>
#include <sstream>

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
      const auto timePoint
        = std::chrono::system_clock::to_time_t(std::chrono::time_point_cast<std::chrono::system_clock::duration>(
          it->second.saveTime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now()));
      const auto localTime = localtime(&timePoint);
      Expects(localTime != nullptr);
      std::stringstream timeStr;
      timeStr.imbue(std::locale(world.getEngine().getLanguage()));
      timeStr << std::put_time(localTime,
                               /* translators: TR charmap encoding */ pgettext("SavegameTime", "%d %B %Y %X"));
      name = (boost::format(/* translators: TR charmap encoding */ pgettext("SavegameTitle", "%1% - %2%"))
              % timeStr.str() % it->second.meta.title)
               .str();

      m_hasSavegame.emplace_back(true);
    }
    else
    {
      name = _("- EMPTY SLOT %1%", i + 1);
      m_hasSavegame.emplace_back(false);
    }
    auto label = std::make_shared<ui::widgets::Label>(name);
    label->fitToContent();
    append(label);
  }
}

std::unique_ptr<MenuState>
  SavegameListMenuState::onSelected(size_t idx, engine::world::World& world, MenuDisplay& display)
{
  if(!m_loading)
  {
    if(m_hasSavegame.at(idx))
    {
      // TODO confirm overwrite if necessary
    }
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
