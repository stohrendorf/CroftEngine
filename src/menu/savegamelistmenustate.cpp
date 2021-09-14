#include "savegamelistmenustate.h"

#include "closepassportmenustate.h"
#include "core/i18n.h"
#include "core/magic.h"
#include "deflateringmenustate.h"
#include "donemenustate.h"
#include "engine/engine.h"
#include "engine/presenter.h"
#include "engine/world/world.h"
#include "hid/actions.h"
#include "hid/inputhandler.h"
#include "listdisplaymenustate.h"
#include "menudisplay.h"
#include "menuring.h"
#include "menustate.h"
#include "requestloadmenustate.h"
#include "ui/widgets/label.h"
#include "ui/widgets/listbox.h"
#include "ui/widgets/messagebox.h"

#include <boost/format.hpp>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <iomanip>
#include <locale>
#include <map>
#include <sstream>
#include <utility>

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

  std::filesystem::file_time_type mostRecentTime = std::filesystem::file_time_type ::min();
  size_t mostRecentSlot = 0;
  for(size_t i = 0; i < core::SavegameSlots; ++i)
  {
    std::string name;
    if(auto it = savedGames.find(i); it != savedGames.end())
    {
      if(it->second.saveTime > mostRecentTime)
      {
        mostRecentTime = it->second.saveTime;
        mostRecentSlot = i;
      }

      const auto timePoint
        = std::chrono::system_clock::to_time_t(std::chrono::time_point_cast<std::chrono::system_clock::duration>(
          it->second.saveTime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now()));
      const auto localTime = localtime(&timePoint);
      Expects(localTime != nullptr);
      std::stringstream timeStr;
      timeStr.imbue(std::locale(world.getEngine().getLocale()));
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

  getListBox()->setSelected(mostRecentSlot);
}

std::unique_ptr<MenuState>
  SavegameListMenuState::onSelected(size_t idx, engine::world::World& world, MenuDisplay& display)
{
  if(!m_loading)
  {
    if(m_hasSavegame.at(idx))
    {
      m_confirmOverwrite = std::make_shared<ui::widgets::MessageBox>(
        /* translators: TR charmap encoding */ _("Overwrite slot %1%?", idx + 1));
      return nullptr;
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

std::unique_ptr<MenuState> SavegameListMenuState::onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display)
{
  if(m_confirmOverwrite == nullptr)
    return ListDisplayMenuState::onFrame(ui, world, display);

  draw(ui, world, display);
  m_confirmOverwrite->fitToContent();
  const auto vp = world.getPresenter().getViewport();
  m_confirmOverwrite->setPosition(
    {(vp.x - m_confirmOverwrite->getSize().x) / 2, vp.y - m_confirmOverwrite->getSize().y - 90});

  if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Left)
     || world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Right))
  {
    m_confirmOverwrite->setConfirmed(!m_confirmOverwrite->isConfirmed());
  }
  else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Action))
  {
    if(m_confirmOverwrite->isConfirmed())
    {
      world.save(getListBox()->getSelected());
      return create<ClosePassportMenuState>(display.getCurrentRing().getSelectedObject(),
                                            create<DeflateRingMenuState>(DeflateRingMenuState::Direction::Backpack,
                                                                         create<DoneMenuState>(MenuResult::Closed)));
    }
    else
    {
      m_confirmOverwrite.reset();
      return nullptr;
    }
  }

  m_confirmOverwrite->update(true);
  m_confirmOverwrite->draw(ui, world.getPresenter());
  return nullptr;
}
} // namespace menu
